#include "base/zdefs.h"
#include "zc/maps.h"
#include "zc/replay.h"
#include "zc/zelda.h"

#include <optional>
#include <cstring>
#include <set>
#include <stdio.h>

#include "base/qrs.h"
#include "base/dmap.h"
#include "zc/hero.h"
#include "zc/guys.h"
#include "subscr.h"
#include "zc/zc_subscr.h"
#include "zc/decorations.h"
#include "gamedata.h"
#include "zc/zc_custom.h"
#include "zc/title.h"
#include "zc/ffscript.h"
#include "drawing.h"
#include "zc/combos.h"
#include "base/zc_math.h"
#include "user_object.h"
#include "slopes.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include "iter.h"
extern FFScript FFCore;
extern HeroClass Hero;
extern ZModule zcm;
extern zcmodule moduledata;
extern refInfo playerScriptData;
#include "zscriptversion.h"
#include "particles.h"
#include <fmt/format.h>
#include "zc/render.h"
#include <array>

extern refInfo *ri; //= NULL;
extern int32_t(*stack)[MAX_SCRIPT_REGISTERS];
extern portal mirror_portal;
using std::set;

extern int32_t skipcont;

extern int32_t draw_screen_clip_rect_x1;
extern int32_t draw_screen_clip_rect_x2;
extern int32_t draw_screen_clip_rect_y1;
extern int32_t draw_screen_clip_rect_y2;

int32_t hero_count = -1;
int32_t hero_animation_speed = 1; //lower is faster animation
static int32_t z3step = 2;
static zfix hero_newstep(1.5);
static zfix hero_newstep_diag(1.5);
bool did_scripta=false;
bool did_scriptb=false;
bool did_scriptl=false;
byte lshift = 0;
int32_t dowpn = -1;
int32_t directItemA = -1;
int32_t directItemB = -1;
int32_t directItemX = -1;
int32_t directItemY = -1;
int32_t directWpn = -1;
int32_t whistleitem=-1;
extern int32_t script_hero_cset;

void playLevelMusic();

extern particle_list particles;

byte lsteps[8] = { 1, 1, 2, 1, 1, 2, 1, 1 };

#define CANFORCEFACEUP	(get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0 && dir!=up && (action==walking || action==none))
#define NO_GRIDLOCK		(get_qr(qr_DISABLE_4WAY_GRIDLOCK)||get_qr(qr_NEW_HERO_MOVEMENT2))
#define SWITCHBLOCK_STATE (switchblock_z<0?switchblock_z:(switchblock_z+z+fakez < 0 ? zslongToFix(2147483647) : switchblock_z+z+fakez))
#define FIXED_Z3_ANIMATION ((zinit.heroAnimationStyle==las_zelda3||zinit.heroAnimationStyle==las_zelda3slow)&&!get_qr(qr_BROKEN_Z3_ANIMATION))

bool item_error()
{
	if(QMisc.miscsfx[sfxERROR])
		sfx(QMisc.miscsfx[sfxERROR]);
	return false;
}
static inline bool on_sideview_slope(int32_t x, int32_t y, int32_t oldx, int32_t oldy)
{
	if(check_new_slope(x, y+1, 16, 16, oldx, oldy) < 0) return true;
	return false;
}

static inline bool platform_fallthrough(bool doslopecheck = true)
{
	if (!getInput(btnDown, false, get_qr(qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK)!=0))
		return false;

	if (get_qr(qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS))
		return true;

	if (Hero.jumping < 0 && get_qr(qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS))
		return true;

	if (doslopecheck && !on_sideview_slope(Hero.x, Hero.y,Hero.old_x,Hero.old_y) && (on_sideview_slope(Hero.x,Hero.y+1,Hero.old_x,Hero.old_y) || on_sideview_slope(Hero.x, Hero.y + 2, Hero.old_x, Hero.old_y)))
		return true;

	return false;
}

static inline bool on_sideview_solid(int32_t x, int32_t y, bool ignoreFallthrough = false, int32_t slopesmisc = 0)
{
	mapscr* s = get_screen_for_world_xy(x, y);
	if(slopesmisc != 1 && check_slope(x, y+1, 16, 16, (slopesmisc == 3)) < 0) return true;
	if(slopesmisc == 2) return false;
	if (_walkflag(x+4,y+16,1) || _walkflag(x+12,y+16,1)) return true;
	if (y>=world_h-16 && currscr>=0x70 && !(s->flags2&wfDOWN)) return true;
	if (platform_fallthrough() && !ignoreFallthrough) return false;
	if(slopesmisc != 1 && check_slope(x, y+1, 16, 16) < 0) return true;
	if (y%16==0 && (checkSVLadderPlatform(x+4,y+16) || checkSVLadderPlatform(x+12,y+16)))
		return true;
	return false;
}

static inline bool on_sideview_solid_oldpos(int32_t x, int32_t y, int32_t oldx, int32_t oldy, bool ignoreFallthrough = false, int32_t slopesmisc = 0)
{
	mapscr* s = get_screen_for_world_xy(x, y);
	if(slopesmisc != 1 && check_new_slope(x, y+1, 16, 16, oldx, oldy, (slopesmisc == 3)) < 0) return true;
	if(slopesmisc == 2) return false;
	if (_walkflag(x+4,y+16,1) || _walkflag(x+12,y+16,1)) return true;
	if (y>=world_h-16 && currscr>=0x70 && !(s->flags2&wfDOWN)) return true;
	if (platform_fallthrough() && !ignoreFallthrough) return false;
	if (slopesmisc != 1 && check_new_slope(x, y + 1, 16, 16, oldx, oldy) < 0) return true;
	if (y%16==0 && (checkSVLadderPlatform(x+4,y+16) || checkSVLadderPlatform(x+12,y+16)))
		return true;
	return false;
}

void HeroClass::snap_platform()
{
	if(check_new_slope(x, y+1, 16, 16, old_x, old_y, false) < 0)
		return;
	if (y>=world_h-16 && currscr>=0x70 && !(hero_screen->flags2&wfDOWN))
	{
		y = world_h-16;
		return;
	}
	if (!(_walkflag(x+4,y+16,1) || _walkflag(x+12,y+16,1)))
		return;
	movexy(0,1,false,true,false,false);
}


bool usingActiveShield(int32_t itmid)
{
	if(Hero.shieldjinxclk) return false;
	switch(Hero.action) //filter allowed actions
	{
		case none: case walking: case rafting:
		case gothit: case swimhit:
			break;
		default: return false;
	}
	if(Hero.lift_wpn && (Hero.liftflags&LIFTFL_DIS_SHIELD)) return false;
	if(itmid < 0)
		itmid = (Hero.active_shield_id < 0
			? current_item_id(itype_shield,true,true) : Hero.active_shield_id);
	if(itmid < 0) return false;
	if(item_disabled(itmid)) return false;
	if(!checkitem_jinx(itmid)) return false;
	if(!(itemsbuf[itmid].flags & ITEM_FLAG9)) return false;
	if(!isItmPressed(itmid)) return false;
	return (checkbunny(itmid) && checkmagiccost(itmid));
}
int32_t getCurrentShield(bool requireActive)
{
	if(Hero.shieldjinxclk) return -1;
	if(Hero.active_shield_id > -1 && usingActiveShield(Hero.active_shield_id))
		return Hero.active_shield_id;
	if(!requireActive) return current_item_id(itype_shield);
	return -1;
}
int32_t getCurrentActiveShield()
{
	int32_t id = Hero.active_shield_id;
	if(id > -1 && usingActiveShield(id))
		return id;
	return -1;
}
int32_t refreshActiveShield()
{
	int32_t id = -1;
    if(DrunkcBbtn())
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Bwpn,0xFFF)];
		if(dat.family == itype_shield && (dat.flags & ITEM_FLAG9))
		{
			id = NEG_OR_MASK(Bwpn,0xFFF);
		}
	}
    if(id < 0 && DrunkcAbtn())
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Awpn,0xFFF)];
		if(dat.family == itype_shield && (dat.flags & ITEM_FLAG9))
		{
			id = NEG_OR_MASK(Awpn,0xFFF);
		}
	}
    if(id < 0 && DrunkcEx1btn())
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Xwpn,0xFFF)];
		if(dat.family == itype_shield && (dat.flags & ITEM_FLAG9))
		{
			id = NEG_OR_MASK(Xwpn,0xFFF);
		}
	}
    if(id < 0 && DrunkcEx2btn())
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Ywpn,0xFFF)];
		if(dat.family == itype_shield && (dat.flags & ITEM_FLAG9))
		{
			id = NEG_OR_MASK(Ywpn,0xFFF);
		}
	}
	if(!usingActiveShield(id))
		return -1;
    return id;
}
static bool is_immobile()
{
	if(!get_qr(qr_NEW_HERO_MOVEMENT))
		return false;
	zfix rate(Hero.steprate);
	int32_t shieldid = getCurrentActiveShield();
	if(shieldid > -1)
	{
		itemdata const& shield = itemsbuf[shieldid];
		if(shield.flags & ITEM_FLAG10) //Change Speed flag
		{
			zfix perc = shield.misc7;
			perc /= 100;
			if(perc < 0)
				perc = (perc*-1)+1;
			rate = (rate * perc) + shield.misc8;
		}
	}
	return rate != 0;
}

bool nomove_action(int action)
{
	switch(action)
	{
		case gothit:
		case drowning:
		case lavadrowning:
		case sidedrowning:
		case falling:
		case freeze:
		case sideswimfreeze:
		case scrolling:
		case casting:
		case sideswimcasting:
		case landhold1:
		case landhold2:
		case waterhold1:
		case waterhold2:
		case sidewaterhold1:
		case sidewaterhold2:
		case hopping:
		case inwind:
			return true;
	}
	return false;
}

bool HeroClass::isStanding(bool forJump)
{
	if(z || fakez) return false;
	if(isSideViewHero() && !on_sideview_solid_oldpos(x,y,old_x,old_y)
		&& !ladderx && !laddery && !getOnSideviewLadder())
		return false;
	if(hoverclk) return false;
	if(nomove_action(action)) return false;
	int32_t val = check_pitslide();
	if(val == -2) return false;
	if(val == -1) return true;
	return forJump;
}
bool HeroClass::isLifting()
{
	if(lift_wpn) return true;
	return false;
}
void HeroClass::set_liftflags(int liftid)
{
	if(unsigned(liftid) >= MAXITEMS)
		return;
	itemdata const& itm = itemsbuf[liftid];
	SETFLAG(liftflags, LIFTFL_DIS_SHIELD, itm.flags & ITEM_FLAG3);
	SETFLAG(liftflags, LIFTFL_DIS_ITEMS, itm.flags & ITEM_FLAG4);
}

void HeroClass::set_respawn_point(bool setwarp)
{
	zfix oldx = x, oldy = y;
	if (replay_version_check(17))
	{
		x = vbound(x,0,world_w-16);
		y = vbound(y,0,world_h-16);
	}
	
	if(setwarp)
	{
		warpx = x;
		warpy = y;
		raftwarpx = x;
		raftwarpy = y;
	}
	do
	{
		if(!get_qr(qr_OLD_RESPAWN_POINTS))
		{
			if(currscr >= 0x80) break;
			bool is_safe = true;
			switch(action)
			{
				case none: case walking:
					break;
				default:
					is_safe = false;
			}
			if(!is_safe) break; //unsafe action
			if(z > 0 || fakez > 0 || hoverclk) break; //in air
			if(sideview_mode() && !on_sideview_solid(x,y,true)) break; //in air sideview
			if(check_pitslide(true) != -1) break; //On a pit
			if (ladderx+laddery) break; //on the ladder
			
			{ //Check water
				int32_t water = 0;
				int32_t types[4] = {0};
				int32_t x1 = x+4, x2 = x+11,
					y1 = y+9, y2 = y+15;
				if (get_qr(qr_SMARTER_WATER))
				{
					if (iswaterex_z3(0, -1, x1, y1, true, false) &&
					iswaterex_z3(0, -1, x1, y2, true, false) &&
					iswaterex_z3(0, -1, x2, y1, true, false) &&
					iswaterex_z3(0, -1, x2, y2, true, false)) water = iswaterex_z3(0, -1, (x2+x1)/2,(y2+y1)/2, true, false);
				}
				else
				{
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
						
					int32_t typec = COMBOTYPE((x2+x1)/2,(y2+y1)/2);
					if(MAPFFCOMBO((x2+x1)/2,(y2+y1)/2))
						typec = FFCOMBOTYPE((x2+x1)/2,(y2+y1)/2);
						
					if(combo_class_buf[types[0]].water && combo_class_buf[types[1]].water &&
							combo_class_buf[types[2]].water && combo_class_buf[types[3]].water && combo_class_buf[typec].water)
						water = typec;
				}
				if(water > 0)
					break;
			} //End check water
			
			rpos_t rposes[] = {
				COMBOPOS_REGION(x,y+(bigHitbox?0:8)),
				COMBOPOS_REGION(x,y+15),
				COMBOPOS_REGION(x+15,y+(bigHitbox?0:8)),
				COMBOPOS_REGION(x+15,y+15)
				};
			for(auto rpos : rposes)
			{
				if(HASFLAG_ANY(mfUNSAFEGROUND, rpos)) //"Unsafe Ground" flag touching the player
				{
					is_safe = false;
					break;
				}
			}
			if(!is_safe) break;
		}
		respawn_x = x;
		respawn_y = y;
		respawn_scr = currscr;
		respawn_dmap = currdmap;
	}
	while(false); //run once, but 'break' works
	
	x = oldx;
	y = oldy;
}

void HeroClass::go_respawn_point()
{
	x = respawn_x;
	y = respawn_y;
	handle_portal_prox(&mirror_portal);
	portals.forEach([&](sprite& p)
	{
		handle_portal_prox((portal*)&p);
		return false;
	});
	warpx=x;
	warpy=y;
	raftwarpx = x;
	raftwarpy = y;
	trySideviewLadder(); //Cling to ladder automatically
	
	if(get_qr(qr_OLD_RESPAWN_POINTS))
		return; //No cross-screen return
	
	if(currdmap != respawn_dmap || currscr != respawn_scr)
	{
		FFCore.warp_player(wtIWARP, respawn_dmap, respawn_scr,
			-1, -1, 0, 0, warpFlagNOSTEPFORWARD|warpFlagDONTKILLMUSIC, -1);
	}
}

void HeroClass::trySideviewLadder()
{
	if(canSideviewLadder() && !on_sideview_solid_oldpos(x,y,old_x,old_y))
		setOnSideviewLadder(true);
}

bool HeroClass::can_pitfall(bool ignore_hover)
{
	return (!(isSideViewGravity()||action==rafting||z>0||fakez>0||fall<0||fakefall<0||(hoverclk && !ignore_hover)||inlikelike||inwallm||pull_hero||toogam||(ladderx||laddery)||getOnSideviewLadder()||drownclk||!(moveflags & FLAG_CAN_PITFALL)));
}

int32_t HeroClass::DrunkClock()
{
    return drunkclk;
}
void HeroClass::setDrunkClock(int32_t newdrunkclk)
{
    drunkclk=newdrunkclk;
}

int32_t HeroClass::StunClock()
{
    return lstunclock;
}
void HeroClass::setStunClock(int32_t v)
{
    lstunclock=v;
}

int32_t HeroClass::BunnyClock()
{
    return lbunnyclock;
}
void HeroClass::setBunnyClock(int32_t v)
{
    lbunnyclock=v;
}

HeroClass::HeroClass() : sprite()
{
	lift_wpn = nullptr;
    init();
}

//2.6

//Stop the subscreen from falling. -Z

bool HeroClass::stopSubscreenFalling(){
	return preventsubscreenfalling;
}

void HeroClass::stopSubscreenFalling(bool v){
	preventsubscreenfalling = v;
}

void HeroClass::ClearhitHeroUIDs()
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
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) 	
	{
		/*
		if ( lastHitBy[q][1] == (frame-1) ) //Verify if this is needed at all now. 
		{
			//Z_scripterrlog("frame is: %d\n", frame);
			//Z_scripterrlog("Player->HitBy frame is: %d\n", lastHitBy[q][1]);
			lastHitBy[q][0] = 0;
		}
		*/
		lastHitBy[q][0] = 0;
	}
}

void HeroClass::sethitHeroUID(int32_t type, int32_t screen_index)
{
	lastHitBy[type][0] = screen_index;
}

int32_t HeroClass::gethitHeroUID(int32_t type)
{
	return lastHitBy[type][0];
}

void HeroClass::set_defence(int32_t type, int32_t v)
{
	defence[type] = v;
}

int32_t HeroClass::get_defence(int32_t type)
{
	return defence[type];
}


//Set Hero;s hurt sfx
void HeroClass::setHurtSFX(int32_t sfx)
{
	QMisc.miscsfx[sfxHURTPLAYER] = sfx;
}	
int32_t HeroClass::getHurtSFX()
{
	return QMisc.miscsfx[sfxHURTPLAYER];
}

bool  HeroClass::getDiagMove()
{
    return diagonalMovement;
}
void HeroClass::setDiagMove(bool newdiag)
{
    diagonalMovement=newdiag;
}
bool  HeroClass::getBigHitbox()
{
    return bigHitbox;
}
void HeroClass::setBigHitbox(bool newbigHitbox)
{
    bigHitbox=newbigHitbox;
	syofs = bigHitbox?0:8;
	sysz_ofs = bigHitbox?0:-8;
}
int32_t HeroClass::getStepRate()
{
    return steprate;
}
void HeroClass::setStepRate(int32_t newrate)
{
    steprate = newrate;
}
int32_t HeroClass::getSwimUpRate()
{
    return game->get_sideswim_up();
}
void HeroClass::setSwimUpRate(int32_t newrate)
{
    game->set_sideswim_up(newrate);
}
int32_t HeroClass::getSwimSideRate()
{
    return game->get_sideswim_side();
}
void HeroClass::setSwimSideRate(int32_t newrate)
{
    game->set_sideswim_side(newrate);
}
int32_t HeroClass::getSwimDownRate()
{
    return game->get_sideswim_down();
}
void HeroClass::setSwimDownRate(int32_t newrate)
{
    game->set_sideswim_down(newrate);
}


//void HeroClass::herostep() { lstep = lstep<(BSZ?27:11) ? lstep+1 : 0; }
void HeroClass::herostep()
{
    lstep = lstep<((zinit.heroAnimationStyle==las_bszelda)?27:11) ? lstep+1 : 0;
	//need to run all global/hero/dmap scripts here?
}

bool is_moving()
{
    return DrunkUp()||DrunkDown()||DrunkLeft()||DrunkRight();
}

// called by ALLOFF()
void HeroClass::resetflags(bool all)
{
    refilling=REFILL_NONE;
    inwallm=false;
    inlikelike=blowcnt=whirlwind=specialcave=hclk=fairyclk=refill_why=didstuff=0;
	usecounts.clear();
    
	if(swordclk>0 || all)
	{
		swordclk=0;
		verifyAWpn();
	}
    if(itemclk>0 || all)
        itemclk=0;
    if(shieldjinxclk>0 || all)
        shieldjinxclk=0;
        
    if(all)
    {
        DivineProtectionShieldClk=0;
        
        if(div_prot_item != -1)
        {
            stop_sfx(itemsbuf[div_prot_item].usesound);
            stop_sfx(itemsbuf[div_prot_item].usesound+1);
        }
        
        div_prot_item = -1;
        hoverclk=jumping=0;
		hoverflags = 0;
    }
    damageovertimeclk = 0;
    newconveyorclk = 0;
    switchhookclk = switchhookstyle = switchhookarg = switchhookmaxtime = 0;
	for(auto q = 0; q < 7; ++q)
		hooked_undercombos[q] = -1;
    hopclk=0;
    hopdir=-1;
    attackclk=0;
    stomping=false;
    reset_swordcharge();
    diveclk=drownclk=drownCombo=0;
    action=none; FFCore.setHeroAction(none);
    conveyor_flags=0;
    magiccastclk=0;
    magicitem=-1;
}

//Can use this for Hero->Stun. -Z
void HeroClass::Freeze()
{
    if (action != inwind)
    {
        if (IsSideSwim()) {action=sideswimfreeze; FFCore.setHeroAction(sideswimfreeze);} 
	else {action=freeze; FFCore.setHeroAction(freeze);} 
        // also cancel Hero's attack
        attackclk = 0;
    }
}
void HeroClass::unfreeze()
{
    if(action==freeze && fairyclk<1) { action=none; FFCore.setHeroAction(none); }
    if(action==sideswimfreeze && fairyclk<1) { action=sideswimming; FFCore.setHeroAction(sideswimming); }
}

void HeroClass::Drown(int32_t state)
{
	// Hero should never drown if the ladder is out
	if(ladderx+laddery)
		return;
	
	drop_liftwpn();
	switch(state)
	{
		case 1:
			action=lavadrowning; FFCore.setHeroAction(lavadrowning);
			attackclk=0;
			attack=wNone;
			attackid=-1;
			reset_swordcharge();
			drownclk=64;
			z=fakez=fall=fakefall=0;
			break;

		
		default:
		{
			if (isSideViewHero() && get_qr(qr_SIDESWIM)){action=sidedrowning; FFCore.setHeroAction(sidedrowning);}
			else {action=drowning; FFCore.setHeroAction(drowning);}
			attackclk=0;
			attack=wNone;
			attackid=-1;
			reset_swordcharge();
			drownclk=64;
			z=fakez=fall=fakefall=0;
			break;
		}
	}
	
}

void HeroClass::finishedmsg()
{
    //these are to cancel out any keys that Hero may
    //be pressing so he doesn't attack at the end of
    //a message if he was scrolling through it quickly.
    rAbtn();
    rBbtn();
    unfreeze();
    
    if(action == landhold1 ||
            action == landhold2 ||
            action == waterhold1 ||
            action == waterhold2 ||
            action == sidewaterhold1 ||
            action == sidewaterhold2)
    {
        holdclk = 1;
    }
}
void HeroClass::setEaten(int32_t i)
{
    inlikelike=i;
}
int32_t HeroClass::getEaten()
{
    return inlikelike;
}
zfix  HeroClass::getX()
{
    return x;
}
zfix  HeroClass::getY()
{
    return y;
}
zfix  HeroClass::getZ()
{
    return z;
}
zfix  HeroClass::getFakeZ()
{
    return fakez;
}
zfix  HeroClass::getFall()
{
    return fall;
}
zfix  HeroClass::getFakeFall()
{
    return fakefall;
}
zfix  HeroClass::getXOfs()
{
    return xofs;
}
zfix  HeroClass::getYOfs()
{
    return yofs;
}
void HeroClass::setXOfs(int32_t newxofs)
{
    xofs=newxofs;
}
void HeroClass::setYOfs(int32_t newyofs)
{
    yofs=newyofs;
}
int32_t  HeroClass::getHXOfs()
{
    return hxofs;
}
int32_t  HeroClass::getHYOfs()
{
    return hyofs;
}
int32_t  HeroClass::getHXSz()
{
    return hit_width;
}
int32_t  HeroClass::getHYSz()
{
    return hit_height;
}
zfix  HeroClass::getClimbCoverX()
{
    return climb_cover_x;
}
zfix  HeroClass::getClimbCoverY()
{
    return climb_cover_y;
}
int32_t  HeroClass::getLadderX()
{
    return ladderx;
}
int32_t  HeroClass::getLadderY()
{
    return laddery;
}

void HeroClass::setX(int32_t new_x)
{
    zfix dx=new_x-x;
    justmoved = 2;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=dx;
	hs_startx+=(int32_t)dx;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=dx;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int32_t j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->x+=dx;
        }
	}
    
    x=new_x;
    
    // A kludge
    if(!diagonalMovement && dir<=down)
        is_on_conveyor=true;
}

void HeroClass::setY(int32_t new_y)
{
    zfix dy=new_y-y;
    justmoved = 2;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=dy;
	hs_starty+=(int32_t)dy;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=dy;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int32_t j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->y+=dy;
        }
	}
    
    y=new_y;
    
    // A kludge
    if(!diagonalMovement && dir>=left)
        is_on_conveyor=true;
}

void HeroClass::setZ(int32_t new_z)
{
    if(isSideViewHero())
        return;
        
    if(z==0 && new_z > 0)
    {
        switch(action)
        {
        case swimming:
	{
            diveclk=0;
            action=walking; FFCore.setHeroAction(walking);
            break;
	}
            
        case waterhold1:
	{
            action=landhold1; FFCore.setHeroAction(landhold1);
            break;
	}
            
        case waterhold2:
	{
            action=landhold2; FFCore.setHeroAction(landhold2);
            break;
	}
            
        default:
            if(charging) //!DIMITODO: Let Hero jump while charging sword
            {
                reset_swordcharge();
                attackclk=0;
            }
            
            break;
        }
    }
    
    z=(new_z>0 ? new_z : 0);
}

void HeroClass::setFakeZ(int32_t new_z)
{
    if(isSideViewHero())
        return;
        
    if(fakez==0 && new_z > 0)
    {
        switch(action)
        {
        case swimming:
	{
            diveclk=0;
            action=walking; FFCore.setHeroAction(walking);
            break;
	}
            
        case waterhold1:
	{
            action=landhold1; FFCore.setHeroAction(landhold1);
            break;
	}
            
        case waterhold2:
	{
            action=landhold2; FFCore.setHeroAction(landhold2);
            break;
	}
            
        default:
            if(charging) //!DIMITODO: Let Hero jump while charging sword
            {
                reset_swordcharge();
                attackclk=0;
            }
            
            break;
        }
    }
    
    fakez=(new_z>0 ? new_z : 0);
}

void HeroClass::setXfix(zfix new_x)
{
	//Z_scripterrlog("setxdbl: %f\n",new_x);
    zfix dx=new_x-x;
    justmoved = 2;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->x+=dx;
	hs_startx+=(int32_t)dx;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->x+=dx;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int32_t j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->x+=dx;
        }
	}
    
    x=new_x;
    
    // A kludge
    if(!diagonalMovement && dir<=down)
        is_on_conveyor=true;
}

void HeroClass::setYfix(zfix new_y)
{
    zfix dy=new_y-y;
    justmoved = 2;
    if(Lwpns.idFirst(wHookshot)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHookshot))->y+=dy;
	hs_starty+=(int32_t)dy;
    }
    
    if(Lwpns.idFirst(wHSHandle)>-1)
    {
        Lwpns.spr(Lwpns.idFirst(wHSHandle))->y+=dy;
    }
	
	if(chainlinks.Count()>0)
	{
		for(int32_t j=0; j<chainlinks.Count(); j++)
        {
            chainlinks.spr(j)->y+=dy;
        }
	}
    
    y=new_y;
    
    // A kludge
    if(!diagonalMovement && dir>=left)
        is_on_conveyor=true;
}

void HeroClass::setZfix(zfix new_z)
{
    if(isSideViewHero())
        return;
        
    if(z==0 && new_z > 0)
    {
        switch(action)
        {
        case swimming:
	{
            diveclk=0;
            action=walking; FFCore.setHeroAction(walking);
            break;
	}
            
        case waterhold1:
	{
            action=landhold1; FFCore.setHeroAction(landhold1);
            break;
	}
            
        case waterhold2:
	{
            action=landhold2; FFCore.setHeroAction(landhold2);
            break;
	}
            
        default:
            if(charging) //!DIMITODO: Let Hero jump while charging sword
            {
                reset_swordcharge();
                attackclk=0;
            }
            
            break;
        }
    }
    
    z=(new_z>0 ? new_z : 0_zf);
}

void HeroClass::setFakeZfix(zfix new_z)
{
    if(isSideViewHero())
        return;
        
    if(fakez==0 && new_z > 0)
    {
        switch(action)
        {
        case swimming:
	{
            diveclk=0;
            action=walking; FFCore.setHeroAction(walking);
            break;
	}
            
        case waterhold1:
	{
            action=landhold1; FFCore.setHeroAction(landhold1);
            break;
	}
            
        case waterhold2:
	{
            action=landhold2; FFCore.setHeroAction(landhold2);
            break;
	}
            
        default:
            if(charging) //!DIMITODO: Let Hero jump while charging sword
            {
                reset_swordcharge();
                attackclk=0;
            }
            
            break;
        }
    }
    
    fakez=(new_z>0 ? new_z : 0_zf);
}

void HeroClass::setFall(zfix new_fall)
{
    fall=new_fall;
    justmoved = 2;
    jumping=-1;
}
void HeroClass::setFakeFall(zfix new_fall)
{
    fakefall=new_fall;
    jumping=-1;
}
void HeroClass::setClimbCoverX(int32_t new_x)
{
    climb_cover_x=new_x;
}
void HeroClass::setClimbCoverY(int32_t new_y)
{
    climb_cover_y=new_y;
}
int32_t  HeroClass::getLStep()
{
    return lstep;
}
int32_t  HeroClass::getCharging()
{
    return charging;
}
bool HeroClass::isCharged()
{
    return spins>0;
}
int32_t  HeroClass::getAttackClk()
{
    return attackclk;
}
void  HeroClass::setAttackClk(int32_t new_clk)
{
    attackclk=new_clk;
}
void HeroClass::setCharging(int32_t new_charging)
{
    charging=new_charging;
}
int32_t  HeroClass::getSwordClk()
{
    return swordclk;
}
int32_t  HeroClass::getItemClk()
{
    return itemclk;
}
void HeroClass::setSwordClk(int32_t newclk)
{
    swordclk=newclk;
	verifyAWpn();
}
void HeroClass::setItemClk(int32_t newclk)
{
    itemclk=newclk;
}
// TODO remove, no longer needed.
zfix  HeroClass::getModifiedX()
{
    zfix tempx=x;
    return tempx;
}

zfix  HeroClass::getModifiedY()
{
    zfix tempy=y;
    return tempy;
}

int32_t HeroClass::getDir()
{
    return dir;
}
void HeroClass::setDir(int32_t newdir)
{
    dir=newdir;
    reset_hookshot();
}
int32_t  HeroClass::getHitDir()
{
    return hitdir;
}
void HeroClass::setHitDir(int32_t newdir)
{
    hitdir = newdir;
}
int32_t  HeroClass::getClk()
{
    return clk;
}
int32_t  HeroClass::getPushing()
{
    return pushing;
}
void HeroClass::Catch()
{
    if(!inwallm && (action==none || action==walking))
    {
        SetAttack();
        attackclk=0;
        attack=wCatching;
    }
}

bool HeroClass::getClock()
{
    return superman;
}
void HeroClass::setClock(bool state)
{
    superman=state;
}
int32_t  HeroClass::getAction() // Used by ZScript
{
    if(spins > 0)
        return isspinning;
    else if(charging > 0)
        return ischarging;
    else if(diveclk > 0)
        return isdiving;
    //else if (pushing > 0) return ispushing; //Needs a QR? -Z or make it an instruction as Hero->Pushing? Probably better, as that has a clk?? 
        
    return action;
}

int32_t  HeroClass::getAction2() // Used by ZScript new FFCore.actions
{
    if(spins > 0)
        return isspinning;
    else if(charging > 0)
        return ischarging;
    else if(diveclk > 0)
        return isdiving;
    //else if (pushing > 0) return ispushing; //Needs a QR? -Z or make it an instruction as Hero->Pushing? Probably better, as that has a clk?? 
        
    return -1;
}

void HeroClass::setAction(actiontype new_action) // Used by ZScript
{
	if(new_action==dying || new_action==won || new_action==scrolling ||
	   new_action==inwind || new_action==ischarging || new_action==sideswimischarging || 
	   new_action==hopping) //!DIMITODO: allow setting sideswimming stuff
		return; // Can't use these actions.
	
	if (!isSideViewHero() && (new_action>=sideswimming && new_action <= sideswimischarging))
		return;
	
	if(new_action==rafting)
	{
		if(get_qr(qr_DISALLOW_SETTING_RAFTING)) return;
		if(!(isRaftFlag(nextflag(x+8,y+8,dir,false))||isRaftFlag(nextflag(x+8,y+8,dir,true))))
			return;
	}
	
	
	if(magicitem>-1 && itemsbuf[magicitem].family==itype_divineescape)
	{
		// Using Divine Escape
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
	else if(action==freeze||action==sideswimfreeze) // Might be in enemy wind
	{
		sprite* wind=0;
		bool foundWind=false;
		for(int32_t i=0; i<Ewpns.Count(); i++)
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
	
	//Unless compat rule is on, reset hopping clocks when writing action!
	if(action == hopping && !get_qr(qr_NO_OVERWRITING_HOPPING))
	{
		hopclk = 0;
		hopdir = -1;
	}
	
	if(new_action != attacking && new_action != sideswimattacking)
	{
		attackclk=0;
		
		if(attack==wHookshot)
			reset_hookshot();
	}
	if(new_action != isspinning && new_action != sideswimisspinning)
	{
		charging = 0;
		spins = 0;
	}
	
	if(action == falling && new_action != falling)
	{
		fallclk = 0; //Stop falling;
	}
	
	if (action == rafting && new_action != rafting)
	{
		raftwarpx = x;//If you wanted to make Link stop rafting on a dock combo, don't make the dock retrigger the raft.
		raftwarpy = y;
	}
	
	switch(new_action)
	{
	case isspinning:
	case sideswimisspinning:
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
		{
			int32_t flippers_id = current_item_id(itype_flippers);
			diveclk = (flippers_id < 0 ? 80 : (itemsbuf[flippers_id].misc1 + itemsbuf[flippers_id].misc2)); // Who cares about qr_NODIVING? It's the questmaker's business.
		}
		return;
		
	case drowning:
	case sidedrowning:
	//I would add a sanity check to see if Hero is in water, but I *KNOW* that quests have used this 
	// INTENTIONALLY while Hero is on Land, as a blink-out effect. :( -Z
		if(!drownclk)
			Drown();
			
		break;
	
	case lavadrowning:
		//Lavadrowning is just drowning but with a different argument. Simplicity! -Dimi
		if(!drownclk)
			Drown(1);
			
		break;
		
	case falling:
		if(!fallclk)
		{
			//If there is a pit under Hero, use its combo.
			if(int32_t c = getpitfall(x+8,y+(bigHitbox?8:12))) fallCombo = c;
			else if(int32_t c = getpitfall(x,y+(bigHitbox?0:8))) fallCombo = c;
			else if(int32_t c = getpitfall(x+15,y+(bigHitbox?0:8))) fallCombo = c;
			else if(int32_t c = getpitfall(x,y+15)) fallCombo = c;
			else if(int32_t c = getpitfall(x+15,y+15)) fallCombo = c;
			//Else, use a null value; triggers default pit values
			else fallCombo = 0;
			fallclk = PITFALL_FALL_FRAMES;
		}
		break;
		
	case gothit:
	case swimhit:
	case sideswimhit:
		if(!hclk)
			hclk=48;
			
		break;
		
	case landhold1:
	case landhold2:
	case waterhold1:
	case waterhold2:
	case sidewaterhold1:
	case sidewaterhold2:
		if(!holdclk)
			holdclk=130;
			
		attack=none;
		break;
		
	case attacking:
	case sideswimattacking:
		attack=none;
		break;
		
	default:
		break;
	}
	
	action=new_action; FFCore.setHeroAction(new_action);
}

void HeroClass::setHeldItem(int32_t newitem)
{
    holditem=newitem;
}
int32_t HeroClass::getHeldItem()
{
    return holditem;
}
bool HeroClass::isDiving()
{
	int32_t flippers_id = current_item_id(itype_flippers);
    return (diveclk > (flippers_id < 0 ? 30 : itemsbuf[flippers_id].misc2));
}
bool HeroClass::isSwimming()
{
    return ((action==swimming)||(action==sideswimming)||IsSideSwim()||
            (action==waterhold1)||(action==waterhold2)||
            (hopclk==0xFF));
}

void HeroClass::setDontDraw(byte new_dontdraw)
{
    dontdraw=new_dontdraw;
}

byte HeroClass::getDontDraw()
{
    return dontdraw;
}

void HeroClass::setHClk(int32_t newhclk)
{
    hclk=newhclk;
}

int32_t HeroClass::getHClk()
{
    return hclk;
}

int32_t HeroClass::getSpecialCave()
{
    return specialcave;    // used only by maps.cpp
}

void HeroClass::init()
{
	usecounts.clear();
	scale = 0;
	rotation = 0;
	do_animation = 1;
	if(lift_wpn)
	{
		delete lift_wpn;
		lift_wpn = nullptr;
	}
	liftclk = 0;
	tliftclk = 0;
	liftheight = 0;
	liftflags = 0;
    if ( dontdraw != 2 ) {  dontdraw = 0; } //scripted dontdraw == 2, normal == 1, draw hero == 0
    hookshot_used=false;
    justmoved = 0;
    hookshot_frozen=false;
    onpassivedmg=false;
    dir = up;
    damageovertimeclk = 0;
    newconveyorclk = 0;
    switchhookclk = switchhookstyle = switchhookarg = switchhookmaxtime = 0;
	for(auto q = 0; q < 7; ++q)
		hooked_undercombos[q] = -1;
    shiftdir = -1;
    sideswimdir = right;
    holddir = -1;
    landswim = 0;
    sdir = up;
    ilswim=true;
    walkable=false;
    moveflags = FLAG_OBEYS_GRAV | FLAG_CAN_PITFALL | FLAG_CAN_WATERDROWN;
    warp_sound = 0;
    subscr_speed = zinit.subscrSpeed;
	steprate = zinit.heroStep;
	shove_offset = zinit.shove_offset;
	is_warping = false;
	coyotetime = 0;
	
	hammer_swim_up_offset = hammeroffsets[0];
	hammer_swim_down_offset = hammeroffsets[1];
	hammer_swim_left_offset = hammeroffsets[2];
	hammer_swim_right_offset = hammeroffsets[3];
	
	prompt_combo = prompt_x = prompt_y = prompt_cset = 0;
    
    if(get_qr(qr_NOARRIVALPOINT))
    {
        x=tmpscr->warpreturnx[0];
        y=tmpscr->warpreturny[0];
    }
    else
    {
        x=tmpscr->warparrivalx;
        y=tmpscr->warparrivaly;
    }
    
    z=fakez=fall=fakefall=0;
    hzsz = 12; // So that flying peahats can still hit him.
    
    if(x==0)   dir=right;
    
    if(x==world_w-16) dir=left;
    
    if(y==0)   dir=down;
    
    if(y==world_h-16) dir=up;
    
    lstep=0;
    skipstep=0;
    autostep=false;
    attackclk=holdclk=hoverclk=jumping=raftclk=0;
    attack=wNone;
    attackid=-1;
    action=none; FFCore.setHeroAction(none); tempaction=none;
    xofs=0;
    yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset);
    cs=6;
    pushing=fairyclk=0;
    id=0;
    inlikelike=0;
    superman=inwallm=false;
    scriptcoldet=1;
    blowcnt=whirlwind=specialcave=0;
    hopclk=diveclk=fallclk=0;
	fallCombo = 0;
	pit_pulldir = -1;
    hopdir=-1;
    conveyor_flags=0;
    drunkclk=0;
    lstunclock = 0;
	is_conveyor_stunned=0;
    convey_forcex=convey_forcey=0;
    drawstyle=3;
    ffwarp = false;
    stepoutindex=stepoutwr=stepoutdmap=stepoutscr=0;
    stepnext=stepsecret=rpos_t::None;
    ffpit = false;
    respawn_x=x;
    respawn_y=y;
	respawn_dmap=currdmap;
	respawn_scr=currscr;
    falling_oldy = y;
    magiccastclk=0;
    magicitem = div_prot_item = -1;
	last_lens_id = 0; //Should be -1 (-Z)
	last_savepoint_id = 0;
	misc_internal_hero_flags = 0;
	last_cane_of_byrna_item_id = -1;
	on_sideview_ladder = false;
	switchblock_z = 0;
	switchblock_offset = false;
	extra_jump_count = 0;
	hoverflags = 0;
    lbunnyclock = 0;
    
    for(int32_t i=0; i<32; i++) miscellaneous[i] = 0;
    
    setBigHitbox(get_qr(qr_LTTPCOLLISION));
    diagonalMovement=(get_qr(qr_LTTPWALK));
    
	shield_active = false;
	shield_forcedir = -1;
	active_shield_id = -1;
	conv_forcedir = -1;
	
    //2.6
	preventsubscreenfalling = false;  //-Z
	walkspeed = 0; //not used, yet. -Z
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][0] = 0; 
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][1] = 0; 
	for ( int32_t q = 0; q < wMax; q++ ) 
	{
		defence[q] = hero_defence[q]; //we will need to have a Hero section in the quest load/save code! -Z Added 3/26/21 - Jman
		//zprint2("defence[%d] is: %d\n", q, defence[q]);
	}
	//Run script!
	if (( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) && (game->get_hasplayed()) ) //if (!hasplayed) runs in game_loop()
	{
		ZScriptVersion::RunScript(ScriptType::Player, SCRIPT_PLAYER_INIT); 
		FFCore.deallocateAllScriptOwned(ScriptType::Player, SCRIPT_PLAYER_INIT);
		FFCore.initZScriptHeroScripts(); //Clear the stack and the refinfo data to be ready for Hero's active script. 
		set_respawn_point(); //screen entry at spawn; //This should be after the init script, so that Hero->X and Hero->Y set by the script
						//are properly set by the engine.
	}
	FFCore.nostepforward = 0;

	if (replay_version_check(12))
		z3step = 2;
}

void HeroClass::draw_under(BITMAP* dest)
{
    int32_t c_raft=current_item_id(itype_raft);
    int32_t c_ladder=current_item_id(itype_ladder);
    
    if(action==rafting && c_raft >-1)
    {
        if(((dir==left) || (dir==right)) && (get_qr(qr_RLFIX)))
        {
            overtile16(dest, itemsbuf[c_raft].tile, x - viewport.x, y+playing_field_offset+4 - viewport.y,
                       itemsbuf[c_raft].csets&15, rotate_value((itemsbuf[c_raft].misc_flags>>2)&3)^3);
        }
        else
        {
            overtile16(dest, itemsbuf[c_raft].tile, x - viewport.x, y+playing_field_offset+4 - viewport.y,
                       itemsbuf[c_raft].csets&15, (itemsbuf[c_raft].misc_flags>>2)&3);
        }
    }
    
    if(ladderx+laddery && c_ladder >-1)
    {
        if((ladderdir>=left) && (get_qr(qr_RLFIX)))
        {
            overtile16(dest, itemsbuf[c_ladder].tile, ladderx - viewport.x, laddery+playing_field_offset - viewport.y,
                       itemsbuf[c_ladder].csets&15, rotate_value((itemsbuf[c_ladder].misc_flags>>2)&3)^3);
        }
        else
        {
            overtile16(dest, itemsbuf[c_ladder].tile, ladderx - viewport.x, laddery+playing_field_offset - viewport.y,
                       itemsbuf[c_ladder].csets&15, (itemsbuf[c_ladder].misc_flags>>2)&3);
        }
    }
}

void HeroClass::drawshadow(BITMAP* dest, bool translucent)
{
    int32_t tempy=yofs;
    yofs+=8;
    shadowtile = wpnsbuf[spr_shadow].tile;
    sprite::drawshadow(dest,translucent);
    yofs=tempy;
}

// The Stone of Agony reacts to these flags.
bool HeroClass::agonyflag(int32_t flag)
{
    switch(flag)
    {
    case mfWHISTLE:
    case mfANYFIRE:
    case mfARROW:
    case mfBOMB:
    case mfSBOMB:
    case mfBRANG:
    case mfMBRANG:
    case mfFBRANG:
    case mfSARROW:
    case mfGARROW:
    case mfSTRONGFIRE:
    case mfMAGICFIRE:
    case mfDIVINEFIRE:
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
int32_t HeroClass::weaponattackpower(int32_t itid)
{
	if(itid < 0)
	{
		itid = current_item_id(attack==wCByrna ? itype_cbyrna
			: attack==wWand ? itype_wand
			: attack==wHammer ? itype_hammer
			: itype_sword);
	}
	int32_t power = attack==wCByrna ? itemsbuf[itid].misc4 : itemsbuf[itid].power;
	
	// Multiply it by the power of the spin attack/quake hammer, if applicable.
	if(spins > 0)
	{
		int scr = currentscroll;
		if(scr < 0)
		{
			scr = current_item_id(attack==wHammer ? (spins>1?itype_quakescroll2:itype_quakescroll)
				: (spins>5 || current_item_id(itype_spinscroll) < 0)
					? itype_spinscroll2 : itype_spinscroll);
		}
		power *= itemsbuf[scr].power;
	}
	else currentscroll = -1;
	return power;
}

#define NET_CLK_TOTAL 24
#define NET_DIR_INC (NET_CLK_TOTAL/3)
// Must only be called once per frame!
void HeroClass::positionNet(weapon *w, int32_t itemid)
{
	itemid = vbound(itemid, 0, MAXITEMS-1);
	int32_t t = w->o_tile,
		wx = 1, wy = 1;
	
	//Invert positioning clock if right-handed animation
	int32_t clock = (itemsbuf[itemid].flags&ITEM_FLAG2 ? (NET_CLK_TOTAL-1)-attackclk : attackclk);
	if(clock >= NET_CLK_TOTAL)
		w->dead = 0;
	int32_t tiledir = dir;
	switch(dir)
	{
		case up:
		{
			if(clock < NET_DIR_INC) tiledir = l_up;
			else if(clock >= NET_DIR_INC*2) tiledir = r_up;
			break;
		}
		case down:
		{
			if(clock < NET_DIR_INC) tiledir = r_down;
			else if(clock >= NET_DIR_INC*2) tiledir = l_down;
			break;
		}
		case left:
		{
			if(clock < NET_DIR_INC) tiledir = l_down;
			else if(clock >= NET_DIR_INC*2) tiledir = l_up;
			break;
		}
		case right:
		{
			if(clock < NET_DIR_INC) tiledir = r_up;
			else if(clock >= NET_DIR_INC*2) tiledir = r_down;
			break;
		}
	}
	int32_t offs = 0;
	if(tiledir > right)
		offs = ((clock%NET_DIR_INC)<NET_DIR_INC/2) ? 1 : 0;
	else offs = vbound(((clock%NET_DIR_INC)/(NET_DIR_INC/3))-1,-1,1);
	//One of 8 positions
	switch(tiledir)
	{
		case up:
		{
			wx = 6*offs;
			wy = -14;
			break;
		}
		case r_up:
		{
			wx = (offs ? 10 : 14);
			wy = (offs ? -12 : -10);
			break;
		}
		case right:
		{
			wx = 14;
			wy = 6*offs;
			break;
		}
		case r_down:
		{
			wx = (offs ? 14 : 10);
			wy = (offs ? 10 : 12);
			break;
		}
		case down:
		{
			wx = -6*offs;
			wy = 14;
			break;
		}
		case l_down:
		{
			wx = (offs ? -10 : -14);
			wy = (offs ? 12 : 10);
			break;
		}
		case left:
		{
			wx = -14;
			wy = -6*offs;
			break;
		}
		case l_up:
		{
			wx = (offs ? -14 : -10);
			wy = (offs ? -10 : -12);
			break;
		}
	}
	
    w->x = x+wx;
    w->y = y+wy-(original_playing_field_offset-2-yofs)-fakez;
    w->z = (z+zofs);
    w->fakez = fakez;
    w->tile = t+tiledir;
    w->power = 0;
    w->dir = dir;
    w->doAutoRotate(true);
}
void HeroClass::positionSword(weapon *w, int32_t itemid)
{
	//if ( w->ScriptGenerated ) return; //t/b/a for script-generated swords.
	//if ( itemsbuf[itemid].ScriptGenerated ) return; //t/b/a for script-generated swords.
    itemid=vbound(itemid, 0, MAXITEMS-1);
    // Place a sword weapon at the right spot.
    int32_t wy=1;
    int32_t wx=1;
    int32_t f=0,t,cs2;
    
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
        f=get_qr(qr_SWORDWANDFLIPFIX)?3:2;
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
        int32_t wpn2=itemsbuf[itemid].wpn2;
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
                t = wpnsbuf[wpn2].tile;
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
                t = wpnsbuf[wpn2].tile;
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
                t = wpnsbuf[wpn2].tile;
                cs2 = wpnsbuf[wpn2].csets&15;
                t+=2;
                f=0;
            }
            
            break;
            
        case right:
            --t;
            
            if(spins>0 || (itemsbuf[itemid].flags & ITEM_FLAG8))
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
                t = wpnsbuf[wpn2].tile;
                cs2 = wpnsbuf[wpn2].csets&15;
                
                if(spins>0 || (itemsbuf[itemid].flags & ITEM_FLAG8))
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
    
    int32_t itemid2 = current_item_id(itype_chargering);
    
    if(charging>(itemid2>=0 ? itemsbuf[itemid2].misc1 : 64))
    {
        cs2=(BSZ ? (frame&3)+6 : ((frame>>2)&1)+7);
    }
    
    /*if(BSZ || ((isdungeon() && currscr<128) && !get_qr(qr_HERODUNGEONPOSFIX)))
    {
      wy+=2;
    }*/
    w->x = x+wx;
    w->y = y+wy-(original_playing_field_offset-2-(yofs+slashyofs))-fakez;
    w->z = (z+zofs);
    w->tile = t;
    w->flip = f;
    w->power = weaponattackpower(itemid);
    w->dir = dir;
    w->doAutoRotate(true);
}

int HeroClass::getHammerState() const
{
	if(attack == wHammer)
	{
		if(attackclk >= 15)
			return 3;
		if(attackclk >= 13)
			return 2;
		return 1;
	}
	return 0;
}
weapon* find_first_wtype(int wtype)
{
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon* w = (weapon*)Lwpns.spr(i);
		
		if(w->id == wtype)
			return w;
	}
	return nullptr;
}
void HeroClass::draw(BITMAP* dest)
{
	/*{
		char buf[36];
		//sprintf(buf,"%d %d %d %d %d %d %d",dir, action, attack, attackclk, charging, spins, tapping);
		textout_shadowed_ex(framebuf,font, buf, 2,72,WHITE,BLACK,-1);
	}*/
	int32_t oxofs = xofs, oyofs = yofs;
	bool shieldModify = false;
	bool invisible=(dontdraw>0) || (tmpscr->flags3&fINVISHERO);
	
	{
		if(action==dying)
		{
			if(!invisible)
			{
				if ( script_hero_cset > -1 ) cs = script_hero_cset;
					sprite::draw(dest);
			}
			goto herodraw_end;
		}
		
		bool useltm=(get_qr(qr_EXPANDEDLTM) != 0);
		
		
		if(!invisible)
			yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
			
		// Stone of Agony
		bool agony=false;
		int32_t agonyid = current_item_id(itype_agony);
		
		if(!invisible)
		{	
			if(agonyid>-1)
			{
				int32_t power=itemsbuf[agonyid].power;
				int32_t left=static_cast<int32_t>(x+8-power)&0xF0; // Check top-left pixel of each tile
				int32_t right=(static_cast<int32_t>(x+8+power)&0xF0)+16;
				int32_t top=static_cast<int32_t>(y+(bigHitbox ? 8 : 12)-power)&0xF0;
				int32_t bottom=(static_cast<int32_t>(y+(bigHitbox ? 8 : 12)+power)&0xF0)+16;
				
				for(int32_t x=left; x<right; x+=16)
				{
					for(int32_t y=top; y<bottom; y+=16)
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
			if ( script_hero_cset > -1 ) cs = script_hero_cset;
			if(!get_qr(qr_HEROFLICKER))
			{
				if(superman && getCanFlicker())
				{
					cs += (((~frame)>>1)&3);
				}
				else if(hclk&&(DivineProtectionShieldClk<=0) && getCanFlicker())
				{
					cs += ((hclk>>1)&3);
				}
			}
		}
		
		if(attackclk || (action==attacking||action==sideswimattacking))
		{
			/* Spaghetti code constants!
			* - Hero.attack contains a weapon type...
			* - which must be converted to an itype...
			* - which must be converted to an item ID...
			* - which is used to acquire a wpn ID! Aack!
			*/
			int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : attack==wBugNet ? itype_bugnet : itype_sword);
			int32_t itemid = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
			itemid=vbound(itemid, 0, MAXITEMS-1);
			// if ( itemsbuf[itemid].ScriptGenerated ) return; //t/b/a for script-generated swords.
			if(attackclk>4||attack==wBugNet||(attack==wSword&&game->get_canslash()))
			{
				if(attack == wBugNet)
				{
					weapon *w=NULL;
					bool found = false;
					for(int32_t q = 0; q < Lwpns.Count(); ++q)
					{
						w = (weapon*)Lwpns.spr(q);
						if(w->id == wBugNet)
						{
							found = true;
							break;
						}
					}
					if(!found)
					{
						Lwpns.add(new weapon((zfix)0,(zfix)0,(zfix)0,wBugNet,0,0,dir,itemid,getUID(),false,false,true));
						
						w = (weapon*)Lwpns.spr(Lwpns.Count()-1);
					}
					positionNet(w, itemid);
				}
				else if((attack==wSword || attack==wWand || ((attack==wFire || attack==wCByrna) && itemsbuf[itemid].wpn)) && wpnsbuf[itemsbuf[itemid].wpn].tile)
				{
					// Create a sword weapon at the right spot.
					weapon *w=find_first_wtype(attack==wSword ? wSword : wWand);
					if(!w)  // Create one if sword nonexistant
					{
						Lwpns.add(new weapon((zfix)0,(zfix)0,(zfix)0,(attack==wSword ? wSword : wWand),0,0,dir,itemid,getUID(),false,false,true));
						w = (weapon*)Lwpns.spr(Lwpns.Count()-1);
						
						positionSword(w,itemid);
						
						// Stone of Agony
						if(agony)
						{
							w->y-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,2))?1:0;
						}
					}
					
					// These are set by positionSword(), above or in checkstab()
					yofs += slashyofs;
					xofs += slashxofs;
					slashyofs = slashxofs = 0;
				}
			}
			
			if(attackclk<7
			|| (attack==wSword && ((attackclk<(game->get_canslash()?15:13) 
			|| FIXED_Z3_ANIMATION && attackclk<(game->get_canslash()?16:12))
			|| (charging>0 && attackclk!=SWORDCHARGEFRAME)))
			|| ((attack==wWand || attack==wFire || attack==wCByrna) && attackclk<13)
			|| (attack==wHammer && attackclk<=30)
			|| (attack==wBugNet && attackclk<NET_CLK_TOTAL))
			{
				if(!invisible)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimstab:ls_stab, dir, zinit.heroAnimationStyle);
					if (FIXED_Z3_ANIMATION)
					{
						if (attackclk >= 2) tile += (extend==2?2:1);
						if (attackclk >= 13) tile += (extend==2?2:1);
					}
					
					if(((game->get_canslash() && (attack==wSword || attack==wWand || attack==wFire || attack==wCByrna)) && itemsbuf[itemid].flags&ITEM_FLAG4 && (attackclk<7||FIXED_Z3_ANIMATION&&(attackclk < 16))))
					{
						herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimslash:ls_slash, dir, zinit.heroAnimationStyle);
						if (FIXED_Z3_ANIMATION)
						{
							if (attackclk >= 7) tile += (extend==2?2:1);
							if (attackclk >= 11) tile += (extend==2?2:1);
							if (attackclk >= 14) tile += (extend==2?2:1);
						}
					}
					if (attack==wBugNet && !get_qr(qr_OLD_BUG_NET))
					{
						if ((dir == right && (itemsbuf[itemid].flags&ITEM_FLAG2)) || (dir != right && !(itemsbuf[itemid].flags&ITEM_FLAG2)))
						{
							if (attackclk < 9) herotile(&tile, &flip, &extend, ls_revslash, dir, zinit.heroAnimationStyle);
							if (attackclk > 15) herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimslash:ls_slash, dir, zinit.heroAnimationStyle);
						}
						else
						{
							if (attackclk < 9) herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimslash:ls_slash, dir, zinit.heroAnimationStyle);
							if (attackclk > 15) herotile(&tile, &flip, &extend, ls_revslash, dir, zinit.heroAnimationStyle);
						}
					}
					
					if((attack==wHammer) && (attackclk<13))
					{
						herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimpound:ls_pound, dir, zinit.heroAnimationStyle);
						if (FIXED_Z3_ANIMATION)
						{
							if (attackclk >= 14) tile += (extend==2?2:1);
							if (attackclk >= 16) tile += (extend==2?2:1);
						}
					}
					
					if(useltm)
					{
						if ( script_hero_sprite <= 0 ) tile+=getTileModifier();
					}
				
					// Stone of Agony
					if(agony)
					{
						yofs-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3))?1:0;
					}

					//Probably what makes Hero flicker, except for the QR check. What makes him flicker when that rule is off?! -Z
					
					//I'm pretty sure he doesn't flicker when the rule is off. Also, take note of the parenthesis after the ! in this if statement; I was blind and didn't see it, and thought this code did something completely different. -Deedee
					if (!(get_qr(qr_HEROFLICKER) && ((superman || hclk) && (frame & 1))))
					{
						masked_draw(dest);
					}

					//Prevent flickering -Z
					if (!getCanFlicker()) masked_draw(dest);
				}
				
				if(attack!=wHammer)
					goto herodraw_end;
			}
			
			if(attack==wHammer) // To do: possibly abstract this out to a positionHammer routine?
			{
				int32_t wy=1;
				int32_t wx=1;
				int32_t f=0,t,cs2;
				weapon *w=NULL;
				bool found = false;
				
				for(int32_t i=0; i<Lwpns.Count(); i++)
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
					Lwpns.add(new weapon((zfix)0,(zfix)0,(zfix)0,wHammer,0,0,dir,itemid,getUID(),false,false,true));
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
					if (IsSideSwim())wy+=hammer_swim_up_offset;
					
					if(attackclk>=13)
					{
						wx-=1;
						wy+=1;
						++t;
					}
					
					if(attackclk>=15)
					{
						if (IsSideSwim())wy-=hammer_swim_up_offset;
						++t;
					}
					
					break;
					
				case down:
					wx=3;
					wy=-14;
					if (IsSideSwim())wy+=hammer_swim_down_offset;
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
						if (IsSideSwim())wy-=hammer_swim_down_offset;
						++t;
					}
					
					break;
					
				case left:
					wx=0;
					wy=-14;
					if (IsSideSwim())wy+=hammer_swim_left_offset;
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
						if (IsSideSwim())wy-=hammer_swim_left_offset;
						++t;
					}
					
					break;
					
				case right:
					wx=0;
					wy=-14;
					if (IsSideSwim())wy+=hammer_swim_right_offset;
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
						if (IsSideSwim())wy-=hammer_swim_right_offset;
						++t;
					}
					
					break;
				}
				
				if(BSZ || ((isdungeon() && currscr<128) && !get_qr(qr_HERODUNGEONPOSFIX)))
				{
					wy+=2;
				}
				
				// Stone of Agony
				if(agony)
				{
					wy-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3))?1:0;
				}
				
				w->x = x+wx;
				w->y = y+wy-(original_playing_field_offset-2-yofs)-fakez;
				w->z = (z+zofs);
				w->tile = t;
				w->flip = f;
				w->hit_width=20;
				w->hit_height=20;
				
				if(dir>down)
				{
					w->hit_height-=6;
				}
				else
				{
					w->hit_width-=6;
					w->hyofs=4;
				}
				
				w->power = weaponattackpower(itemid);
				
				if(attackclk==15 && z==0 && fakez==0 && (sideviewhammerpound() || !isSideViewHero()))
				{
					sfx(((iswaterex(MAPCOMBO(x+wx+8,y+wy), currmap, currscr, -1, x+wx+8, y+wy, true) || COMBOTYPE(x+wx+8,y+wy)==cSHALLOWWATER) && get_qr(qr_MORESOUNDS)) ? WAV_ZN1SPLASH : itemsbuf[itemid].usesound,pan(x.getInt()));
				}
				
				goto herodraw_end;
			}
		}
		else if(!charging && !spins)  // remove the sword
		{
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				weapon *w = (weapon*)Lwpns.spr(i);
				
				if(w->id == wSword || w->id == wHammer || w->id==wWand)
					w->dead=1;
			}
		}
		
		if(invisible)
		{
			goto herodraw_end;
		}
		
		if(action != casting && action != sideswimcasting)
		{
			// Keep this consistent with checkspecial2, line 7800-ish...
			bool inwater = iswaterex(MAPCOMBO(x+4,y+9), currmap, currscr, -1, x+4, y+9, true, false)  && iswaterex(MAPCOMBO(x+4,y+15), currmap, currscr, -1, x+4, y+15, true, false) &&  iswaterex(MAPCOMBO(x+11,y+9), currmap, currscr, -1, x+11, y+9, true, false) && iswaterex(MAPCOMBO(x+11,y+15), currmap, currscr, -1, x+11, y+15, true, false);
			
			int32_t jumping2 = int32_t(jumping*((zinit.gravity2 / 100)/16.0));
			bool noliftspr = get_qr(qr_NO_LIFT_SPRITE);
			//if (jumping!=0) al_trace("%d %d %f %d\n",jumping,zinit.gravity,zinit.gravity/16.0,jumping2);
			switch(zinit.heroAnimationStyle)
			{
			case las_original:                                               //normal
				if(action==drowning)
				{
					if(inwater)
					{
						herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_drown, dir, zinit.heroAnimationStyle);
						if ( script_hero_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
					}
					else
					{
						goto herodraw_end;
					}
				}
				else if(action==lavadrowning)
				{
					herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_lavadrown, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
				}
				else if(action==sidedrowning)
				{
					herotile(&tile, &flip, &extend, ls_sidedrown, down, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
				}
				else if (action == sideswimming || action == sideswimhit)
				{
					herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if ( script_hero_sprite <= 0 ) ++flip;
						}
						else
						{
							if ( script_hero_sprite <= 0 ) extend==2?tile+=2:++tile;
						}
					}
				}
				else if(action==swimming || action==swimhit || hopclk==0xFF)
				{
					herotile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.heroAnimationStyle);
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if ( script_hero_sprite <= 0 ) ++flip;
						}
						else
						{
							if ( script_hero_sprite <= 0 ) extend==2?tile+=2:++tile;
						}
					}
					
					if(isDiving())
					{
						herotile(&tile, &flip, &extend, ls_dive, dir, zinit.heroAnimationStyle);
						if ( script_hero_sprite <= 0 ) tile+=((frame>>3) & 1)*(extend==2?2:1);
					}
				}
				else if(charging > 0 && attack != wHammer)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimcharge:ls_charge, dir, zinit.heroAnimationStyle);
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if ( script_hero_sprite <= 0 ) ++flip;
						}
						else
						{
							if ( script_hero_sprite <= 0 ) extend==2?tile+=2:++tile;
						}
					}
				}
				else if((z>0 || fakez>0 || isSideViewHero()) && jumping2>0 && jumping2<24 && game->get_life()>0 && action!=rafting)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile+=((int32_t)jumping2/8)*(extend==2?2:1);
				}
				else if(fallclk>0)
				{
					herotile(&tile, &flip, &extend, ls_falling, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile+=((PITFALL_FALL_FRAMES-fallclk)/10)*(extend==2?2:1);
				}
				else if(!noliftspr&&action==lifting&&isLifting())
				{
					herotile(&tile, &flip, &extend, ls_lifting, dir, zinit.heroAnimationStyle);
					if(script_hero_sprite <= 0)
					{
						auto frames = vbound(liftingspr[dir][spr_frames],1,255);
						auto speed = tliftclk/frames;
						if (speed < 1) speed = 1;
						auto curframe = (tliftclk - liftclk) / speed;
						if (!tliftclk) curframe = frames - 1;
						if(unsigned(curframe) < frames)
							tile += curframe * (extend == 2 ? 2 : 1);
					}
				}
				else
				{
					if(IsSideSwim())
						herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					else if(!noliftspr&&isLifting())
						herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
					else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					
					if(dir>up)
					{
						useltm=true;
						shieldModify=true;
					}
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if ( script_hero_sprite <= 0 ) ++flip;
						}
						else
						{
							if ( script_hero_sprite <= 0 ) extend==2?tile+=2:++tile;
						}
					}
				}
				
				break;
				
			case las_bszelda:                                               //BS
				if(action==drowning)
				{
					if(inwater)
					{
						herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_drown, dir, zinit.heroAnimationStyle);
						if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
					else
					{
						goto herodraw_end;
					}
				}
				else if (action == sidedrowning)
				{
					herotile(&tile, &flip, &extend, ls_sidedrown, down, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if(action==lavadrowning)
				{
					herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_lavadrown, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if (action == sideswimming || action == sideswimhit)
				{
					herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					
					if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if(action==swimming || action==swimhit || hopclk==0xFF)
				{
					if (get_qr(qr_COPIED_SWIM_SPRITES)) 
					{
						herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					}
					else
					{
						herotile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.heroAnimationStyle);
					}
					if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					
					if(isDiving())
					{
						if (get_qr(qr_COPIED_SWIM_SPRITES)) 
						{
							herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
						}
						else
						{
							herotile(&tile, &flip, &extend, ls_dive, dir, zinit.heroAnimationStyle);
						}
						if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
				}
				else if(charging > 0 && attack != wHammer)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimcharge:ls_charge, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if((z>0 || fakez>0 || isSideViewHero()) && jumping2>0 && jumping2<24 && game->get_life()>0)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile+=((int32_t)jumping2/8)*(extend==2?2:1);
				}
				else if(fallclk>0)
				{
					herotile(&tile, &flip, &extend, ls_falling, dir, zinit.heroAnimationStyle);
					if ( script_hero_sprite <= 0 ) tile += ((PITFALL_FALL_FRAMES-fallclk)/10)*(extend==2?2:1);
				}
				else if(!noliftspr&&action==lifting&&isLifting())
				{
					herotile(&tile, &flip, &extend, ls_lifting, dir, zinit.heroAnimationStyle);
					if(script_hero_sprite <= 0)
					{
						auto frames = vbound(liftingspr[dir][spr_frames],1,255);
						auto speed = tliftclk/frames;
						if (speed < 1) speed = 1;
						auto curframe = (tliftclk - liftclk) / speed;
						if (!tliftclk) curframe = frames - 1;
						if(unsigned(curframe) < frames)
							tile += curframe * (extend == 2 ? 2 : 1);
					}
				}
				else
				{
					if(IsSideSwim())
						herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					else if(!noliftspr&&isLifting())
						herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
					else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					
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
					if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				
				break;
				
			case las_zelda3slow:                                           //8-frame Zelda 3 (slow)
			case las_zelda3:                                               //8-frame Zelda 3
				if(action == drowning)
				{
					if(inwater)
					{
						herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_drown, dir, zinit.heroAnimationStyle);
						if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
					else
					{
						goto herodraw_end;
					}
				}
				else if(action == lavadrowning)
				{
						herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_lavadrown, dir, zinit.heroAnimationStyle);
						if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				
				}
				else if(action == sidedrowning)
				{
						herotile(&tile, &flip, &extend, ls_sidedrown, down, zinit.heroAnimationStyle);
						if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if (action == sideswimming || action == sideswimhit)
				{
					herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					
					if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if(action == swimming || action==swimhit || hopclk==0xFF)
				{
					herotile(&tile, &flip, &extend, is_moving()?ls_swim:ls_float, dir, zinit.heroAnimationStyle);
					if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					
					if(isDiving())
					{
						herotile(&tile, &flip, &extend, ls_dive, dir, zinit.heroAnimationStyle);
						if (script_hero_sprite <= 0 ) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
				}
				else if(charging > 0 && attack != wHammer)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimcharge:ls_charge, dir, zinit.heroAnimationStyle);
					if (script_hero_sprite <= 0 ) tile+=(extend==2?2:1);
					//int32_t l=hero_count/hero_animation_speed;
					int32_t l=(hero_count/hero_animation_speed)&15;
					//int32_t l=((p[lt_clock]/hero_animation_speed)&15);
					l-=((l>3)?1:0)+((l>12)?1:0);
					if (script_hero_sprite <= 0 ) tile+=(l/2)*(extend==2?2:1);
				}
				else if((z>0 || fakez>0 || isSideViewHero()) && jumping2>0 && jumping2<24 && game->get_life()>0)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if (script_hero_sprite <= 0 ) tile+=((int32_t)jumping2/8)*(extend==2?2:1);
				}
				else if(fallclk>0)
				{
					herotile(&tile, &flip, &extend, ls_falling, dir, zinit.heroAnimationStyle);
					if (script_hero_sprite <= 0 ) tile += ((PITFALL_FALL_FRAMES-fallclk)/10)*(extend==2?2:1);
				}
				else if(!noliftspr&&action==lifting&&isLifting())
				{
					herotile(&tile, &flip, &extend, ls_lifting, dir, zinit.heroAnimationStyle);
					if(script_hero_sprite <= 0)
					{
						auto frames = vbound(liftingspr[dir][spr_frames],1,255);
						auto speed = tliftclk/frames;
						if (speed < 1) speed = 1;
						auto curframe = (tliftclk-liftclk)/speed;
						if (!tliftclk) curframe = frames - 1;
						if(unsigned(curframe) < frames)
							tile += curframe * (extend == 2 ? 2 : 1);
					}
				}
				else
				{
					if(IsSideSwim())
						herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					else if(!noliftspr&&isLifting())
						herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
					else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					
					if(action == walking || action == climbcoverbottom || action == climbcovertop)
					{
						if (script_hero_sprite <= 0 ) tile += (extend == 2 ? 2 : 1);
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
						int32_t l = hero_count / hero_animation_speed;
						l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
						if (script_hero_sprite <= 0 ) tile += (l / 2) * (extend == 2 ? 2 : 1);
					}
				}
				
				break;
				
			default:
				break;
			}
		}
		
		yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
		
		if(action==won)
		{
			yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) - 2;
		}
		
		if(action==landhold1 || action==landhold2)
		{
			useltm=(get_qr(qr_EXPANDEDLTM) != 0);
			yofs = oyofs-((!BSZ && isdungeon() && currscr<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
			herotile(&tile, &flip, &extend, (action==landhold1)?ls_landhold1:ls_landhold2, dir, zinit.heroAnimationStyle);
		}
		else if(action==waterhold1 || action==waterhold2)
		{
			useltm=(get_qr(qr_EXPANDEDLTM) != 0);
			herotile(&tile, &flip, &extend, (action==waterhold1)?ls_waterhold1:ls_waterhold2, dir, zinit.heroAnimationStyle);
		}
		else if(action==sidewaterhold1 || action==sidewaterhold2)
		{
			useltm=(get_qr(qr_EXPANDEDLTM) != 0);
			herotile(&tile, &flip, &extend, (action==sidewaterhold1)?ls_sidewaterhold1:ls_sidewaterhold2, dir, zinit.heroAnimationStyle);
		}
		
		if(action!=casting && action!=sideswimcasting)
		{
			if(useltm)
			{
				if (script_hero_sprite <= 0 ) tile+=getTileModifier();
			}
		}
		
		// Stone of Agony
		if(agony)
		{
			yofs-=!(frame%zc_max(60-itemsbuf[agonyid].misc1,3))?1:0;
		}
		
		if(!(get_qr(qr_HEROFLICKER)&&((superman||hclk)&&(frame&1))))
		{
			masked_draw(dest);
		}
		
		//draw held items after Hero so they don't go behind his head
		if(action==landhold1 || action==landhold2)
		{
			if(holditem > -1)
			{
				int hx = x - ((action==landhold1)?4:0);
				int hy = y+yofs-16-(get_qr(qr_NOITEMOFFSET))-fakez-z;
				if(get_qr(qr_HOLDITEMANIMATION))
				{
					putitem2(dest,hx,hy,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
				}
				else
				{
					putitem(dest,hx,hy,holditem);
				}
			}
		}
		else if(action==waterhold1 || action==waterhold2)
		{
			if(holditem > -1)
			{
				int hx = x - ((action==waterhold1)?4:0);
				int hy = y+yofs-12-(get_qr(qr_NOITEMOFFSET))-fakez-z;
				if(get_qr(qr_HOLDITEMANIMATION))
				{
					putitem2(dest,hx,hy,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
				}
				else
				{
					putitem(dest,hx,hy,holditem);
				}
			}
		}
		else if(action==sidewaterhold1 || action==sidewaterhold2) //!DIMITODO: Check to see if this looks right or if it needs waterhold's offset.
		{
			if(holditem > -1)
			{
				int hx = x - ((action==sidewaterhold1)?4:0);
				int hy = y+yofs-16-(get_qr(qr_NOITEMOFFSET))-fakez-z;
				if(get_qr(qr_HOLDITEMANIMATION))
				{
					putitem2(dest,hx,hy,holditem,lens_hint_item[holditem][0], lens_hint_item[holditem][1], 0);
				}
				else
				{
					putitem(dest,hx,hy,holditem);
				}
			}
		}
		if(fairyclk==0||(get_qr(qr_NOHEARTRING)))
		{
			goto herodraw_end;
		}
		
		double a2 = fairyclk*4.5 + 90;
		int32_t hearts=0;
		int32_t htile = 2;
		
		do
		{
			int32_t nx=125;
			
			if(get_qr(qr_HEARTRINGFIX))
			{
				nx=x;
			}
			
			int32_t ny=88;
			
			if(get_qr(qr_HEARTRINGFIX))
			{
				ny=y;
			}
			
			int32_t tx = zc::math::CosD(a2)*53  +nx;
			int32_t ty = -zc::math::SinD(a2)*53 +ny+playing_field_offset;
			overtile8(dest,htile,tx,ty,1,0);
			a2-=45;
			++hearts;
		}
		while(a2>90 && hearts<8);
	}
herodraw_end:
	xofs=oxofs;
	yofs=oyofs;
	do_primitives(dest, SPLAYER_PLAYER_DRAW, tmpscr, 0, playing_field_offset);
}

void HeroClass::masked_draw(BITMAP* dest)
{
	zfix lz, lfz;
	if(lift_wpn)
	{
		lz = lift_wpn->z;
		lfz = lift_wpn->fakez;
	}
	
	if(isdungeon() && currscr<128 && (x<16 || x>(world_w-32) || y<18 || y>(world_h-30)) && !get_qr(qr_FREEFORM))
	{
		// clip under doorways
		BITMAP *sub=create_sub_bitmap(dest,16,playing_field_offset+16,224,144);
		
		if(sub!=NULL)
		{
			yofs -= (playing_field_offset+16);
			xofs -= 16;
			sprite::draw(sub);
			if(lift_wpn)
			{
				handle_lift(false);
				bool shad = lift_wpn->has_shadow;
				lift_wpn->has_shadow = false;
				lift_wpn->z += z;
				lift_wpn->fakez += fakez;
				lift_wpn->draw(sub);
				lift_wpn->has_shadow = shad;
			}
			prompt_draw(sub);
			xofs+=16;
			yofs += (playing_field_offset+16);
			destroy_bitmap(sub);
		}
	}
	else
	{
		sprite::draw(dest);
		if(lift_wpn)
		{
			handle_lift(false);
			bool shad = lift_wpn->has_shadow;
			lift_wpn->has_shadow = false;
			lift_wpn->z += z;
			lift_wpn->fakez += fakez;
			lift_wpn->draw(dest);
			lift_wpn->has_shadow = shad;
		}
		prompt_draw(dest);
	}
	
	if(lift_wpn)
	{
		lift_wpn->z = lz;
		lift_wpn->fakez = lfz;
	}
	return;
}
void HeroClass::prompt_draw(BITMAP* dest)
{
	if(!prompt_combo) return;
	int32_t sx = real_x(x+xofs+prompt_x);
	int32_t sy = real_y(y + yofs + prompt_y) - real_z(z + zofs);
	sy -= fake_z(fakez);
	overcombo(dest, sx, sy, prompt_combo, prompt_cset);
	return;
}

void collectitem_script(int32_t id)
{
	if(itemsbuf[id].collect_script)
	{
		//clear item script stack. 
		FFCore.ref(ScriptType::Item, -id).Clear();

		if ( id > 0 && !(FFCore.doscript(ScriptType::Item, -id) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) //No collect script on item 0. 
		{
			int i = -id;
			FFCore.reset_script_engine_data(ScriptType::Item, i);
			ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id].collect_script, i);
			FFCore.deallocateAllScriptOwned(ScriptType::Item, i);
		}
		else if (id == 0 && !(FFCore.doscript(ScriptType::Item, -id) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING))) //item 0
		{
			int i = COLLECT_SCRIPT_ITEM_ZERO;
			FFCore.reset_script_engine_data(ScriptType::Item, i);
			ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id].collect_script, i);
			FFCore.deallocateAllScriptOwned(ScriptType::Item, i);
		}
		//runningItemScripts[id] = 0;
	}
}
void passiveitem_script(int32_t id, bool doRun = false)
{
	//Passive item scripts on colelction
	if(itemsbuf[id].script && ( (itemsbuf[id].flags&ITEM_PASSIVESCRIPT) && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ))
	{
		FFCore.reset_script_engine_data(ScriptType::Item, id);
		
		if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
			&& current_item(itemsbuf[id].family) > itemsbuf[id].fam_type)
		{
			FFCore.doscript(ScriptType::Item, id) = false;
			return;
		}
		if(doRun)
			ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id].script, id);
	}
}

// separate case for sword/wand/hammer/slashed weapons only
// the main weapon checking is in the global function check_collisions()
bool HeroClass::checkstab()
{
    if(action!=attacking && action!=sideswimattacking || (attack!=wSword && attack!=wWand && attack!=wHammer && attack!=wCByrna && attack!=wFire && attack != wBugNet)
            || (attackclk<=4))
        return false;
        
    weapon *w=NULL;
    
    int32_t wx=0,wy=0,wz=0,wxsz=0,wysz=0;
    bool found = false;
    int32_t melee_weapon_index = 0;
	int32_t parentitem=-1;
    weapon* meleeweap = nullptr;
    for(int32_t i=0; i<Lwpns.Count(); i++)
    {
        w = (weapon*)Lwpns.spr(i);
        
        if(w->id == (attack==wCByrna || attack==wFire ? wWand : attack))  // Kludge: Byrna and Candle sticks are wWand-type.
        {
            found = true;
            melee_weapon_index = i+1;
			meleeweap = w;
            // Position the sword as Hero slashes with it.
            if(w->id!=wHammer&&w->id!=wBugNet)
                positionSword(w,w->parentitem);
                
            wx=w->x;
            wy=w->y;
            wz=w->z;
            wxsz = w->hit_width;
            wysz = w->hit_height;
			parentitem = w->parentitem;
            break;
        }
    }
	
    if(attack==wSword && attackclk>=14 && charging==0)
        return false;
        
    if(!found)
        return false;
	
	if(attack == wFire)
		return false;
	
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
			
			if(attackclk==12 && z==0 && fakez==0 && sideviewhammerpound())
			{
				//decorations.add(new dHammerSmack((zfix)wx, (zfix)wy, dHAMMERSMACK, 0));
						/* The hammer smack sprites weren't being positioned properly if Hero changed directions on the same frame that they are created.
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
				*/
			}
			
			return false;
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
	int32_t itype = (attack==wWand ? itype_wand : attack==wSword ? itype_sword : attack==wCByrna ? itype_cbyrna : attack==wBugNet ? itype_bugnet : itype_hammer);
	int32_t itemid = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
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
			//wy+=((spins>0 || get_qr(qr_SLASHFLIPFIX)) ? -4 : 4);
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
	wy-=(w->fakez).getInt();
	
	for(int32_t i=0; i<guys.Count(); i++)
	{
		if(attack==wBugNet) break;
		// So that Hero can actually hit peahats while jumping, his weapons' hzsz becomes 16 in midair.
		if((guys.spr(i)->hit(wx,wy,wz,wxsz,wysz,wz>0?16:8) && ((attack!=wWand && attack!=wHammer && attack!=wCByrna) || !(itemsbuf[itemid].flags & ITEM_FLAG3)))
				|| ((attack==wWand || attack==wCByrna) && guys.spr(i)->hit(wx,wy-8,z,16,24,z>8) && !(itemsbuf[itemid].flags & ITEM_FLAG3))
				|| (attack==wHammer && guys.spr(i)->hit(wx,wy-8,z,16,24,z>0?16:8) && !(itemsbuf[itemid].flags & ITEM_FLAG3)))
		{
			// Checking the whimsical ring for every collision check causes
			// an odd bug. It's much more likely to activate on a 0-damage
			// weapon, since a 0-damage hit won't make the enemy invulnerable
			// to damaging hits in the following frames.
			
			int32_t whimsyid = current_item_id(itype_whimsicalring);
			
			int32_t dmg = weaponattackpower(itemid);
			if(whimsyid>-1)
			{
				if(!(zc_oldrand()%zc_max(itemsbuf[whimsyid].misc1,1)))
					dmg += current_item_power(itype_whimsicalring);
				else whimsyid = -1;
			}
			int32_t atkringid = current_item_id(itype_atkring);
			if(atkringid>-1)
			{
				dmg *= itemsbuf[atkringid].misc2; //Multiplier
				dmg += itemsbuf[atkringid].misc1; //Additive
			}
			
			int32_t h = hit_enemy(i,attack,dmg*game->get_hero_dmgmult(),wx,wy,dir,directWpn,w);
			enemy *e = (enemy*)guys.spr(i);
			if (h == -1) 
			{ 
				e->hitby[HIT_BY_LWEAPON] = melee_weapon_index; 
				e->hitby[HIT_BY_LWEAPON_UID] = w->script_UID;
				e->hitby[HIT_BY_LWEAPON_TYPE] = w->id;
				if (w->parentitem > -1) e->hitby[HIT_BY_LWEAPON_PARENT_FAMILY] = itemsbuf[w->parentitem].family; 
				else e->hitby[HIT_BY_LWEAPON_PARENT_FAMILY] = -1;
				e->hitby[HIT_BY_LWEAPON_PARENT_ID] = w->parentitem;
				e->hitby[HIT_BY_LWEAPON_ENGINE_UID] = w->getUID();
			} //temp_hit = true; }
			//melee weapons and non-melee weapons both writing to this index may be a problem. It needs to be cleared by something earlier than this check.
			
			if(h<0 && whimsyid>-1)
			{
				sfx(itemsbuf[whimsyid].usesound);
			}
			
			if(h && charging>0)
			{
				attackclk = SWORDTAPFRAME;
				spins=0;
			}
			
			if(h && hclk==0 && inlikelike != 1 && !get_qr(qr_DYING_ENEMIES_IGNORE_STUN))
			{
				if(GuyHit(i,x+7,y+7-fakez,z,2,2,hzsz)!=-1)
				{
					hithero(i);
				}
			}
			
			if(h==2)
				break;
		}
	}
		
	if(attack == wBugNet
		|| (parentitem==-1&&!get_qr(qr_NOITEMMELEE))
		|| (parentitem>-1&&!(itemsbuf[parentitem].flags & ITEM_FLAG7)))
	{
		int32_t bugnetid = attack != wBugNet ? -1 : (parentitem > -1 ? parentitem : current_item_id(itype_bugnet));
		for(int32_t j=0; j<items.Count(); j++)
		{
			item* ptr = (item*)items.spr(j);
			bool dofairy = (attack==wBugNet && itemsbuf[ptr->id].family == itype_fairy)
				&& (bugnetid > -1 && !(itemsbuf[bugnetid].flags & ITEM_FLAG1));
			
			if((itemsbuf[ptr->id].family == itype_bottlefill || dofairy) && !game->canFillBottle())
				continue; //No picking these up unless you have a bottle to fill!
			if((ptr->pickup & ipCANGRAB) || (ptr->pickup & ipTIMER) || dofairy)
			{
				if(((ptr->pickup & ipCANGRAB) || ptr->clk2 >= 32 || dofairy) && !ptr->fallclk && !ptr->drownclk)
				{
					if(ptr->hit(wx,wy,z,wxsz,wysz,1) || (attack==wWand && ptr->hit(x,y-8-fakez,z,wxsz,wysz,1))
							|| (attack==wHammer && ptr->hit(x,y-8-fakez,z,wxsz,wysz,1)))
					{
						int screen_index = get_screen_index_for_world_xy(wx, wy);
						mapscr* screen = get_screen_for_world_xy(wx, wy);
						int32_t pickup = ptr->pickup;
						int32_t id2 = ptr->id;
						int32_t pstr = ptr->pstring;
						int32_t pstr_flags = ptr->pickup_string_flags;
						if(!dofairy)
						{
							std::vector<int32_t> &ev = FFCore.eventData;
							ev.clear();
							ev.push_back(id2*10000);
							ev.push_back(pickup*10000);
							ev.push_back(pstr*10000);
							ev.push_back(pstr_flags*10000);
							ev.push_back(0);
							ev.push_back(ptr->getUID());
							ev.push_back(GENEVT_ICTYPE_MELEE*10000);
							ev.push_back(w->getUID());
							
							throwGenScriptEvent(GENSCR_EVENT_COLLECT_ITEM);
							bool nullify = ev[4] != 0;
							if(nullify) continue;
							id2 = ev[0]/10000;
							pickup = (pickup&(ipCHECK|ipDUMMY)) | (ev[1]/10000);
							pstr = ev[2] / 10000;
							pstr_flags = ev[3] / 10000;
						}
						
						if(pickup&ipONETIME) // set mITEM for one-time-only items
							setmapflag(screen, screen_index, mITEM);
						else if(pickup&ipONETIME2) // set mSPECIALITEM flag for other one-time-only items
							setmapflag(screen, screen_index, (currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
						
						if(ptr->pickupexstate > -1 && ptr->pickupexstate < 32)
							setxmapflag(screen_index, 1<<ptr->pickupexstate);
						if(pickup&ipSECRETS)								// Trigger secrets if this item has the secret pickup
						{
							if (screen->flags9&fITEMSECRETPERM) setmapflag(screen, screen_index, mSECRET);
							trigger_secrets_for_screen(TriggerSource::ItemsSecret, screen_index, false);
						}
						//!DIMI
						
						if(dofairy)
						{
							game->fillBottle(itemsbuf[ptr->id].misc4);
						}
						else
						{
							collectitem_script(id2);
							
							getitem(id2, false, true);
						}
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
								donewmsg(pstr);
								break;
							}
						}
						
						--j;
					}
				}
			}
		}
	}
	
	if(attack==wCByrna || attack==wBugNet)
		return false;
	
	if(meleeweap->no_triggers())
		return false;
	
	if(attack==wSword)
	{
		if(attackclk == 6)
		{
			for(int32_t q=0; q<176; q++)
			{
				set_bit(screengrid,q,0);
				set_bit(screengrid_layer[0],q,0);
				set_bit(screengrid_layer[1],q,0);
			}

			for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
				ffc_handle.ffc->recently_hit = false;
			});
		}
		
		if(dir==up && ((x.getInt()&15)==0))
		{
			check_slash_block(wx,wy);
			check_slash_block(wx,wy+8);
			
			//layers
			check_slash_block_layer(wx,wy,1);
			check_slash_block_layer(wx,wy+8,1);
			check_slash_block_layer(wx,wy,1);
			check_slash_block_layer(wx,wy+8,1);
			//2
			check_slash_block_layer(wx,wy,2);
			check_slash_block_layer(wx,wy+8,2);
			check_slash_block_layer(wx,wy,2);
			check_slash_block_layer(wx,wy+8,2);
		}
		else if(dir==up && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
		{
			check_slash_block(wx,wy);
			check_slash_block(wx,wy+8);
			check_slash_block(wx+8,wy);
			check_slash_block(wx+8,wy+8);
			///layer 1
			check_slash_block_layer(wx,wy,1);
			check_slash_block_layer(wx,wy+8,1);
			check_slash_block_layer(wx+8,wy,1);
			check_slash_block_layer(wx+8,wy+8,1);
			///layer 2
			check_slash_block_layer(wx,wy,2);
			check_slash_block_layer(wx,wy+8,2);
			check_slash_block_layer(wx+8,wy,2);
			check_slash_block_layer(wx+8,wy+8,2);
		}
		if(dir==down && ((x.getInt()&15)==0))
		{
			check_slash_block(wx,wy+wysz-8);
			check_slash_block(wx,wy+wysz);
			
			//layer 1
			check_slash_block_layer(wx,wy+wysz-8,1);
			check_slash_block_layer(wx,wy+wysz,1);
			//layer 2
			check_slash_block_layer(wx,wy+wysz-8,2);
			check_slash_block_layer(wx,wy+wysz,2);
		}
		else if(dir==down && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
		{
			check_slash_block(wx,wy+wysz-8);
			check_slash_block(wx,wy+wysz);
			check_slash_block(wx+8,wy+wysz-8);
			check_slash_block(wx+8,wy+wysz);
			//layer 1
			check_slash_block_layer(wx,wy+wysz-8,1);
			check_slash_block_layer(wx,wy+wysz,1);
			check_slash_block_layer(wx+8,wy+wysz-8,1);
			check_slash_block_layer(wx+8,wy+wysz,1);
			//layer 2
			check_slash_block_layer(wx,wy+wysz-8,2);
			check_slash_block_layer(wx,wy+wysz,2);
			check_slash_block_layer(wx+8,wy+wysz-8,2);
			check_slash_block_layer(wx+8,wy+wysz,2);
		}
		
		if(dir==left)
		{
			check_slash_block(wx,wy+8);
			check_slash_block(wx+8,wy+8);
			//layer 1
			check_slash_block_layer(wx,wy+8,1);
			check_slash_block_layer(wx+8,wy+8,1);
			//layer 2
			check_slash_block_layer(wx,wy+8,2);
			check_slash_block_layer(wx+8,wy+8,2);
		}
		
		if(dir==right)
		{
			check_slash_block(wx+wxsz,wy+8);
			check_slash_block(wx+wxsz-8,wy+8);
			//layer 1
			check_slash_block_layer(wx+wxsz,wy+8,1);
			check_slash_block_layer(wx+wxsz-8,wy+8,1);
			//layer 2
			check_slash_block_layer(wx+wxsz,wy+8,2);
			check_slash_block_layer(wx+wxsz-8,wy+8,2);
		}
	}
	else if(attack==wWand)
	{
		if(attackclk == 5)
		{
			for(int32_t q=0; q<176; q++)
			{
				set_bit(screengrid,q,0);
				set_bit(screengrid_layer[0],q,0);
				set_bit(screengrid_layer[1],q,0);
			}
			
			for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
				ffc_handle.ffc->recently_hit = false;
			});
		}
		
		// cutable blocks
		if(dir==up && (x.getInt()&15)==0)
		{
			check_wand_block(wx,wy);
			check_wand_block(wx,wy+8);
		}
		else if(dir==up && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
		{
			check_wand_block(wx,wy);
			check_wand_block(wx,wy+8);
			check_wand_block(wx+8,wy);
			check_wand_block(wx+8,wy+8);
		}
		
		if(dir==down && (x.getInt()&15)==0)
		{
			check_wand_block(wx,wy+wysz-8);
			check_wand_block(wx,wy+wysz);
		}
		else if(dir==down && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
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
	else if((attack==wHammer) && ((attackclk==15) || ( spins>0 && attackclk >=15 )))
	{
		// poundable blocks
		for(int32_t q=0; q<176; q++)
		{
			set_bit(screengrid,q,0);
			set_bit(screengrid_layer[0],q,0);
			set_bit(screengrid_layer[1],q,0);
		}
		
		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			ffc_handle.ffc->recently_hit = false;
		});
			
		if(dir==up && (x.getInt()&15)==0)
		{
			check_pound_block(wx,wy);
			check_pound_block(wx,wy+8);
		}
		else if(dir==up && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
		{
			check_pound_block(wx,wy);
			check_pound_block(wx,wy+8);
			check_pound_block(wx+8,wy);
			check_pound_block(wx+8,wy+8);
		}
		
		if(dir==down && (x.getInt()&15)==0)
		{
			check_pound_block(wx,wy+wysz-8);
			check_pound_block(wx,wy+wysz);
		}
		else if(dir==down && ((x.getInt()&15)==8||diagonalMovement||NO_GRIDLOCK))
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
	else return false;
	
	return true;
}

void HeroClass::check_slash_block_layer(int32_t bx, int32_t by, int32_t layer)
{
    if(!(get_qr(qr_BUSHESONLAYERS1AND2))) 
    {
	    //zprint("bit off\n");
	    return;
    }
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    //first things first
    if(attack!=wSword)
        return;
        
    if(z>8||fakez>8 || attackclk==SWORDCHARGEFRAME  // is not charging>0, as tapping a wall reduces attackclk but retains charging
            || (attackclk>SWORDTAPFRAME && tapping))
        return;
        
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
   
    int32_t flag = MAPFLAGL(layer,bx,by);
    int32_t flag2 = MAPCOMBOFLAGL(layer,bx,by);
    int32_t cid = MAPCOMBOL(layer,bx,by);
    int32_t type = combobuf[cid].type;
	if(combobuf[cid].triggerflags[0] & combotriggerONLYGENTRIG)
		type = cNONE;

	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, layer);
	mapscr* s = rpos_handle.screen;
	int i = rpos_handle.pos();
        
    bool ignorescreen=false;
    
    if((get_bit(screengrid_layer[layer-1], i) != 0) || (!isCuttableType(type)))
		return;
    
    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? itemsbuf[directWpn].fam_type : current_item(itype_sword);
	
	if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid_layer[layer-1],i,1);
	if(isCuttableNextType(type))
	{
		s->data[i]++;
	}
	else
	{
		s->data[i] = tmpscr->undercombo;
		s->cset[i] = tmpscr->undercset;
		s->sflag[i] = 0;
	}
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)bx, (zfix)by,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr->secretsfx);
	}
	else if(isCuttableItemType(type))
	{
		int32_t it = -1;
		int32_t thedropset = -1;
		
		//select_dropitem( (combobuf[MAPCOMBO(bx,by)-1].usrflags&cflag2) ? (combobuf[MAPCOMBO(bx,by)-1].attributes[1])/10000L : 12, bx, by);
		if ( (combobuf[cid].usrflags&cflag2) )
		{
			if(combobuf[cid].usrflags&cflag11)
				it = combobuf[cid].attribytes[1];
			else
			{
				it = select_dropitem(combobuf[cid].attribytes[1]); 
				thedropset = combobuf[cid].attribytes[1]; 
			}
		}
		else
		{
			it = select_dropitem(12);
			thedropset = 12;
		}
		if(it!=-1)
		{
			item* itm = (new item((zfix)bx, (zfix)by,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
			itm->from_dropset = thedropset;
			items.add(itm);
		}
	}
	
	putcombo(scrollbuf,bx - viewport.x,by - viewport.y,rpos_handle.data(),rpos_handle.cset());
	
	if(get_qr(qr_MORESOUNDS))
	{
		if (!isBushType(type) && !isFlowersType(type) && !isGrassType(type))
		{
			if (combobuf[cid].usrflags&cflag3)
			{
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
			}
		}
		else
		{
			if (combobuf[cid].usrflags&cflag3)
			{
				sfx(combobuf[cid].attribytes[2],int32_t(bx));
			}
			else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
		}
	}
	
	spawn_decoration_xy(combobuf[cid], fx, fy);
}

void HeroClass::check_slash_block(int32_t bx, int32_t by)
{
	//first things first
	if(attack!=wSword)
		return;

	// keep things inside the screen boundaries
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h-1);
	int32_t fx=bx;
	int32_t fy=by;

	if(z>8||fakez>8 || attackclk==SWORDCHARGEFRAME  // is not charging>0, as tapping a wall reduces attackclk but retains charging
			|| (attackclk>SWORDTAPFRAME && tapping))
		return;
		
	// find out which combo row/column the coordinates are in
	bx = TRUNCATE_TILE(bx);
	by = TRUNCATE_TILE(by);
	
	int cid = MAPCOMBO(bx,by);
	int cid_ff = MAPFFCOMBO(fx,fy);
	auto current_ffc_handle = getFFCAt(fx,fy);
	newcombo const& cmb = combobuf[cid];
	newcombo const& cmb_ff = combobuf[cid_ff];
	int type = cmb.type;
	int type2 = cmb_ff.type;
	int flag = MAPFLAG(bx,by);
	int flag2 = cmb.flag;
	int flag3 = cmb_ff.flag;
	
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, 0);
	int32_t i = rpos_handle.pos();

	bool ignorescreen=false;
	bool ignoreffc=false;
	
	if(get_bit(screengrid, i) != 0)
	{
		ignorescreen = true;
	}
	else if(cmb.triggerflags[0] & combotriggerONLYGENTRIG)
		ignorescreen = true;
	
	
	if (!current_ffc_handle || current_ffc_handle->ffc->recently_hit)
	{
		ignoreffc = true;
	}
	else if(cmb_ff.triggerflags[0] & combotriggerONLYGENTRIG)
		ignoreffc = true;
	
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
	
	mapscr *s = currscr >= 128 ? &special_warp_return_screen : rpos_handle.screen;
	
	int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? itemsbuf[directWpn].fam_type : current_item(itype_sword);
	byte skipsecrets = 0;
	
	if ( isNextType(type) ) //->Next combos should not trigger secrets. Their child combo, may want to do that! -Z 17th December, 2019
	{
		if (get_qr(qr_OLD_SLASHNEXT_SECRETS))
		{
			skipsecrets = 0;
		}
		else skipsecrets = 1;
	}
	
	if(!ignorescreen && (!skipsecrets || !get_qr(qr_BUGGY_BUGGY_SLASH_TRIGGERS)))
	{
		if((flag >= 16)&&(flag <= 31) && !skipsecrets)
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
			sfx(s->secretsfx);
		}
		else if(((flag>=mfSWORD&&flag<=mfXSWORD)||(flag==mfSTRIKE)))
		{
			for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
			{
				trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
			}
			
			trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
			for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
			{
				trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
			}
			
			trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
	else if(!ignorescreen && skipsecrets)
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
	}
	
	if(((flag3>=mfSWORD&&flag3<=mfXSWORD)||(flag3==mfSTRIKE)) && !ignoreffc)
	{
		for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
		{
			trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
		}
		
		trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
	}
	else if(!ignoreffc)
	{
		if(isCuttableNextType(type2))
		{
			current_ffc_handle->ffc->incData(1);
		}
		else
		{
			current_ffc_handle->ffc->setData(s->undercombo);
			current_ffc_handle->ffc->cset = s->undercset;
		}
	}
	
	if(!ignorescreen)
	{
		if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid,i,1);
		
		if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
		{
			items.add(new item((zfix)bx, (zfix)by,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
			sfx(tmpscr->secretsfx);
		}
		else if(isCuttableItemType(type))
		{
			int32_t it = -1;
			int32_t thedropset = -1;
			if ( (cmb.usrflags&cflag2) ) //specific dropset or item
			{
				if ( cmb.usrflags&cflag11 ) 
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
				item* itm = (new item((zfix)bx, (zfix)by,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
		}

		putcombo(scrollbuf, bx - viewport.x, by - viewport.y, s->data[i], s->cset[i]);
		
		if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type) && !isFlowersType(type) && !isGrassType(type))
			{
				if (cmb.usrflags&cflag3)
				{
					sfx(cmb.attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (cmb.usrflags&cflag3)
				{
					sfx(cmb.attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		spawn_decoration_xy(cmb, fx, fy);
	}
	
	if(!ignoreffc)
	{
		// TODO z3 !
		if(!isTouchyType(type2) && !get_qr(qr_CONT_SWORD_TRIGGERS))
		{
			current_ffc_handle->ffc->recently_hit = true;
		}
		
		if(isCuttableItemType(type2))
		{
			int32_t it=-1;
			int32_t thedropset=-1;
			if ( (cmb_ff.usrflags&cflag2) )
			{
				if(cmb_ff.usrflags&cflag11)
					it = cmb_ff.attribytes[1];
				else
				{
					it = select_dropitem(cmb_ff.attribytes[1]); 
					thedropset = cmb_ff.attribytes[1]; 
				}
			}
			else
			{
				if(get_qr(qr_HARDCODED_FFC_BUSH_DROPS))
				{
					int32_t r=zc_oldrand()%100;
					
					if(r<15)
						it=iHeart; // 15%
					else if(r<35)
						it=iRupy; // 20%
				}
				else
				{
					it = select_dropitem(12);
					thedropset = 12;
				}
			}
			
			if(it!=-1 && itemsbuf[it].family != itype_misc) // Don't drop non-gameplay items
			{
				item* itm = (new item((zfix)fx, (zfix)fy,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
		}
		
		if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type2) && !isFlowersType(type2) && !isGrassType(type2))
			{
				if (cmb_ff.usrflags&cflag3)
				{
					sfx(cmb_ff.attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (cmb_ff.usrflags&cflag3)
				{
					sfx(cmb_ff.attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		spawn_decoration_xy(cmb_ff, fx, fy);
	}
}

void HeroClass::check_wpn_triggers(int32_t bx, int32_t by, weapon *w)
{
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h);
	switch(w->useweapon)
	{
		case wArrow:
			trigger_secrets_if_flag(bx,by,mfARROW,true);
			trigger_secrets_if_flag(bx,by,mfSARROW,true);
			trigger_secrets_if_flag(bx,by,mfGARROW,true);
			break;
		case wBeam:
			for(int32_t i = 0; i <4; i++) trigger_secrets_if_flag(bx,by,mfSWORDBEAM+i,true);
			break;
		case wHookshot:
			trigger_secrets_if_flag(bx,by,mfHOOKSHOT,true);
			break;
		case wBrang:
			for(int32_t i = 0; i <3; i++) trigger_secrets_if_flag(bx,by,mfBRANG+i,true);
			break;
		case wMagic:
			trigger_secrets_if_flag(bx,by,mfWANDMAGIC,true);
			break;
		case wRefMagic:
			trigger_secrets_if_flag(bx,by,mfWANDMAGIC,true);
			break;
		case wRefBeam:
			for(int32_t i = 0; i <4; i++) trigger_secrets_if_flag(bx,by,mfSWORDBEAM+i,true);
			break;
		//reflected magic needs to happen in mirrors:
		//
		//trigger_secrets_if_flag(bx,by,mfREFMAGIC,true)
		case wRefFireball:
			trigger_secrets_if_flag(bx,by,mfREFFIREBALL,true);
			break;
		case wBomb:
			trigger_secrets_if_flag(bx+w->txsz,by+tysz+(isSideViewGravity()?2:-3),mfBOMB,true);
			break;
		
		case wSBomb:
			trigger_secrets_if_flag(bx+w->txsz,by+tysz+(isSideViewGravity()?2:-3),mfSBOMB,true);
			break;
			
		case wFire:
			trigger_secrets_if_flag(bx,by,mfANYFIRE,true);
			trigger_secrets_if_flag(bx,by,mfSTRONGFIRE,true);
			trigger_secrets_if_flag(bx,by,mfMAGICFIRE,true);
			break;
		
		case wScript1:
			break;
		case wScript2:
			break;
		case wScript3:
			break;
		case wScript4:
			break;
		case wScript5:
			break;
		case wScript6:
			break;
		case wScript7:
			break;
		case wScript8:
			break;
		case wScript9:
			break;
		case wScript10:
			break;
		case wIce:
			break;
		case wCByrna:
			break;
		case wWhistle:
			break;
		case wSSparkle:
		case wFSparkle:
			break;
		case wWind:
			break;
		case wBait:
			break;
		case wFlame:
		case wThrown:
		case wBombos:
		case wEther:
		case wQuake:
		case wSwordLA:
		case wSword180:
		case wStomp:
			break;
		case wSword:
		case wWand:
		//case wCandle:
		case wHSHandle:
		case wLitBomb:
		case wLitSBomb:
			break;
		default: break;
		
	}
}

void HeroClass::check_slash_block_layer2(int32_t bx, int32_t by, weapon *w, int32_t layer)
{
	
    if(!(get_qr(qr_BUSHESONLAYERS1AND2))) 
    {
	    //zprint("bit off\n");
	    return;
    }
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    //first things first
    if(w->useweapon != wSword)
        return;
        
    //find out which combo row/column the coordinates are in
	bx = TRUNCATE_TILE(bx);
	by = TRUNCATE_TILE(by);
    
   
    int32_t flag = MAPFLAGL(layer,bx,by);
    int32_t flag2 = MAPCOMBOFLAGL(layer,bx,by);
    int32_t cid = MAPCOMBOL(layer,bx,by);
    int32_t type = combobuf[cid].type;
	if(combobuf[cid].triggerflags[0] & combotriggerONLYGENTRIG)
		type = cNONE;
    //zprint("cid is: %d\n", cid);
     //zprint("type is: %d\n", type);
	
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, layer);
    int32_t i = rpos_handle.pos();
    
	// bool checked = w->rposes_checked.contains({rpos_handle.layer, rpos_handle.rpos});
    if((get_bit(w->wscreengrid_layer[layer-1], i) != 0) || (!isCuttableType(type)))
    {
	return; 
        //ignorescreen = true;
	//zprint("ignoring\n");
    }

	mapscr* s = rpos_handle.screen;
    
    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? itemsbuf[directWpn].fam_type : current_item(itype_sword);
    
    {
	    if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(w->wscreengrid_layer[layer-1],i,1);
            if(isCuttableNextType(type) || isCuttableNextType(type))
            {
                s->data[i]++;
            }
            else
            {
                s->data[i] = tmpscr->undercombo;
                s->cset[i] = tmpscr->undercset;
                s->sflag[i] = 0;
            }
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(tmpscr->secretsfx);
        }
        else if(isCuttableItemType(type))
        {
            int32_t it = -1;
            int32_t thedropset = -1;
		
			if ( (combobuf[cid].usrflags&cflag2) )
			{
				if(combobuf[cid].usrflags&cflag11)
					it = combobuf[cid].attribytes[1];
				else
				{
					it = select_dropitem(combobuf[cid].attribytes[1]); 
					thedropset = combobuf[cid].attribytes[1];
				}
			}
			else
			{
				it = select_dropitem(12);
				thedropset = 12;
			}
			
            if(it!=-1)
            {
                item* itm = (new item((zfix)bx, (zfix)by,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
        }
        
        putcombo(scrollbuf,bx - viewport.x, by - viewport.y,rpos_handle.data(),rpos_handle.cset());
        
        if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type) && !isFlowersType(type) && !isGrassType(type))
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		int16_t decotype = (combobuf[cid].usrflags & cflag1) ? ((combobuf[cid].usrflags & cflag10) ? (combobuf[cid].attribytes[0]) : (-1)) : (0);
		if(decotype > 3) decotype = 0;
		if(!decotype) decotype = (isBushType(type) ? 1 : (isFlowersType(type) ? 2 : (isGrassType(type) ? 3 : ((combobuf[cid].usrflags & cflag1) ? -1 : -2))));
		switch(decotype)
		{
			case -2: break; //nothing
			case -1:
				decorations.add(new comboSprite((zfix)fx, (zfix)fy, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
				break;
			case 1: decorations.add(new dBushLeaves((zfix)fx, (zfix)fy, dBUSHLEAVES, 0, 0)); break;
			case 2: decorations.add(new dFlowerClippings((zfix)fx, (zfix)fy, dFLOWERCLIPPINGS, 0, 0)); break;
			case 3: decorations.add(new dGrassClippings((zfix)fx, (zfix)fy, dGRASSCLIPPINGS, 0, 0)); break;
		}
            
    }
    
}

void HeroClass::check_slash_block2(int32_t bx, int32_t by, weapon *w)
{
	/*
	int32_t par_item = w->parentitem;
	al_trace("check_slash_block(weapon *w): par_item is: %d\n", par_item);
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].useweapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
	al_trace("check_slash_block(weapon *w): usewpn is: %d\n", usewpn);
	*/
    
	
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    int32_t cid = MAPCOMBO(bx,by);
        
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t type = COMBOTYPE(bx,by);
    int32_t type2 = FFCOMBOTYPE(fx,fy);
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3 = MAPFFCOMBOFLAG(fx,fy);
	if(combobuf[cid].triggerflags[0] & combotriggerONLYGENTRIG)
		type = cNONE;
    byte dontignore = 0;
    byte dontignoreffc = 0;
    
	    if (isCuttableType(type) && MatchComboTrigger(w, combobuf.data(), cid))
	    {
		al_trace("This weapon (%d) can slash the combo: combobuf[%d].\n", w->id, cid);
		dontignore = 1;
	    }
    
	    /*to-do, ffcs
	    if (isCuttableType(type2) && MatchComboTrigger(w, combobuf, cid))
	    {
		al_trace("This weapon (%d) can slash the combo: combobuf[%d].\n", w->id, cid);
		dontignoreffc = 1;
	    }*/
	if(w->useweapon != wSword && !dontignore) return;

    auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, 0);
    int32_t i = rpos_handle.pos();

	if (get_bit(w->wscreengrid,i)) return;
        
    bool ignorescreen=false;
    bool ignoreffc=false;
    
    if(get_bit(w->wscreengrid, i) != 0)
    {
        ignorescreen = true; dontignore = 0;
    }
    
    auto current_ffc_handle = getFFCAt(fx,fy);
    
    if (!current_ffc_handle || current_ffc_handle->ffc->recently_hit)
    {
        ignoreffc = true;
    }
    else if(combobuf[current_ffc_handle->ffc->getData()].triggerflags[0] & combotriggerONLYGENTRIG)
		type2 = cNONE;
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
    
    mapscr *s = currscr >= 128 ? &special_warp_return_screen : rpos_handle.screen;
    
    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? itemsbuf[directWpn].fam_type : current_item(itype_sword);
    byte skipsecrets = 0;
    if ( isNextType(type) ) //->Next combos should not trigger secrets. Their child combo, may want to do that! -Z 17th December, 2019
    {
		if (get_qr(qr_OLD_SLASHNEXT_SECRETS))
		{
			skipsecrets = 0;
		}
		else skipsecrets = 1; 
    }
    if((!skipsecrets || !get_qr(qr_BUGGY_BUGGY_SLASH_TRIGGERS)) && (!ignorescreen || dontignore))
    {
        if((flag >= 16)&&(flag <= 31)&&!skipsecrets)
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
            for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
            }
            
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
            for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
            }
            
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
    else if(skipsecrets && (!ignorescreen || dontignore))
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
    }
    
    if(((flag3>=mfSWORD&&flag3<=mfXSWORD)||(flag3==mfSTRIKE)) && !ignoreffc)
    {
        for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
        {
            trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
        }
        
        trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
    }
    else if(!ignoreffc)
    {
        if(isCuttableNextType(type2))
        {
            current_ffc_handle->ffc->incData(1);
        }
        else
        {
            current_ffc_handle->ffc->setData(s->undercombo);
            current_ffc_handle->ffc->cset = s->undercset;
        }
    }
    
    if(!ignorescreen || dontignore)
    {
        if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(w->wscreengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(tmpscr->secretsfx);
        }
		else if(isCuttableItemType(type))
        {
			int32_t it = -1;
			int32_t thedropset = -1;
			if ( (combobuf[cid].usrflags&cflag2) ) //specific dropset or item
			{
				if ( combobuf[cid].usrflags&cflag11 ) 
				{
					it = combobuf[cid].attribytes[1];
				}
				else
				{
					it = select_dropitem(combobuf[cid].attribytes[1]);
					thedropset = combobuf[cid].attribytes[1];
				}
			}
			else
			{
				it = select_dropitem(12);
				thedropset = 12;
			}
			
			if(it!=-1)
			{
				item* itm = (new item((zfix)bx, (zfix)by,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
        }
        
        
        putcombo(scrollbuf,bx-viewport.x,by-viewport.y,s->data[i],s->cset[i]);
        
        if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type) && !isFlowersType(type) && !isGrassType(type))
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		int16_t decotype = (combobuf[cid].usrflags & cflag1) ? ((combobuf[cid].usrflags & cflag10) ? (combobuf[cid].attribytes[0]) : (-1)) : (0);
		if(decotype > 3) decotype = 0;
		if(!decotype) decotype = (isBushType(type) ? 1 : (isFlowersType(type) ? 2 : (isGrassType(type) ? 3 : ((combobuf[cid].usrflags & cflag1) ? -1 : -2))));
		switch(decotype)
		{
			case -2: break; //nothing
			case -1:
				decorations.add(new comboSprite((zfix)fx, (zfix)fy, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
				break;
			case 1: decorations.add(new dBushLeaves((zfix)fx, (zfix)fy, dBUSHLEAVES, 0, 0)); break;
			case 2: decorations.add(new dFlowerClippings((zfix)fx, (zfix)fy, dFLOWERCLIPPINGS, 0, 0)); break;
			case 3: decorations.add(new dGrassClippings((zfix)fx, (zfix)fy, dGRASSCLIPPINGS, 0, 0)); break;
		}
    }
    
    if(!ignoreffc)
    {
        if(!isTouchyType(type2) && !get_qr(qr_CONT_SWORD_TRIGGERS))
		{
			current_ffc_handle->ffc->recently_hit = true;
		}
        
        if(isCuttableItemType(type2))
        {
            int32_t it=-1;
            int32_t thedropset=-1;
			if ( (combobuf[cid].usrflags&cflag2) )
			{
				if(combobuf[cid].usrflags&cflag11)
					it = combobuf[cid].attribytes[1];
				else
				{
					it = select_dropitem(combobuf[cid].attribytes[1]); 
					thedropset = combobuf[cid].attribytes[1];
				}
			}
			else
			{
				int32_t r=zc_oldrand()%100;
				
				if(r<15)
				{
					it=iHeart;                                // 15%
				}
				else if(r<35)
				{
					it=iRupy;                                 // 20%
				}
			}
            
            if(it!=-1 && itemsbuf[it].family != itype_misc) // Don't drop non-gameplay items
            {
                item* itm = (new item((zfix)fx, (zfix)fy,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
        }
        
        if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type2) && !isFlowersType(type2) && !isGrassType(type2))
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		int16_t decotype = (combobuf[cid].usrflags & cflag1) ? ((combobuf[cid].usrflags & cflag10) ? (combobuf[cid].attribytes[0]) : (-1)) : (0);
		if(decotype > 3) decotype = 0;
		if(!decotype) decotype = (isBushType(type2) ? 1 : (isFlowersType(type2) ? 2 : (isGrassType(type2) ? 3 : ((combobuf[cid].usrflags & cflag1) ? -1 : -2))));
		switch(decotype)
		{
			case -2: break; //nothing
			case -1:
				decorations.add(new comboSprite((zfix)fx, (zfix)fy, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
				break;
			case 1: decorations.add(new dBushLeaves((zfix)fx, (zfix)fy, dBUSHLEAVES, 0, 0)); break;
			case 2: decorations.add(new dFlowerClippings((zfix)fx, (zfix)fy, dFLOWERCLIPPINGS, 0, 0)); break;
			case 3: decorations.add(new dGrassClippings((zfix)fx, (zfix)fy, dGRASSCLIPPINGS, 0, 0)); break;
		}
    }
}

void HeroClass::check_wand_block2(int32_t bx, int32_t by, weapon *w)
{
	/*
	int32_t par_item = w->parentitem;
	al_trace("check_wand_block(weapon *w): par_item is: %d\n", par_item);
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].useweapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
	al_trace("check_wand_block(weapon *w): usewpn is: %d\n", usewpn);
	*/
	
	byte dontignore = 0;
	byte dontignoreffc = 0;
    
	
	
    

    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    int32_t cid = MAPCOMBO(bx,by);
   
    //Z_scripterrlog("check_wand_block2 MatchComboTrigger() returned: %d\n", );
    if(w->useweapon != wWand && !MatchComboTrigger (w, combobuf.data(), cid)) return;
    if ( MatchComboTrigger (w, combobuf.data(), cid) ) dontignore = 1;
    
    //first things first
    if(z>8||fakez>8) return;
    
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3=0;
    int32_t flag31 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag32 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag33 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag34 = MAPFFCOMBOFLAG(fx,fy);
    
    if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
        flag3=mfWAND;
        
    if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
        flag3=mfSTRIKE;
        
    int32_t i = (bx>>4) + by;
    
    if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
        return;

    //mapscr *s = currscr >= 128 ? &special_warp_return_screen : *tmpscr;
    
    //trigger_secrets_if_flag(bx,by,mfWAND,true);
    //trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
    if((trigger_secrets_if_flag(bx,by,mfWAND,true)==false)&&(trigger_secrets_if_flag(bx,by,mfSTRIKE,true)==false))
    {
        if(flag3==mfWAND||flag3==mfSTRIKE)
        {
            trigger_secrets_if_flag(fx,fy,mfWAND,true);
            trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
        }
    }
    
    if(dontignore) { trigger_secrets_if_flag(bx,by,mfWAND,true); }
}

void HeroClass::check_slash_block(weapon *w)
{
	//first things 
	
	int32_t par_item = w->parentitem;
	al_trace("check_slash_block(weapon *w): par_item is: %d\n", par_item);
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].useweapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
	al_trace("check_slash_block(weapon *w): usewpn is: %d\n", usewpn);
    if(usewpn != wSword) return;
	
	
    int32_t bx = 0, by = 0;
	bx = ((int32_t)w->x) + (((int32_t)w->hit_width)/2);
	by = ((int32_t)w->y) + (((int32_t)w->hit_height)/2);
	al_trace("check_slash_block(weapon *w): bx is: %d\n", bx);
	al_trace("check_slash_block(weapon *w): by is: %d\n", by);
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    
    int32_t cid = MAPCOMBO(bx,by);
        
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t type = COMBOTYPE(bx,by);
    int32_t type2 = FFCOMBOTYPE(fx,fy);
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3 = MAPFFCOMBOFLAG(fx,fy);
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, 0);
    int32_t i = rpos_handle.pos();
        
	if(combobuf[cid].triggerflags[0] & combotriggerONLYGENTRIG)
		type = cNONE;
    bool ignorescreen=false;
    bool ignoreffc=false;
    
    if(get_bit(screengrid, i) != 0)
    {
        ignorescreen = true;
    }
    
    auto current_ffc_handle = getFFCAt(fx,fy);
    
    if (!current_ffc_handle || current_ffc_handle->ffc->recently_hit)
    {
        ignoreffc = true;
    }
    else if(combobuf[current_ffc_handle->data()].triggerflags[0] & combotriggerONLYGENTRIG)
		type2 = cNONE;
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
    
    mapscr *s = currscr >= 128 ? &special_warp_return_screen : rpos_handle.screen;
    
    int32_t sworditem = (par_item >-1 ? itemsbuf[par_item].fam_type : current_item(itype_sword)); //Get the level of the item, else the highest sword level in inventory.
    
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
            for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
            }
            
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
            for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
            {
                trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
            }
            
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
        for(int32_t i2=0; i2<=zc_min(sworditem-1,3); i2++)
        {
            trigger_secrets_if_flag(bx,by,mfSWORD+i2,true);
        }
        
        trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
    }
    else if(!ignoreffc)
    {
        if(isCuttableNextType(type2))
        {
            current_ffc_handle->ffc->incData(1);
        }
        else
        {
            current_ffc_handle->ffc->setData(s->undercombo);
            current_ffc_handle->ffc->cset = s->undercset;
        }
    }
    
    if(!ignorescreen)
    {
        if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(tmpscr->secretsfx);
        }
        else if(isCuttableItemType(type))
        {
			int32_t it = -1;
			int32_t thedropset = -1;
			if ( (combobuf[cid].usrflags&cflag2) ) //specific dropset or item
			{
				if ( combobuf[cid].usrflags&cflag11 ) 
				{
					it = combobuf[cid].attribytes[1];
				}
				else
				{
					it = select_dropitem(combobuf[cid].attribytes[1]);
					thedropset = combobuf[cid].attribytes[1];
				}
			}
			else
			{
				it = select_dropitem(12);
				thedropset = 12;
			}
			
			if(it!=-1)
			{
				item* itm = (new item((zfix)bx, (zfix)by,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
				itm->from_dropset = thedropset;
				items.add(itm);
			}
        }
        
        putcombo(scrollbuf,bx-viewport.x,by-viewport.y,s->data[i],s->cset[i]);
        
        if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type) && !isFlowersType(type) && !isGrassType(type))
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		int16_t decotype = (combobuf[cid].usrflags & cflag1) ? ((combobuf[cid].usrflags & cflag10) ? (combobuf[cid].attribytes[0]) : (-1)) : (0);
		if(decotype > 3) decotype = 0;
		if(!decotype) decotype = (isBushType(type) ? 1 : (isFlowersType(type) ? 2 : (isGrassType(type) ? 3 : ((combobuf[cid].usrflags & cflag1) ? -1 : -2))));
		switch(decotype)
		{
			case -2: break; //nothing
			case -1:
				decorations.add(new comboSprite((zfix)fx, (zfix)fy, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
				break;
			case 1: decorations.add(new dBushLeaves((zfix)fx, (zfix)fy, dBUSHLEAVES, 0, 0)); break;
			case 2: decorations.add(new dFlowerClippings((zfix)fx, (zfix)fy, dFLOWERCLIPPINGS, 0, 0)); break;
			case 3: decorations.add(new dGrassClippings((zfix)fx, (zfix)fy, dGRASSCLIPPINGS, 0, 0)); break;
		}
    }
    
    if(!ignoreffc)
    {
		// TODO z3 !
        if(!isTouchyType(type2) && !get_qr(qr_CONT_SWORD_TRIGGERS))
		{
			current_ffc_handle->ffc->recently_hit = true;
		}
        
        if(isCuttableItemType(type2))
        {
            int32_t it=-1;
			int32_t thedropset = -1;
			if ( (combobuf[MAPCOMBO(bx,by)-1].usrflags&cflag2) )
			{
				if(combobuf[MAPCOMBO(bx,by)-1].usrflags&cflag11)
					it = combobuf[MAPCOMBO(bx,by)-1].attribytes[1];
				else
				{
					thedropset = combobuf[MAPCOMBO(bx,by)-1].attribytes[1];
					it = select_dropitem(thedropset);
				}
			}
			else
			{
				it = select_dropitem(12);
				thedropset = 12;
			}
			
            if(it!=-1 && itemsbuf[it].family != itype_misc) // Don't drop non-gameplay items
            {
				item* itm = (new item((zfix)fx, (zfix)fy,(zfix)0, it, ipBIGRANGE + ipTIMER, 0));
                itm->from_dropset = thedropset;
                items.add(itm);
            }
        }
        
		if(get_qr(qr_MORESOUNDS))
		{
			if (!isBushType(type2) && !isFlowersType(type2) && !isGrassType(type2))
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],int32_t(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],int32_t(bx));
			}
		}
		
		int16_t decotype = (combobuf[cid].usrflags & cflag1) ? ((combobuf[cid].usrflags & cflag10) ? (combobuf[cid].attribytes[0]) : (-1)) : (0);
		if(decotype > 3) decotype = 0;
		if(!decotype) decotype = (isBushType(type2) ? 1 : (isFlowersType(type2) ? 2 : (isGrassType(type2) ? 3 : ((combobuf[cid].usrflags & cflag1) ? -1 : -2))));
		switch(decotype)
		{
			case -2: break; //nothing
			case -1:
				decorations.add(new comboSprite((zfix)fx, (zfix)fy, dCOMBOSPRITE, 0, combobuf[cid].attribytes[0]));
				break;
			case 1: decorations.add(new dBushLeaves((zfix)fx, (zfix)fy, dBUSHLEAVES, 0, 0)); break;
			case 2: decorations.add(new dFlowerClippings((zfix)fx, (zfix)fy, dFLOWERCLIPPINGS, 0, 0)); break;
			case 3: decorations.add(new dGrassClippings((zfix)fx, (zfix)fy, dGRASSCLIPPINGS, 0, 0)); break;
		}
    }
}

void HeroClass::check_wand_block(int32_t bx, int32_t by)
{
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    int32_t cid = MAPCOMBO(bx,by);
    
    //first things first
    if(z>8||fakez>8) return;
    
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3=0;
    int32_t flag31 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag32 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag33 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag34 = MAPFFCOMBOFLAG(fx,fy);
    
    if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
        flag3=mfWAND;
        
    if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
        flag3=mfSTRIKE;
    
    if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
        return;
        
    //mapscr *s = currscr >= 128 ? &special_warp_return_screen : *tmpscr;
    
    //trigger_secrets_if_flag(bx,by,mfWAND,true);
    //trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
    if((trigger_secrets_if_flag(bx,by,mfWAND,true)==false)&&(trigger_secrets_if_flag(bx,by,mfSTRIKE,true)==false))
    {
        if(flag3==mfWAND||flag3==mfSTRIKE)
        {
            trigger_secrets_if_flag(fx,fy,mfWAND,true);
            trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
        }
    }
}

void HeroClass::check_pound_block(int bx, int by, weapon* w)
{
	if(w && w->no_triggers()) return;
	if(w && w->id == wHammer && getHammerState() < 3)
		return;
	if(get_qr(qr_POUNDLAYERS1AND2))
	{
		check_pound_block_layer(bx,by,1,w);
		check_pound_block_layer(bx,by,2,w);
	}
	auto* grid = w ? w->wscreengrid : screengrid;
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w);
    int32_t fy=vbound(by, 0, world_h);
    int32_t cid = MAPCOMBO(bx,by);
    
    //first things first
    if(z>8||fakez>8) return;
    
    //find out which combo row/column the coordinates are in
	bx = TRUNCATE_TILE(bx);
	by = TRUNCATE_TILE(by);
    
    int32_t type = COMBOTYPE(bx,by);
    int32_t type2 = FFCOMBOTYPE(fx,fy);
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3 = MAPFFCOMBOFLAG(fx,fy);

    rpos_t rpos = COMBOPOS_REGION(bx, by);
    int32_t pos = RPOS_TO_POS(rpos);
    if (!is_valid_rpos(rpos))
        return;
        
    bool ignorescreen=false;
    bool ignoreffc=false;
    bool pound=false;
    
    if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
        ignorescreen = true; // Affect only FFCs
    
    if(get_bit(grid, pos) != 0)
        ignorescreen = true;
        
    auto current_ffc_handle = getFFCAt(fx,fy);
    
    if (!current_ffc_handle || current_ffc_handle->ffc->recently_hit)
        ignoreffc = true;
        
    if(type2!=cPOUND && flag3!=mfSTRIKE && flag3!=mfHAMMER)
        ignoreffc = true;
        
    if(ignorescreen && ignoreffc)  // Nothing to do.
        return;
        
    mapscr *s = currscr >= 128 ? &special_warp_return_screen : get_screen_for_world_xy(bx, by);
    
    if(!ignorescreen)
    {
        if(flag==mfHAMMER||flag==mfSTRIKE)  // Takes precedence over Secret Tile and Armos->Secret
        {
            trigger_secrets_if_flag(bx,by,mfHAMMER,true);
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
        }
        else if(flag2==mfHAMMER||flag2==mfSTRIKE)
        {
            trigger_secrets_if_flag(bx,by,mfHAMMER,true);
            trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
        }
        else if((flag >= 16)&&(flag <= 31))
        {
            s->data[pos] = s->secretcombo[(s->sflag[pos])-16+4];
            s->cset[pos] = s->secretcset[(s->sflag[pos])-16+4];
            s->sflag[pos] = s->secretflag[(s->sflag[pos])-16+4];
        }
        else if(flag == mfARMOS_SECRET)
        {
            s->data[pos] = s->secretcombo[sSTAIRS];
            s->cset[pos] = s->secretcset[sSTAIRS];
            s->sflag[pos] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else if((flag2 >= 16)&&(flag2 <= 31))
        {
            s->data[pos] = s->secretcombo[(s->sflag[pos])-16+4];
            s->cset[pos] = s->secretcset[(s->sflag[pos])-16+4];
            s->sflag[pos] = s->secretflag[(s->sflag[pos])-16+4];
        }
        else if(flag2 == mfARMOS_SECRET)
        {
            s->data[pos] = s->secretcombo[sSTAIRS];
            s->cset[pos] = s->secretcset[sSTAIRS];
            s->sflag[pos] = s->secretflag[sSTAIRS];
            sfx(tmpscr->secretsfx);
        }
        else pound = true;
    }
    
    if(!ignoreffc)
    {
        if(flag3==mfHAMMER||flag3==mfSTRIKE)
        {
            trigger_secrets_if_flag(fx,fy,mfHAMMER,true);
            trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
        }
        else
        {
            current_ffc_handle->ffc->incData(1);
        }
    }
    
    if(!ignorescreen)
    {
        if(pound)
            s->data[pos]+=1;
            
        set_bit(grid,pos,1);
        
        set_bit(screengrid,pos,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by, (zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(tmpscr->secretsfx);
        }
        
        if(type==cPOUND && get_qr(qr_MORESOUNDS))
            sfx(QMisc.miscsfx[sfxHAMMERPOUND],int32_t(bx));
        
        putcombo(scrollbuf, bx - viewport.x, by - viewport.y, s->data[pos], s->cset[pos]);
    }
    
    if(!ignoreffc)
    {
		current_ffc_handle->ffc->recently_hit = true;

        if(type2==cPOUND && get_qr(qr_MORESOUNDS))
            sfx(QMisc.miscsfx[sfxHAMMERPOUND],int32_t(bx));
    }
    
    return;
}

void HeroClass::check_pound_block_layer(int bx, int by, int lyr, weapon* w)
{
	if(lyr < 1 || lyr > 2) return; //sanity
	//keep things inside the screen boundaries
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h);
	int32_t cid = MAPCOMBOL(lyr,bx,by);
	newcombo const& scr_cmb = combobuf[cid];
	auto* grid = w ? w->wscreengrid_layer[lyr-1] : screengrid_layer[lyr-1];
	
	//first things first
	if(z>8||fakez>8) return;
	
	//find out which combo row/column the coordinates are in
	bx = TRUNCATE_TILE(bx);
	by = TRUNCATE_TILE(by);
	
	int32_t type = scr_cmb.type;
	int32_t flag = MAPFLAGL(lyr,bx,by);
	int32_t flag2 = scr_cmb.flag;
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, lyr - 1);
	int32_t i = rpos_handle.pos();
	
	bool pound=false;
	
	if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
		return;
		
	if(get_bit(grid, i) != 0)
		return;
		
	mapscr *s = FFCore.tempScreens[lyr];
	
	if(flag==mfHAMMER||flag==mfSTRIKE)  // Takes precedence over Secret Tile and Armos->Secret
	{
		trigger_secrets_if_flag(bx,by,mfHAMMER,true);
		trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
	}
	else if(flag2==mfHAMMER||flag2==mfSTRIKE)
	{
		trigger_secrets_if_flag(bx,by,mfHAMMER,true);
		trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
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
	
	if(pound)
		s->data[i]+=1;
		
	set_bit(grid,i,1);
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)bx, (zfix)by, (zfix)0, tmpscr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(tmpscr->secretsfx);
	}
	
	if(type==cPOUND && get_qr(qr_MORESOUNDS))
		sfx(QMisc.miscsfx[sfxHAMMERPOUND],int32_t(bx));
	

	putcombo(scrollbuf,bx-viewport.x,by-viewport.y,s->data[i],s->cset[i]);
}

void HeroClass::check_wand_block(weapon *w)
{
	
    int32_t par_item = w->parentitem;
	al_trace("check_wand_block(weapon *w): par_item is: %d\n", par_item);
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].useweapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
	al_trace("check_wand_block(weapon *w): usewpn is: %d\n", usewpn);
    if(usewpn != wWand) return;
	
	
    int32_t bx = 0, by = 0;
	bx = ((int32_t)w->x) + (((int32_t)w->hit_width)/2);
	by = ((int32_t)w->y) + (((int32_t)w->hit_height)/2);
	
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h);
    int32_t fx=vbound(bx, 0, world_w-1);
    int32_t fy=vbound(by, 0, world_h);
    int32_t cid = MAPCOMBO(bx,by);
    //first things first
    if(z>8||fakez>8) return;
    
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3=0;
    int32_t flag31 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag32 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag33 = MAPFFCOMBOFLAG(fx,fy);
    int32_t flag34 = MAPFFCOMBOFLAG(fx,fy);
    
    if(flag31==mfWAND||flag32==mfWAND||flag33==mfWAND||flag34==mfWAND)
        flag3=mfWAND;
        
    if(flag31==mfSTRIKE||flag32==mfSTRIKE||flag33==mfSTRIKE||flag34==mfSTRIKE)
        flag3=mfSTRIKE;
    
    if(flag!=mfWAND&&flag2!=mfWAND&&flag3!=mfWAND&&flag!=mfSTRIKE&&flag2!=mfSTRIKE&&flag3!=mfSTRIKE)
        return;
        
    //mapscr *s = currscr >= 128 ? &special_warp_return_screen : *tmpscr;
    
    //trigger_secrets_if_flag(bx,by,mfWAND,true);
    //trigger_secrets_if_flag(bx,by,mfSTRIKE,true);
    if((trigger_secrets_if_flag(bx,by,mfWAND,true)==false)&&(trigger_secrets_if_flag(bx,by,mfSTRIKE,true)==false))
    {
        if(flag3==mfWAND||flag3==mfSTRIKE)
        {
            trigger_secrets_if_flag(fx,fy,mfWAND,true);
            trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
        }
    }
}

//defend results should match defence types. 
//RETURN VALUES: 
// -1 iGNORE WEAPON
// 0 No effects
// 1 Effects, weapon is not ignored or removed
int32_t HeroClass::defend(weapon *w)
{
	int32_t def = conv_edef_unblockable(defence[w->id], w->unblockable);
	switch(def)
	{
		case edNORMAL: return 1;
		case edHALFDAMAGE: // : IMPLEMENTED : Take half damage
		{
			w->power *= 0.5;
			return 1;
		}
		case edQUARTDAMAGE:
		{
			w->power *= 0.25;
			return 1;
		}
		case edSTUNONLY:
		{
			setStunClock(120);
			return 1;
		}
		case edSTUNORCHINK: // : IMPLEMENTED : If damage > 0, stun instead. Else, bounce off.
		{
			if (w->power > 0) 
			{
				setStunClock(120);
				return 1;
			}
			else 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
		}
		case edSTUNORIGNORE: // : IMPLEMENTED : If damage > 0, stun instead. Else, ignore.
		{
			if (w->power > 0) 
			{
				setStunClock(120);
				return 1;
			}
			else 
			{
				return -1;
			}
		}
		case edCHINKL1: // : IMPLEMENTED : Bounces off, plays SFX_CHINK
		{
			if (w->power < 1) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case edCHINKL2: // : IMPLEMENTED : Bounce off unless damage >= 2
		{
			if (w->power < 2) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case edCHINKL4: //: IMPLEMENTED : Bounce off unless damage >= 4
		{
			if (w->power < 4) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case edCHINKL6: // : IMPLEMENTED : Bounce off unless damage >= 6
		{
			if (w->power < 6) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case edCHINKL8: // : IMPLEMENTED : Bounce off unless damage >= 8
		{
			if (w->power < 8) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case edCHINK: // : IMPLEMENTED : Bounces off, plays SFX_CHINK
		{
			sfx(WAV_CHINK,pan(int32_t(x)));
			w->dead = 0;
			return -1;
		}
		case edIGNOREL1: // : IMPLEMENTED : Ignore unless damage > 1.
		{
			if (w->power < 1) 
			{
				return -1;
			}
			else return 1;
		}
		case edIGNORE: // : IMPLEMENTED : Do Nothing
		{
			return -1;
		}
		case ed1HKO: // : IMPLEMENTED : One-hit knock-out
		{
			game->set_life(0);
			return 1;
		}
		case edCHINKL10: //: IMPLEMENTED : If damage is less than 10
		{
			if (w->power < 10) 
			{
				sfx(WAV_CHINK,pan(int32_t(x)));
				w->dead = 0;
				return -1;
			}
			else 
			{
				return 1;
			}
		}
		case ed2x: // : IMPLEMENTED : Double damage.
		{
			w->power *= 2;
			return 1;
		}
		case ed3x: // : IMPLEMENTED : Triple Damage.
		{
			w->power *= 3;
			return 1;
		}
		case ed4x: // : IMPLEMENTED : 4x damage.
		{
			w->power *= 4;
			return 1;
		}
		case edHEAL: // : IMPLEMENTED : Gain the weapon damage in HP.
		{
			//sfx(WAV_HEAL,pan(int32_t(x)));
			game->set_life(zc_min(game->get_life()+w->power, game->get_maxlife()));
			w->dead = 0;
			return -1;
		}
	
		case edFREEZE: return 1; //Not IMPLEMENTED
	
		case edLEVELDAMAGE: //Damage * item level
		{
			w->power *= w->family_level;
			return 1;
		}
		case edLEVELREDUCTION: //Damage / item level
		{
			if ( w->family_level > 0 ) 
			{
				w->power /= w->family_level;
			}
			else w->power = 0;
			return 1;
		}
	
		//edLEVELCHINK2, //If item level is < 2: This needs a weapon variable that is set by 
		//edLEVELCHINK3, //If item level is < 3: the item that generates it (itemdata::level stored to
		//edLEVELCHINK4, //If item level is < 4: weapon::level, or something similar; then a check to
		//edLEVELCHINK5, //If item level is < 5: read weapon::level in hit detection. 
	
		//edSHOCK, //buzz blob
	
	
		case edBREAKSHIELD: //destroy the player's present shield
		{
			w->power = 0;
			w->dead = 0;
			int32_t itemid = getCurrentShield();
			//sfx(WAV_BREAKSHIELD,pan(int32_t(x)));
			if(itemsbuf[itemid].flags&ITEM_EDIBLE)
				game->set_item(itemid, false);
			//Remove Hero's shield
			return -1; 
		}
	
	
		
		default: return 0;
	}
}
ALLEGRO_COLOR HeroClass::hitboxColor(byte opacity) const
{
	return al_map_rgba(0,0,255,opacity);
}
int32_t HeroClass::compareDir(int32_t other)
{
	if(other != NORMAL_DIR(other))
		return 0; //*sigh* scripts expect dirs >=8 to NOT hit shields...
	int32_t ret = 0;
	auto d = (shield_forcedir < 0) ? dir : shield_forcedir;
	switch(d)
	{
		case up:
		{
			switch(X_DIR(other))
			{
				case left:
					ret |= CMPDIR_RIGHT;
					break;
				case right:
					ret |= CMPDIR_LEFT;
					break;
			}
			switch(Y_DIR(other))
			{
				case up:
					ret |= CMPDIR_BACK;
					break;
				case down:
					ret |= CMPDIR_FRONT;
					break;
			}
			break;
		}
		case down:
		{
			switch(X_DIR(other))
			{
				case left:
					ret |= CMPDIR_LEFT;
					break;
				case right:
					ret |= CMPDIR_RIGHT;
					break;
			}
			switch(Y_DIR(other))
			{
				case up:
					ret |= CMPDIR_FRONT;
					break;
				case down:
					ret |= CMPDIR_BACK;
					break;
			}
			break;
		}
		case left:
		{
			switch(X_DIR(other))
			{
				case left:
					ret |= CMPDIR_BACK;
					break;
				case right:
					ret |= CMPDIR_FRONT;
					break;
			}
			switch(Y_DIR(other))
			{
				case up:
					ret |= CMPDIR_LEFT;
					break;
				case down:
					ret |= CMPDIR_RIGHT;
					break;
			}
			break;
		}
		case right:
		{
			switch(X_DIR(other))
			{
				case left:
					ret |= CMPDIR_FRONT;
					break;
				case right:
					ret |= CMPDIR_BACK;
					break;
			}
			switch(Y_DIR(other))
			{
				case up:
					ret |= CMPDIR_RIGHT;
					break;
				case down:
					ret |= CMPDIR_LEFT;
					break;
			}
			break;
		}
	}
	return ret;
}

bool compareShield(int32_t cmpdir, itemdata const& shield)
{
	bool standard = !(shield.flags&ITEM_FLAG9) || usingActiveShield();
	if(standard) //Use standard sides, either a passive shield, or a held active shield
	{
		if((cmpdir&CMPDIR_FRONT) && (shield.flags&ITEM_FLAG1))
			return true;
		else if((cmpdir&CMPDIR_BACK) && (shield.flags&ITEM_FLAG2))
			return true;
		else if((cmpdir&CMPDIR_LEFT) && (shield.flags&ITEM_FLAG3))
			return true;
		else if((cmpdir&CMPDIR_RIGHT) && (shield.flags&ITEM_FLAG4))
			return true;
	}
	else //Active Shield that is NOT held down
	{
		if((cmpdir&CMPDIR_FRONT) && (shield.flags&ITEM_FLAG5))
			return true;
		else if((cmpdir&CMPDIR_BACK) && (shield.flags&ITEM_FLAG6))
			return true;
		else if((cmpdir&CMPDIR_LEFT) && (shield.flags&ITEM_FLAG7))
			return true;
		else if((cmpdir&CMPDIR_RIGHT) && (shield.flags&ITEM_FLAG8))
			return true;
	}
	return false;
}

int32_t HeroClass::EwpnHit()
{
	for(int32_t i=0; i<Ewpns.Count(); i++)
	{
		if(Ewpns.spr(i)->hit(x+7,y+7-fakez,z,2,2,1))
		{
			weapon *ew = (weapon*)(Ewpns.spr(i));
			
			if((ew->ignoreHero)==true || ew->fallclk|| ew->drownclk)
				break;
			
			int32_t stompid = current_item_id(itype_stompboots);
			
			if(current_item(itype_stompboots) && checkbunny(stompid) && checkmagiccost(stompid) && (stomping ||
			((z+fakez) > (ew->z+(ew->fakez))) ||
			((isSideViewHero() && (y+16)-(ew->y)<=14) && falling_oldy<y)))
			{
				itemdata const& stomp = itemsbuf[stompid];
				bool remove = false;
				switch(ew->id)
				{
					case ewFireball2:
					case ewFireball:
						if(ew->type & 1) //Boss fireball
						{
							if(stomp.misc2 & (shFIREBALL2))
								remove = true;
						}
						else
						{
							if(stomp.misc2 & (shFIREBALL))
								remove = true;
						}
						
						break;
						
					case ewMagic:
						if((stomp.misc2 & shMAGIC))
							remove = true;
						break;
						
					case ewSword:
						if((stomp.misc2 & shSWORD))
							remove = true;
							
						break;
						
					case ewFlame:
						if((stomp.misc2 & shFLAME))
							remove = true;
							
						break;
						
					case ewRock:
						if((stomp.misc2 & shROCK))
							remove = true;
							
						break;
						
					case ewArrow:
						if((stomp.misc2 & shARROW))
							remove = true;
							
						break;
						
					case ewBrang:
						if((stomp.misc2 & shBRANG))
							remove = true;
							
						break;
						
					default: // Just throw the script weapons in here...
						if(ew->id>=wScript1 && ew->id<=wScript10)
						{
							if((stomp.misc2 & shSCRIPT))
								remove = true;
						}
						
						break;
				}
				if (remove)
				{
					ew->onhit(false);
					sfx(WAV_CHINK,pan(x.getInt()));
					continue;
				}
			}
		
			int32_t defresult = defend(ew);
			if ( defresult == -1 ) return -1; //The weapon did something special, but it is otherwise ignored, possibly killed by defend(). 
				
			if(ew->id==ewWind)
			{
				xofs=1000;
				action=freeze; FFCore.setHeroAction(freeze);
				ew->misc=999;                                         // in enemy wind
				attackclk=0;
				return -1;
			}
			
			switch(ew->id)
			{
				case ewLitBomb:
				case ewBomb:
				case ewLitSBomb:
				case ewSBomb:
					return i;
			}
			
			int32_t itemid = getCurrentShield(false);
			if(itemid<0 || !(checkbunny(itemid) && checkmagiccost(itemid)))
				return i;
			itemdata const& shield = itemsbuf[itemid];
			bool allow_inactive = (shield.flags & ITEM_FLAG9);
			auto cmpdir = compareDir(ew->dir);
			bool hitshield = compareShield(cmpdir, shield);
			
			if(!allow_inactive && ((lift_wpn && (liftflags & LIFTFL_DIS_SHIELD)) || (action==attacking||action==sideswimattacking) || action==swimming || action == sideswimming || action == sideswimattacking || charging > 0 || spins > 0 || hopclk==0xFF))
				return i;
			
			if(!hitshield)
				return i;
			
			paymagiccost(itemid);
			
			bool reflect = false;
			
			switch(ew->id)
			{
				case ewFireball2:
				case ewFireball:
					if(ew->type & 1) //Boss fireball
					{
						if(!(shield.misc1 & (shFIREBALL2)))
							return i;
							
						reflect = ((shield.misc2 & shFIREBALL2) != 0);
					}
					else
					{
						if(!(shield.misc1 & (shFIREBALL)))
							return i;
							
						reflect = ((shield.misc2 & shFIREBALL) != 0);
					}
					
					break;
					
				case ewMagic:
					if(!(shield.misc1 & shMAGIC))
						return i;
						
					reflect = ((shield.misc2 & shMAGIC) != 0);
					break;
					
				case ewSword:
					if(!(shield.misc1 & shSWORD))
						return i;
						
					reflect = ((shield.misc2 & shSWORD) != 0);
					break;
					
				case ewFlame:
					if(!(shield.misc1 & shFLAME))
						return i;
						
					reflect = ((shield.misc2 & shFLAME) != 0); // Actually isn't reflected.
					break;
					
				case ewRock:
					if(!(shield.misc1 & shROCK))
						return i;
						
					reflect = (shield.misc2 & shROCK);
					break;
					
				case ewArrow:
					if(!(shield.misc1 & shARROW))
						return i;
						
					reflect = ((shield.misc2 & shARROW) != 0); // Actually isn't reflected.
					break;
					
				case ewBrang:
					if(!(shield.misc1 & shBRANG))
						return i;
						
					break;
					
				default: // Just throw the script weapons in here...
					if(ew->id>=wScript1 && ew->id<=wScript10)
					{
						if(!(shield.misc1 & shSCRIPT))
							return i;
							
						reflect = ((shield.misc2 & shSCRIPT) != 0);
					}
					
					break;
			}
			
			if(reflect && (ew->unblockable&WPNUNB_REFL))
				reflect = false;
			if(!reflect && (ew->unblockable&WPNUNB_SHLD))
				return i;
			
			int32_t oldid = ew->id;
			ew->onhit(false, reflect ? 2 : 1, dir);
			
			if(ew->id != oldid)                                     // changed type from ewX to wX
			{
				//        ew->power*=game->get_hero_dmgmult();
				Lwpns.add(ew);
				Ewpns.remove(ew);
				ew->isLWeapon = true; //Make sure this gets set everywhere!
			}
			
			if(ew->id==wRefMagic)
			{
				ew->ignoreHero=true;
				ew->ignorecombo=-1;
			}
			
			sfx(shield.usesound,pan(x.getInt()));
		}
	}
	
	return -1;
}

int32_t HeroClass::LwpnHit()                                    //only here to check magic hits
{
	for(int32_t i=0; i<Lwpns.Count(); i++)
		if(Lwpns.spr(i)->hit(x+7,y+7-fakez,z,2,2,1))
		{
			weapon *lw = (weapon*)(Lwpns.spr(i));
			
			if((lw->ignoreHero)==true)
				break;
			
			if (!(lw->id == wRefFireball || lw->id == wRefMagic || lw->id == wRefBeam || lw->id == wRefRock)) return -1;
			int32_t itemid = getCurrentShield(false);
			if(itemid<0 || !(checkbunny(itemid) && checkmagiccost(itemid)))
				return i;
			itemdata const& shield = itemsbuf[itemid];
			auto cmpdir = compareDir(lw->dir);
			bool hitshield = compareShield(cmpdir, shield);
			bool reflect = false;
			
			switch(lw->id)
			{
			case wRefFireball:
				if(itemid<0)
					return i;
					
				if(lw->type & 1)  //Boss fireball
					return i;
					
				if(!(shield.misc1 & (shFIREBALL)))
					return i;
					
				reflect = ((shield.misc2 & shFIREBALL) != 0);
				break;
				
			case wRefMagic:
				if(itemid<0)
					return i;
					
				if(!(shield.misc1 & shMAGIC))
					return i;
					
				reflect = ((shield.misc2 & shMAGIC) != 0);
				break;
				
			case wRefBeam:
				if(itemid<0)
					return i;
					
				if(!(shield.misc1 & shSWORD))
					return i;
					
				reflect = ((shield.misc2 & shSWORD) != 0);
				break;
				
			case wRefRock:
				if(itemid<0)
					return i;
					
				if(!(shield.misc1 & shROCK))
					return i;
					
				reflect = (shield.misc2 & shROCK);
				break;
				
			default:
				return -1;
			}
			
			bool allow_inactive = (shield.flags & ITEM_FLAG9);
			if(!allow_inactive && ((lift_wpn && (liftflags & LIFTFL_DIS_SHIELD)) || (action==attacking||action==sideswimattacking) || action==swimming || action == sideswimming || action == sideswimattacking || charging > 0 || spins > 0 || hopclk==0xFF))
				return i;
			
			if(!hitshield)
				return i;
				
			if(itemid<0 || !(checkbunny(itemid) && checkmagiccost(itemid))) return i;
			
			paymagiccost(itemid);
			
			lw->onhit(false, 1+reflect, dir);
			lw->ignoreHero=true;
			lw->ignorecombo=-1;
			sfx(shield.usesound,pan(x.getInt()));
		}
		
	return -1;
}

void HeroClass::checkhit()
{
	if(checkhero==true)
	{
		if(hclk>0)
		{
			--hclk;
		}
		
		if(DivineProtectionShieldClk>0)
		{
			--DivineProtectionShieldClk;
			
			if(DivineProtectionShieldClk == 0 && div_prot_item != -1)
			{
				stop_sfx(itemsbuf[div_prot_item].usesound);
				stop_sfx(itemsbuf[div_prot_item].usesound+1);
				div_prot_item = -1;
			}
			else if(get_qr(qr_MORESOUNDS) && !(DivineProtectionShieldClk&0xF00) && div_prot_item != -1)
			{
				stop_sfx(itemsbuf[div_prot_item].usesound);
				cont_sfx(itemsbuf[div_prot_item].usesound+1);
			}
		}
	}
	
	if(hclk<39 && action==gothit)
	{
		action=none; FFCore.setHeroAction(none);
	}
		
	if(hclk<39 && (action==swimhit || action == sideswimhit))
	{
		SetSwim();
	}
		
	if(hclk>=40 && action==gothit)
	{
		int val = check_pitslide();
		if(((ladderx+laddery) && ((hitdir&2)==ladderdir))||(!(ladderx+laddery)))
		{
			for(int32_t i=0; i<4; i++)
			{
				switch(hitdir)
				{
					case up:
						if(hit_walkflag(x,y+(bigHitbox?-1:7),2)||(x.getInt()&7?hit_walkflag(x+16,y+(bigHitbox?-1:7),1):0))	
						{
							action=none; FFCore.setHeroAction(none);
						}
						else if (val == -1) --y;
						
						break;
						
					case down:
						if(hit_walkflag(x,y+16,2)||(x.getInt()&7?hit_walkflag(x+16,y+16,1):0))   
						{
							action=none; FFCore.setHeroAction(none);
						}
						else if (val == -1) ++y;
						
						break;
						
					case left:
						if(hit_walkflag(x-1,y+(bigHitbox?0:8),1)||hit_walkflag(x-1,y+8,1)||(y.getInt()&7?hit_walkflag(x-1,y+16,1):0))  
						{
							action=none; FFCore.setHeroAction(none);
						}
						else if (val == -1) --x;
						
						break;
						
					case right:
						if(hit_walkflag(x+16,y+(bigHitbox?0:8),1)||hit_walkflag(x+16,y+8,1)||(y.getInt()&7?hit_walkflag(x+16,y+16,1):0))
						{
							action=none; FFCore.setHeroAction(none);
						}
						else if (val == -1) ++x;
						
						break;
				}
			}
		}
	}
	
	if(hclk>0 || inlikelike == 1 || action==inwind || action==drowning || action==lavadrowning || action==sidedrowning || inwallm || isDiving() || (action==hopping && hopclk<255))
	{
		return;
	}
	
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		sprite *s = Lwpns.spr(i);
		int32_t itemid = ((weapon*)(Lwpns.spr(i)))->parentitem;
		//if ( itemdbuf[parentitem].flags&ITEM_FLAGS3 ) //can damage Hero
		//if ( itemsbuf[parentitem].misc1 > 0 ) //damages Hero by this amount. 
		if((!(itemid==-1&&get_qr(qr_FIREPROOFHERO)||((itemid>-1&&itemsbuf[itemid].family==itype_candle||itemsbuf[itemid].family==itype_book)&&(itemsbuf[itemid].flags & ITEM_FLAG3)))) && (scriptcoldet&1) && !fallclk && (!superman || !get_qr(qr_FIREPROOFHERO2)))
		{
			if(s->id==wFire && (superman ? (diagonalMovement?s->hit(x+4,y+4-fakez,z,7,7,1):s->hit(x+7,y+7-fakez,z,2,2,1)) : s->hit(this))&&
						(itemid < 0 || itemsbuf[itemid].family!=itype_divinefire))
			{
				std::vector<int32_t> &ev = FFCore.eventData;
				ev.clear();
				ev.push_back(lwpn_dp(i)*10000);
				ev.push_back(s->hitdir(x,y,16,16,dir)*10000);
				ev.push_back(0);
				ev.push_back(DivineProtectionShieldClk>0?10000:0);
				ev.push_back(48*10000);
				ev.push_back(ZSD_LWPN*10000);
				ev.push_back(s->getUID());
				ev.push_back(ZSD_NONE*10000);
				ev.push_back(0);
				
				throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
				int32_t dmg = ev[0]/10000;
				bool nullhit = ev[2] != 0;
				
				if(nullhit) {ev.clear(); return;}
				
				//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
				ev[0] = ringpower(dmg)*10000;
				
				throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
				dmg = ev[0]/10000;
				int32_t hdir = ev[1]/10000;
				nullhit = ev[2] != 0;
				bool divineprot = ev[3] != 0;
				int32_t iframes = ev[4] / 10000;
				ev.clear();
				if(nullhit) return;
				if (Lwpns.spr(i) != s)
				{
					auto hit = Lwpns.find(s);
					if (hit < 0)
						s = nullptr;
					else
					{
						s = Lwpns.spr(hit);
						i = hit;
					}
				}
				if(!divineprot)
				{
					game->set_life(zc_max(game->get_life()-dmg,0));
					weapon* w = (weapon*)s;
					if (!get_qr(qr_BROKENHITBY) && w)
					{
						sethitHeroUID(HIT_BY_LWEAPON,(i+1));
						if (get_qr(qr_BROKENHITBY))
						{
							sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
						}
						else
						{
							
							sethitHeroUID(HIT_BY_LWEAPON_UID,w->getScriptUID());
						}
						sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
						sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
						if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
						else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
						sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].family);
					}
				}
				
				hitdir = hdir;
				
				if (action != rafting && action != freeze && action != sideswimfreeze)
				{
					if (IsSideSwim())
					{
						action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
					}
					else if (action == swimming || hopclk == 0xFF)
					{
						action=swimhit; FFCore.setHeroAction(swimhit);
					}
					else
					{
						action=gothit; FFCore.setHeroAction(gothit);
					}
				}
					
				if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
				{
					spins = charging = attackclk = 0;
					attack=none;
					tapping = false;
				}
				
				hclk=iframes;
				sfx(getHurtSFX(),pan(x.getInt()));
				return;
			}
		}
		
		//   check enemy weapons true, 1, -1
		//
		if((itemsbuf[itemid].flags & ITEM_FLAG6))
		{
			if(s->id==wBrang || (s->id==wHookshot&&!pull_hero))
			{
				int32_t itemid = ((weapon*)s)->parentitem>-1 ? ((weapon*)s)->parentitem :
							 directWpn>-1 ? directWpn : current_item_id(s->id==wHookshot ? (((weapon*)s)->family_class == itype_switchhook ? itype_switchhook : itype_hookshot) : itype_brang);
				itemid = vbound(itemid, 0, MAXITEMS-1);
				
				for(int32_t j=0; j<Ewpns.Count(); j++)
				{
					sprite *t = Ewpns.spr(j);
					
					if(s->hit(t->x+7,t->y+7-t->fakez,t->z,2,2,1))
					{
						bool reflect = false;
					   // sethitHeroUID(HIT_BY_EWEAPON,j); //set that Hero was hit by a specific eweapon index. 
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
							int32_t mask = (((weapon*)t)->type&1 ? shFIREBALL2 : shFIREBALL);
							
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
							int32_t oldid = ew->id;
							ew->onhit(true, reflect ? 2 : 1, s->dir);
							
							if(ew->id != oldid || (ew->id>=wScript1 && ew->id<=wScript10)) // changed type from ewX to wX... Except for script weapons
							{
								Lwpns.add(ew);
								Ewpns.remove(ew);
				ew->isLWeapon = true; //Make sure this gets set everywhere!
							}
							
							if(ew->id==wRefMagic)
							{
								ew->ignoreHero=true;
								ew->ignorecombo=-1;
							}
							
							break;
						}
						
						break;
					}
				}
			}
		}
		
		if((itemsbuf[itemid].flags & ITEM_FLAG2)||(itemid==-1&&get_qr(qr_OUCHBOMBS)))
		{
			if(((s->id==wBomb)||(s->id==wSBomb)) && !superman && (scriptcoldet&1) && !fallclk)
			{
				weapon* w = (weapon*)s;
				bool didhit = s->hit(this);
				if(didhit)
				{
					std::vector<int32_t> &ev = FFCore.eventData;
					ev.clear();
					ev.push_back(((w->parentitem>-1 ? itemsbuf[w->parentitem].misc3 : w->power) *game->get_hp_per_heart())*10000);
					ev.push_back(w->hitdir(x,y,16,16,dir)*10000);
					ev.push_back(0);
					ev.push_back(DivineProtectionShieldClk>0?10000:0);
					ev.push_back(48*10000);
					ev.push_back(ZSD_LWPN*10000);
					ev.push_back(w->getUID());
					ev.push_back(ZSD_NONE*10000);
					ev.push_back(0);
					
					throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
					int32_t dmg = ev[0]/10000;
					bool nullhit = ev[2] != 0;
					
					if(nullhit) {ev.clear(); return;}
					
					//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
					ev[0] = ringpower(dmg)*10000;
					
					throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
					dmg = ev[0]/10000;
					int32_t hdir = ev[1]/10000;
					nullhit = ev[2] != 0;
					bool divineprot = ev[3] != 0;
					int32_t iframes = ev[4] / 10000;
					ev.clear();
					if(nullhit) return;
					if (Lwpns.spr(i) != w)
					{
						auto hit = Lwpns.find(w);
						if (hit < 0)
							s = nullptr;
						else
						{
							s = Lwpns.spr(hit);
							i = hit;
						}
						w = (weapon*)s;
					}
					if(!divineprot)
					{
						game->set_life(zc_min(game->get_maxlife(), zc_max(game->get_life()-dmg,0)));
						if (!get_qr(qr_BROKENHITBY) && w)
						{
							sethitHeroUID(HIT_BY_LWEAPON,(i+1));
							if (get_qr(qr_BROKENHITBY))
							{
								sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
							}
							else
							{
								
								sethitHeroUID(HIT_BY_LWEAPON_UID,w->getScriptUID());
							}
							sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
							sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
							if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
							else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
							sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].family);
						}
					}
					
					hitdir = hdir;
					
					if (action != rafting && action != freeze && action != sideswimfreeze)
					{
						if (IsSideSwim())
						{
							action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
						}
						else if (action == swimming || hopclk == 0xFF)
						{
							action=swimhit; FFCore.setHeroAction(swimhit);
						}
						else
						{
							action=gothit; FFCore.setHeroAction(gothit);
						}
					}
						
					if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
					{
						spins = charging = attackclk = 0;
						attack=none;
						tapping = false;
					}
					
					hclk=iframes;
					sfx(getHurtSFX(),pan(x.getInt()));
					return;
				}
			}
		}
		
		if(hclk==0 && s->id==wWind && s->hit(x+7,y+7-fakez,z,2,2,1) && !fairyclk)
		{
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(0);
			ev.push_back(s->dir*10000);
			ev.push_back(0);
			ev.push_back(0);
			ev.push_back(ZSD_LWPN*10000);
			ev.push_back(s->getUID());
			ev.push_back(ZSD_NONE*10000);
			ev.push_back(0);
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
			bool nullhit = ev[2] != 0;
			if(nullhit) {ev.clear(); return;}
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
			int32_t hdir = ev[1]/10000;
			nullhit = ev[2] != 0;
			ev.clear();
			if(nullhit) return;
			if (Lwpns.spr(i) != s)
			{
				auto hit = Lwpns.find(s);
				if (hit < 0)
					s = nullptr;
				else
				{
					s = Lwpns.spr(hit);
					i = hit;
				}
			}
			
			reset_hookshot();
			xofs=1000;
			action=inwind; FFCore.setHeroAction(inwind);
			dir = hdir;
			if(s) s->dir = hdir;
			spins = charging = attackclk = 0;
			
			// In case Hero used two whistles in a row, summoning two whirlwinds,
			// check which whistle's whirlwind picked him up so the correct
			// warp ring will be used
			int32_t whistle=s ? ((weapon*)s)->parentitem : -1;
			
			if(whistle>-1 && itemsbuf[whistle].family==itype_whistle)
				whistleitem=whistle;
				
			return;
		}
	}
	
	if(action==rafting || action==freeze || action==sideswimfreeze ||
			action==casting || action==sideswimcasting || action==drowning || action==lavadrowning || action==sidedrowning)
		return;
	
	int32_t hit2 = -1;
	do
	{
		hit2 = diagonalMovement ? GuyHitFrom(hit2+1,x+4,y+4-fakez,z,8,8,hzsz)
			: GuyHitFrom(hit2+1,x+7,y+7-fakez,z,2,2,hzsz);
		
		if(hit2!=-1)
		{
			if (hithero(hit2) == 0) return;
		}
	} while (hit2 != -1);
	if (superman || !(scriptcoldet&1) || fallclk) return;
	hit2 = LwpnHit();
	
	if(hit2!=-1)
	{
		weapon* lwpnspr = (weapon*)Lwpns.spr(hit2);
		std::vector<int32_t> &ev = FFCore.eventData;
		ev.clear();
		ev.push_back((lwpn_dp(hit2)*10000));
		ev.push_back(lwpnspr->hitdir(x,y,16,16,dir)*10000);
		ev.push_back(0);
		ev.push_back(DivineProtectionShieldClk>0?10000:0);
		ev.push_back(48*10000);
		ev.push_back(ZSD_LWPN*10000);
		ev.push_back(lwpnspr->getUID());
		ev.push_back(ZSD_NONE*10000);
		ev.push_back(0);
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
		int32_t dmg = ev[0]/10000;
		bool nullhit = ev[2] != 0;
		
		if(nullhit) {ev.clear(); return;}
		
		//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
		ev[0] = ringpower(dmg)*10000;
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
		dmg = ev[0]/10000;
		int32_t hdir = ev[1]/10000;
		nullhit = ev[2] != 0;
		bool divineprot = ev[3] != 0;
		int32_t iframes = ev[4] / 10000;
		ev.clear();
		if(nullhit) return;
		if (Lwpns.spr(hit2) != lwpnspr)
		{
			hit2 = Lwpns.find(lwpnspr);
			if (hit2 < 0)
				lwpnspr = nullptr;
			else lwpnspr = (weapon*)Lwpns.spr(hit2);
		}
		if (!divineprot)
		{
			game->set_life(zc_max(game->get_life() - dmg, 0));
			if (lwpnspr)
			{
				sethitHeroUID(HIT_BY_LWEAPON, (hit2 + 1));
				if (get_qr(qr_BROKENHITBY))
				{
					sethitHeroUID(HIT_BY_LWEAPON_UID, lwpnspr->getUID());
				}
				else
				{

					sethitHeroUID(HIT_BY_LWEAPON_UID, lwpnspr->getScriptUID());
				}
				sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID, lwpnspr->getUID());
				sethitHeroUID(HIT_BY_LWEAPON_TYPE, lwpnspr->id);
				if (lwpnspr->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, lwpnspr->parentitem);
				else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
				sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[lwpnspr->parentitem].family);
			}
		}
		
		hitdir = hdir;
		if(lwpnspr)
			lwpnspr->onhit(false);
		
		if (IsSideSwim())
		{
			action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
		}
		else if(action==swimming || hopclk==0xFF)
		{
			action=swimhit; FFCore.setHeroAction(swimhit);
		}
		else
		{
			action=gothit; FFCore.setHeroAction(gothit);
		}
			
		hclk=iframes;
		
		if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
		{
			spins = charging = attackclk = 0;
			attack=none;
			tapping = false;
		}
		
		sfx(getHurtSFX(),pan(x.getInt()));
		return;
	}
	
	//else  { sethitHeroUID(HIT_BY_LWEAPON,(0));  //fails to clear
	
	hit2 = EwpnHit();
	
	if(hit2!=-1)
	{
		weapon* ewpnspr = (weapon*)Ewpns.spr(hit2);
		std::vector<int32_t> &ev = FFCore.eventData;
		ev.clear();
		ev.push_back((ewpn_dp(hit2)*10000));
		ev.push_back(ewpnspr->hitdir(x,y,16,16,dir)*10000);
		ev.push_back(0);
		ev.push_back(DivineProtectionShieldClk>0?10000:0);
		ev.push_back(48*10000);
		ev.push_back(ZSD_EWPN*10000);
		ev.push_back(ewpnspr->getUID());
		ev.push_back(ZSD_NONE*10000);
		ev.push_back(0);
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
		int32_t dmg = ev[0]/10000;
		bool nullhit = ev[2] != 0;
		
		if(nullhit) {ev.clear(); return;}
		
		//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
		ev[0] = ringpower(dmg)*10000;
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
		dmg = ev[0]/10000;
		int32_t hdir = ev[1]/10000;
		nullhit = ev[2] != 0;
		bool divineprot = ev[3] != 0;
		int32_t iframes = ev[4] / 10000;
		ev.clear();
		if (nullhit) return;
		if (Ewpns.spr(hit2) != ewpnspr)
		{
			hit2 = Ewpns.find(ewpnspr);
			if (hit2 < 0)
				ewpnspr = nullptr;
			else ewpnspr = (weapon*)Ewpns.spr(hit2);
		}
		if (!divineprot)
		{
			game->set_life(zc_max(game->get_life() - dmg, 0));
			if (ewpnspr)
			{
				sethitHeroUID(HIT_BY_EWEAPON, (hit2 + 1));
				if (get_qr(qr_BROKENHITBY))
				{
					sethitHeroUID(HIT_BY_EWEAPON_UID, ewpnspr->getUID());
				}
				else
				{

					sethitHeroUID(HIT_BY_EWEAPON_UID, ewpnspr->getScriptUID());
				}
				sethitHeroUID(HIT_BY_EWEAPON_ENGINE_UID, ewpnspr->getUID());
				sethitHeroUID(HIT_BY_EWEAPON_TYPE, ewpnspr->id);
			}
		}
		
		hitdir = hdir;
		if(ewpnspr)
			ewpnspr->onhit(false);
		
		if (IsSideSwim())
		{
			action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
		}
		else if(action==swimming || hopclk==0xFF)
		{
			action=swimhit; FFCore.setHeroAction(swimhit);
		}
		else
		{
			action=gothit; FFCore.setHeroAction(gothit);
		}
			
		hclk=iframes;
		
		if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
		{
			spins = charging = attackclk = 0;
			attack=none;
			tapping = false;
		}
		
		sfx(getHurtSFX(),pan(x.getInt()));
		return;
	}
	
	// The rest of this method deals with damage combos, which can be jumped over.
	// TODO z3
	if((z>0 || fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) return;
	
	int32_t dx1 = (int32_t)x+8-(tmpscr->csensitive);
	int32_t dx2 = (int32_t)x+8+(tmpscr->csensitive-1);
	int32_t dy1 = (int32_t)y+(bigHitbox?8:12)-(bigHitbox?tmpscr->csensitive:(tmpscr->csensitive+1)/2);
	int32_t dy2 = (int32_t)y+(bigHitbox?8:12)+(bigHitbox?tmpscr->csensitive-1:((tmpscr->csensitive+1)/2)-1);
	
	for(int32_t i=get_qr(qr_DMGCOMBOLAYERFIX) ? 1 : -1; i>=-1; i--)  // Layers 0, 1 and 2!!
		checkdamagecombos(dx1,dx2,dy1,dy2,i);
}

bool HeroClass::checkdamagecombos(int32_t dx, int32_t dy)
{
    return checkdamagecombos(dx,dx,dy,dy);
}

void HeroClass::doHit(int32_t hdir)
{
	hitdir = hdir;
	
	if (action != rafting && action != freeze && action != sideswimfreeze)
	{
		if (IsSideSwim())
		{
			action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
		}
		else if (action == swimming || hopclk == 0xFF)
		{
			action=swimhit; FFCore.setHeroAction(swimhit);
		}
		else
		{
			action=gothit; FFCore.setHeroAction(gothit);
		}
	}
		
	hclk=48;
	
	if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
	{
		spins = charging = attackclk = 0;
		attack=none;
		tapping = false;
	}
	
	sfx(getHurtSFX(),pan(x.getInt()));
}

bool HeroClass::checkdamagecombos(int32_t dx1, int32_t dx2, int32_t dy1, int32_t dy2, int32_t layer, bool solid, bool do_health_check) //layer = -1, solid = false, do_health_check = true
{
	if(hclk || superman || fallclk)
		return false;
		
	int32_t hp_mod[4] = {0};
	int32_t cid[8];
	byte hasKB = 0;
	
	{
		cid[0] = layer>-1?MAPCOMBO2(layer,dx1,dy1):MAPCOMBO(dx1,dy1);
		newcombo& cmb = combobuf[cid[0]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1) 
				hp_mod[0] = cmb.attributes[0] / -10000L;
			else 
				hp_mod[0]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<0;
		}
	}
	{
		cid[1] = layer>-1?MAPCOMBO2(layer,dx1,dy2):MAPCOMBO(dx1,dy2);
		newcombo& cmb = combobuf[cid[1]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1) 
				hp_mod[1] = cmb.attributes[0] / -10000L;
			else 
				hp_mod[1]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<1;
		}
	}
	{
		cid[2] = layer>-1?MAPCOMBO2(layer,dx2,dy1):MAPCOMBO(dx2,dy1);
		newcombo& cmb = combobuf[cid[2]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1) 
				hp_mod[2] = cmb.attributes[0] / -10000L;
			else 
				hp_mod[2]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<2;
		}
	}
	{
		cid[3] = layer>-1?MAPCOMBO2(layer,dx2,dy2):MAPCOMBO(dx2,dy2);
		newcombo& cmb = combobuf[cid[3]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1) 
				hp_mod[3] = cmb.attributes[0] / -10000L;
			else 
				hp_mod[3]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<3;
		}
	}
	
	int32_t bestcid=0;
	rpos_t best_rpos = rpos_t::None;
	int32_t hp_modtotal=0;
	rpos_t rposes[] = {COMBOPOS_REGION(dx1,dy1),COMBOPOS_REGION(dx1,dy2),COMBOPOS_REGION(dx2,dy1),COMBOPOS_REGION(dx2,dy2)};
	if (!_effectflag(dx1,dy1,1, layer)) {hp_mod[0] = 0; hasKB &= ~(1<<0);}
	if (!_effectflag(dx1,dy2,1, layer)) {hp_mod[1] = 0; hasKB &= ~(1<<1);}
	if (!_effectflag(dx2,dy1,1, layer)) {hp_mod[2] = 0; hasKB &= ~(1<<2);}
	if (!_effectflag(dx2,dy2,1, layer)) {hp_mod[3] = 0; hasKB &= ~(1<<3);}
	
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && !_walkflag_layer(dx1,dy1,i)) {hp_mod[0] = 0; hasKB &= ~(1<<0);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && !_walkflag_layer(dx1,dy2,i)) {hp_mod[1] = 0; hasKB &= ~(1<<1);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && !_walkflag_layer(dx2,dy1,i)) {hp_mod[2] = 0; hasKB &= ~(1<<2);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && !_walkflag_layer(dx2,dy2,i)) {hp_mod[3] = 0; hasKB &= ~(1<<3);}
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && _effectflag_layer(dx1,dy1,i)) {hp_mod[0] = 0; hasKB &= ~(1<<0);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && _effectflag_layer(dx1,dy2,i)) {hp_mod[1] = 0; hasKB &= ~(1<<1);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && _effectflag_layer(dx2,dy1,i)) {hp_mod[2] = 0; hasKB &= ~(1<<2);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && _effectflag_layer(dx2,dy2,i)) {hp_mod[3] = 0; hasKB &= ~(1<<3);}
		}
	}
	
	for(int32_t i=0; i<4; i++)
	{
		if(get_qr(qr_DMGCOMBOPRI))
		{
			if(hp_modtotal >= 0) //Okay, if it's over 0, it's healing Hero.
			{
				if(hp_mod[i] < hp_modtotal)
				{
					hp_modtotal = hp_mod[i];
					bestcid = cid[i];
					best_rpos = rposes[i];
				}
			}
			else if(hp_mod[i] < 0) //If it's under 0, it's hurting Hero.
			{
				if(hp_mod[i] > hp_modtotal)
				{
					hp_modtotal = hp_mod[i];
					bestcid = cid[i];
					best_rpos = rposes[i];
				}
			}
		}
		else if(hp_mod[i] < hp_modtotal)
		{
			hp_modtotal = hp_mod[i];
			bestcid = cid[i];
			best_rpos = rposes[i];
		}
	}

	int32_t ffc_ids[] = {-1, -1, -1, -1};
	{
		auto ffc_handle = getFFCAt(dx1,dy1);
		ffc_ids[0] = ffc_handle ? ffc_handle->id : -1;
		cid[4] = ffc_handle ? ffc_handle->data() : 0;
		newcombo& cmb = combobuf[cid[4]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[0] = cmb.attributes[0]/10000L;
			else
				hp_mod[0]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<4;
		}
	}
	{
		auto ffc_handle = getFFCAt(dx1,dy2);
		ffc_ids[1] = ffc_handle ? ffc_handle->id : -1;
		cid[5] = ffc_handle ? ffc_handle->data() : 0;
		newcombo& cmb = combobuf[cid[5]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[1] = cmb.attributes[0]/10000L;
			else
				hp_mod[1]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<5;
		}
	}
	{
		auto ffc_handle = getFFCAt(dx2,dy1);
		ffc_ids[2] = ffc_handle ? ffc_handle->id : -1;
		cid[6] = ffc_handle ? ffc_handle->data() : 0;
		newcombo& cmb = combobuf[cid[6]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[2] = cmb.attributes[0]/10000L;
			else
				hp_mod[2]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<6;
		}
	}
	{
		auto ffc_handle = getFFCAt(dx2,dy2);
		ffc_ids[3] = ffc_handle ? ffc_handle->id : -1;
		cid[7] = ffc_handle ? ffc_handle->data() : 0;
		newcombo& cmb = combobuf[cid[7]];
		if ( !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[3] = cmb.attributes[0]/10000L;
			else
				hp_mod[3]=combo_class_buf[cmb.type].modify_hp_amount;
			if(!(cmb.usrflags&cflag2))
				hasKB |= 1<<7;
		}
	}
	
	int32_t bestffccid = 0;
	int best_ffcid = -1;
	int32_t hp_modtotalffc = 0;
	
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && !_walkflag_layer(dx1,dy1,i,1)) {hp_mod[0] = 0; hasKB &= ~(1<<4);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && !_walkflag_layer(dx1,dy2,i,1)) {hp_mod[1] = 0; hasKB &= ~(1<<5);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && !_walkflag_layer(dx2,dy1,i,1)) {hp_mod[2] = 0; hasKB &= ~(1<<6);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && !_walkflag_layer(dx2,dy2,i,1)) {hp_mod[3] = 0; hasKB &= ~(1<<7);}
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && _effectflag_layer(dx1,dy1,i,1)) {hp_mod[0] = 0; hasKB &= ~(1<<4);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && _effectflag_layer(dx1,dy2,i,1)) {hp_mod[1] = 0; hasKB &= ~(1<<5);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && _effectflag_layer(dx2,dy1,i,1)) {hp_mod[2] = 0; hasKB &= ~(1<<6);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && _effectflag_layer(dx2,dy2,i,1)) {hp_mod[3] = 0; hasKB &= ~(1<<7);}
		}
	}
	
	for(int32_t i=0; i<4; i++)
	{
		if (ffc_ids[i] == -1) continue;

		if(get_qr(qr_DMGCOMBOPRI))
		{
			if(hp_modtotalffc >= 0)
			{
				if(hp_mod[i] < hp_modtotalffc)
				{
					hp_modtotalffc = hp_mod[i];
					bestffccid = cid[4+i];
					best_ffcid = ffc_ids[i];
				}
			}
			else if(hp_mod[i] < 0)
			{
				if(hp_mod[i] > hp_modtotalffc)
				{
					hp_modtotalffc = hp_mod[i];
					bestffccid = cid[4+i];
					best_ffcid = ffc_ids[i];
				}
			}
		}
		else if(hp_mod[i] < hp_modtotalffc)
		{
			hp_modtotalffc = hp_mod[i];
			bestffccid = cid[4+i];
			best_ffcid = ffc_ids[i];
		}
	}
	
	int32_t hp_modmin = zc_min(hp_modtotal, hp_modtotalffc);
	int best_type = 0;
	if(hp_modtotalffc < hp_modtotal)
	{
		bestcid = bestffccid;
		best_type = 1;
	}
	
	bool global_defring = ((itemsbuf[current_item_id(itype_ring)].flags & ITEM_FLAG1));
	bool global_perilring = ((itemsbuf[current_item_id(itype_perilring)].flags & ITEM_FLAG1));

	bool current_ring = ((tmpscr->flags6&fTOGGLERINGDAMAGE) != 0);
	if(current_ring)
	{
		global_defring = !global_defring;
		global_perilring = !global_perilring;
	}
	int32_t itemid = current_item_id(itype_boots);
	
	bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & ITEM_FLAG1);
	bool ignoreBoots = itemid >= 0 && (itemsbuf[itemid].flags & ITEM_FLAG3);
	
	if(hp_modmin<0)
	{
		if((itemid<0) || ignoreBoots || (tmpscr->flags5&fDAMAGEWITHBOOTS) || (4<<current_item_power(itype_boots)<(abs(hp_modmin))) || (solid && bootsnosolid) || !(checkbunny(itemid) && checkmagiccost(itemid)))
		{
			if (!do_health_check) return true;
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(-hp_modmin*10000);
			ev.push_back((hasKB ? dir^1 : -1)*10000);
			ev.push_back(0);
			ev.push_back(DivineProtectionShieldClk>0?10000:0);
			ev.push_back(48*10000);
			ev.push_back(ZSD_COMBODATA*10000);
			ev.push_back(bestcid);
			ev.push_back((best_type ? ZSD_FFC : ZSD_COMBOPOS)*10000);
			ev.push_back((best_type ? best_ffcid : (int)best_rpos)*10000);
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
			int32_t dmg = ev[0]/10000;
			bool nullhit = ev[2] != 0;
			
			if(nullhit) {ev.clear(); return false;}
	
			//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
			ev[0] = ringpower(dmg, !global_perilring, !global_defring)*10000;
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
			dmg = ev[0]/10000;
			int32_t hdir = ev[1]/10000;
			nullhit = ev[2] != 0;
			bool divineprot = ev[3] != 0;
			int32_t iframes = ev[4] / 10000;
			ev.clear();
			if(nullhit) return false;
			
			if(!divineprot)
			{
				game->set_life(zc_max(game->get_life()-dmg,0));
			}
			
			hitdir = hdir;
			doHit(hitdir);
			hclk = iframes;
			return true;
		}
		else if (do_health_check) paymagiccost(itemid); // Boots are successful
	}
	
	return false;
}

int32_t HeroClass::hithero(int32_t hit2, int32_t force_hdir)
{
	if(force_hdir > 3) force_hdir = -1;
	enemy* enemyptr = (enemy*)guys.spr(hit2);
	if(!enemyptr) return 0;
	//printf("Stomp check: %d <= 12, %d < %d\n", int32_t((y+16)-(((enemy*)guys.spr(hit2))->y)), (int32_t)falling_oldy, (int32_t)y);
	int32_t stompid = current_item_id(itype_stompboots);
	if(current_item(itype_stompboots) && checkbunny(stompid) && checkmagiccost(stompid) && (stomping ||
			((z+fakez) > (enemyptr->z+(enemyptr->fakez))) ||
			((isSideViewHero() && (y+16)-(enemyptr->y)<=14) && falling_oldy<y)))
	{
		paymagiccost(stompid);
		hit_enemy(hit2,wStomp,itemsbuf[stompid].power*game->get_hero_dmgmult(),x,y,0,stompid);
		
		if(itemsbuf[stompid].flags & ITEM_FLAG1)
		{
			fall = -(itemsbuf[stompid].misc1);
		}
		
		if(itemsbuf[stompid].flags & ITEM_DOWNGRADE)
			game->set_item(stompid,false);
			
		// Stomp Boots script
		if(itemsbuf[stompid].script != 0 && !(FFCore.doscript(ScriptType::Item, stompid) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
		{
			int i = stompid;
			FFCore.reset_script_engine_data(ScriptType::Item, i);
			ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[stompid].script, i);
			FFCore.deallocateAllScriptOwned(ScriptType::Item, i);
		}
		
		return -1;
	}
	else if(superman || !(scriptcoldet&1) || fallclk)
		return 0;
	//!TODO SOLIDPUSH Enemy flag to make them not deal contact damage
	//!Add a flag check to this if:
	else if (!(enemyptr->stunclk==0 && enemyptr->frozenclock==0 && (!get_qr(qr_SAFEENEMYFADE) || enemyptr->fading != fade_flicker)
			&& (enemyptr->d->family != eeGUY || enemyptr->dmisc1)))
	{
		return -1;
	}
	
	std::vector<int32_t> &ev = FFCore.eventData;
	ev.clear();
	//Args: 'damage (pre-ring)','hitdir','nullifyhit','type:npc','npc uid'
	ev.push_back((enemy_dp(hit2) *10000));
	ev.push_back((force_hdir>-1 ? force_hdir : ((sprite*)enemyptr)->hitdir(x,y,16,16,dir))*10000);
	ev.push_back(0);
	ev.push_back(DivineProtectionShieldClk>0?10000:0);
	ev.push_back(48*10000);
	ev.push_back(ZSD_NPC*10000);
	ev.push_back(enemyptr->getUID());
	ev.push_back(ZSD_NONE*10000);
	ev.push_back(0);
	
	throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
	int32_t dmg = ev[0] / 10000;
	bool nullhit = ev[2] != 0;
	
	if(nullhit) {ev.clear(); return -1;}
	
	//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
	ev[0] = ((ringpower(dmg)*10000));
	
	throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
	dmg = ev[0] / 10000;
	int32_t hdir = ev[1] / 10000;
	nullhit = ev[2] != 0;
	bool divineprot = ev[3] != 0;
	int32_t iframes = ev[4] / 10000;
	ev.clear();
	
	if(nullhit) return -1;
	
	if(!divineprot)
	{
		game->set_life(zc_max(game->get_life()-dmg,0));
		sethitHeroUID(HIT_BY_NPC,(hit2+1));
		sethitHeroUID(HIT_BY_NPC_UID,enemyptr->getUID());
		if (get_qr(qr_BROKENHITBY))
		{
			sethitHeroUID(HIT_BY_NPC_UID,enemyptr->getUID());
		}
		else
		{
			sethitHeroUID(HIT_BY_NPC_UID,enemyptr->script_UID);
		}
		sethitHeroUID(HIT_BY_NPC_ENGINE_UID,enemyptr->getUID());
		sethitHeroUID(HIT_BY_NPC_ID, enemyptr->id);
		sethitHeroUID(HIT_BY_NPC_TYPE, enemyptr->family);
	}
	
	hitdir = hdir;
	if (IsSideSwim())
	{
	   action=sideswimhit; FFCore.setHeroAction(sideswimhit); 
	}
	else if(action==swimming || hopclk==0xFF)
	{
		action=swimhit; FFCore.setHeroAction(swimhit);
	}
	else
	{
		action=gothit; FFCore.setHeroAction(gothit);
	}
		
	hclk=iframes;
	sfx(getHurtSFX(),pan(x.getInt()));
	
	if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
	{
		spins = charging = attackclk = 0;
		attack=none;
		tapping = false;
	}
	
	enemy_scored(hit2);
	int32_t dm7 = enemyptr->dmisc7;
	int32_t dm8 = enemyptr->dmisc8;
	
	switch(enemyptr->family)
	{
		case eeWALLM:
			if(enemyptr->hp>0)
			{
				GrabHero(hit2);
				inwallm=true;
				action=none; FFCore.setHeroAction(none);
			}
			break;
		
		//case eBUBBLEST:
		//case eeBUBBLE:
		case eeWALK:
		{
			int32_t itemid = current_item_id(itype_whispring);
			//I can only assume these are supposed to be int32_t, not bool ~pkmnfrk
			int32_t sworddivisor = ((itemid>-1 && itemsbuf[itemid].misc1 & 1) ? itemsbuf[itemid].power : 1);
			int32_t itemdivisor = ((itemid>-1 && itemsbuf[itemid].misc1 & 2) ? itemsbuf[itemid].power : 1);
			
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
						if(sworddivisor) swordclk=(itemid >-1 && itemsbuf[itemid].flags & ITEM_FLAG1)? int32_t(150/sworddivisor) : -1;
						
					if(dm8==1 || dm8==2)
						if(itemdivisor) itemclk=(itemid >-1 && itemsbuf[itemid].flags & ITEM_FLAG1)? int32_t(150/itemdivisor) : -1;
						
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
			verifyAWpn();
			if(dm7 >= e7tEATITEMS)
			{
				EatHero(hit2);
				inlikelike=(dm7 == e7tEATHURT ? 2:1);
				action=none; FFCore.setHeroAction(none);
			}
		}
	}
	return 0;
}

void HeroClass::addsparkle(int32_t wpn)
{
	//return;
    weapon *w = (weapon*)Lwpns.spr(wpn);
    int32_t itemid = w->parentitem;
    
    if(itemid<0)
        return;
        
    int32_t itemtype = itemsbuf[itemid].family;
    
    if(itemtype!=itype_cbyrna && frame%4)
        return;
        
    int32_t wpn2 = (itemtype==itype_cbyrna) ? itemsbuf[itemid].wpn4 : itemsbuf[itemid].wpn2;
    int32_t wpn3 = (itemtype==itype_cbyrna) ? itemsbuf[itemid].wpn5 : itemsbuf[itemid].wpn3;
    // Either one (wpn2) or the other (wpn3). If both are present, randomise.
    int32_t sparkle_type = (!wpn2 ? (!wpn3 ? 0 : wpn3) : (!wpn3 ? wpn2 : (zc_oldrand()&1 ? wpn2 : wpn3)));
    int32_t direction=w->dir;
    
    if(sparkle_type)
    {
        int32_t h=0;
        int32_t v=0;
        
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
	if(itemtype==itype_brang && get_qr(qr_WRONG_BRANG_TRAIL_DIR)) direction = 0;
		zfix x = w->x+(itemtype==itype_cbyrna ? 2 : zc_oldrand()%4)+(h*4);
		zfix y = w->y+(itemtype==itype_cbyrna ? 2 : zc_oldrand()%4)+(v*4)-w->fakez;
        Lwpns.add(new weapon(x, y, w->z, sparkle_type==wpn3 ? wFSparkle : wSSparkle,sparkle_type,0,direction,itemid,getUID(),false,false,true, 0, sparkle_type));
	weapon *w = (weapon*)(Lwpns.spr(Lwpns.Count()-1));
	}
}

// For wPhantoms
void HeroClass::addsparkle2(int32_t type1, int32_t type2)
{
    if(frame%4) return;
    
    int32_t arrow = -1;
    
    for(int32_t i=0; i<Lwpns.Count(); i++)
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
    
    zfix x = (Lwpns.spr(arrow)->x-3)+(zc_oldrand()%7);
    zfix y = (Lwpns.spr(arrow)->y-3)+(zc_oldrand()%7)-Lwpns.spr(arrow)->fakez;
    Lwpns.add(new weapon(x, y, Lwpns.spr(arrow)->z, wPhantom, type2,0,0,((weapon*)Lwpns.spr(arrow))->parentitem,-1));
}

//cleans up decorations that exit the bounds of the screen for a int32_t time, to prevebt them wrapping around.
void HeroClass::PhantomsCleanup()
{
	if(Lwpns.idCount(wPhantom))
	{
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			weapon *w = ((weapon *)Lwpns.spr(i));
			if ( w->id == wPhantom && !w->isScriptGenerated() )
			{
				if ( w->x < -10000 || w->y > 10000 || w->x < -10000 || w->y > 10000 )
				{
					Lwpns.remove(w);
				}				
			}
		}	
	}
}

//Waitframe handler for refilling operations
static void do_refill_waitframe()
{
	put_passive_subscr(framebuf,0,passive_subscreen_offset,game->should_show_time(),sspUP);
	if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN))
	{
		script_drawing_commands.Clear();
		if(DMaps[currdmap].passive_sub_script != 0)
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script, currdmap);
		
		if (FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[currdmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script, currdmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}	
		do_script_draws(framebuf, tmpscr, 0, playing_field_offset);
	}
	advanceframe(true);
}
//Special handler if it's a "fairy revive"
static void do_death_refill_waitframe()
{
	//!TODO Run a new script slot each frame here, before calling do_refill_waitframe()
	//This script should be able to draw a 'fairy saving the player' animation -Em
	do_refill_waitframe();
}

static size_t find_bottle_for_slot(size_t slot, bool unowned=false)
{
	int32_t found_unowned = -1;
	for(int q = 0; q < MAXITEMS; ++q)
	{
		if(itemsbuf[q].family == itype_bottle && itemsbuf[q].misc1 == slot)
		{
			if(game->get_item(q))
				return q;
			if(unowned)
				found_unowned = q;
		}
	}
	return found_unowned;
}

int32_t getPushDir(int32_t flag)
{
	switch(flag)
	{
		case mfPUSHUD: case mfPUSH4: case mfPUSHU: case mfPUSHUDNS:
		case mfPUSH4NS: case mfPUSHUNS: case mfPUSHUDINS: case mfPUSH4INS:
		case mfPUSHUINS:
			return up;
		case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
			return down;
		case mfPUSHLR: case mfPUSHL: case mfPUSHLRNS: case mfPUSHLNS:
		case mfPUSHLRINS: case mfPUSHLINS:
			return left;
		case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
			return right;
	}
	return -1;
}

void post_item_collect();

bool HeroClass::handle_portal_collide(portal* p)
{
	if(!p) return false;
	p->animate(0);
	if(p->destdmap < 0 || p->destdmap >= MAXDMAPS)
		return false;
	if(abs(x - p->x) < 12
		&& abs(y - p->y) < 12)
	{
		if(p->prox_active)
		{
			//Store some values to restore if 'warp fails'
			int32_t tLastEntrance = lastentrance,
					tLastEntranceDMap = lastentrance_dmap,
					tContScr = game->get_continue_scrn(),
					tContDMap = game->get_continue_dmap();
			int32_t sourcescr = currscr, sourcedmap = currdmap;
			zfix tx = x, ty = y, tz = z;
			x = p->x;
			y = p->y;
			
			int32_t weff = p->weffect,
				wsfx = p->wsfx;
			
			int32_t savep = p->saved_data;
			//After this line, 'p' becomes INVALID!
			FFCore.warp_player(wtIWARP, p->destdmap, p->destscr,
				-1, -1, weff, wsfx, 0, -1);
			
			if(mirrorBonk()) //Invalid landing, warp back!
			{
				action = none; FFCore.setHeroAction(none);
				lastentrance = tLastEntrance;
				lastentrance_dmap = tLastEntranceDMap;
				game->set_continue_scrn(tContScr);
				game->set_continue_dmap(tContDMap);
				x = tx;
				y = ty;
				z = tz;
				FFCore.warp_player(wtIWARP, sourcedmap, sourcescr, -1, -1, weff,
					wsfx, 0, -1);
				handle_portal_prox(&mirror_portal);
				portals.forEach([&](sprite& p)
				{
					handle_portal_prox((portal*)&p);
					return false;
				});
			}
			else game->clear_portal(savep); //Remove portal once used
			return true;
		}
	}
	else p->prox_active = true;
	return false;
}
void HeroClass::handle_portal_prox(portal* p)
{
	if(!p) return;
	p->prox_active = !(abs(x - p->x) < 12 && abs(y - p->y) < 12);
}
// returns true when game over
bool HeroClass::animate(int32_t)
{
	int32_t lsave=0;
	if(immortal > 0)
		--immortal;
	prompt_combo = 0;
	if (onpassivedmg)
	{
		onpassivedmg=false;
	}
	else if (damageovertimeclk)
	{
		damageovertimeclk = 0;
	}
	
	if(lift_wpn)
	{
		auto oldid = lift_wpn->id;
		switch(lift_wpn->id)
		{
			case wLitBomb:
			case wBomb:
			case wLitSBomb:
			case wSBomb:
				if(lift_wpn->misc && get_qr(qr_HELD_BOMBS_EXPLODE)) //timed fuse
				{
					lift_wpn->limited_animate();
					if(lift_wpn->id != oldid)
					{
						lift_wpn->moveflags &= ~FLAG_OBEYS_GRAV;
						drop_liftwpn();
						goto heroanimate_skip_liftwpn;
					}
					++lift_wpn->clk;
				}
				break;
		}
		if(lift_wpn->dead>0)
			--lift_wpn->dead;
		
		if(lift_wpn->dead==0)
		{
			if(lift_wpn->death_spawnitem > -1)
			{
				item* itm = (new item(lift_wpn->x, lift_wpn->y, lift_wpn->z, lift_wpn->death_spawnitem, lift_wpn->death_item_pflags, 0));
				itm->fakez = lift_wpn->fakez;
				items.add(itm);
			}
			if(lift_wpn->death_spawndropset > -1)
			{
				auto itid = select_dropitem(lift_wpn->death_spawndropset);
				if(itid > -1)
				{
					item* itm = (new item(lift_wpn->x, lift_wpn->y, lift_wpn->z, itid, lift_wpn->death_item_pflags, 0));
					itm->fakez = lift_wpn->fakez;
					itm->from_dropset = lift_wpn->death_spawndropset;
					items.add(itm);
				}
			}
			switch(lift_wpn->death_sprite)
			{
				case -2: decorations.add(new dBushLeaves(lift_wpn->x, lift_wpn->y-(lift_wpn->z+lift_wpn->fakez), dBUSHLEAVES, 0, 0)); break;
				case -3: decorations.add(new dFlowerClippings(lift_wpn->x, lift_wpn->y-(lift_wpn->z+lift_wpn->fakez), dFLOWERCLIPPINGS, 0, 0)); break;
				case -4: decorations.add(new dGrassClippings(lift_wpn->x, lift_wpn->y-(lift_wpn->z+lift_wpn->fakez), dGRASSCLIPPINGS, 0, 0)); break;
				default:
					if(lift_wpn->death_sprite < 0) break;
					decorations.add(new comboSprite(lift_wpn->x, lift_wpn->y-(lift_wpn->z+lift_wpn->fakez), dCOMBOSPRITE, 0, lift_wpn->death_sprite));
			}
			if(lift_wpn->death_sfx > 0)
				sfx(lift_wpn->death_sfx, pan(int32_t(lift_wpn->x)));
			delete lift_wpn;
			lift_wpn = nullptr;
		}
heroanimate_skip_liftwpn:;
	}
	
	if(cheats_execute_goto)
	{
		didpit=true;
		pitx=x;
		pity=y;
		dowarp(3,0);
		cheats_execute_goto=false;
		solid_update(false);
		return false;
	}
	
	if(cheats_execute_light)
	{
		naturaldark = !naturaldark;
		lighting(false, false, pal_litOVERRIDE);//Forcibly set permLit, overriding its current setting
		cheats_execute_light = false;
	}
	
	if(action!=climbcovertop&&action!=climbcoverbottom)
	{
		climb_cover_x=-1000;
		climb_cover_y=-1000;
	}
	
	handle_portal_collide(&mirror_portal);
	portals.forEach([&](sprite& p)
	{
		return handle_portal_collide((portal*)&p);
	});
	
	if(z<=8&&fakez<=8) //Tall Grass
	{
		if (get_qr(qr_GRASS_SENSITIVE))
		{
			bool g1 = isGrassType(COMBOTYPE(x+4,y+15)), g2 = isGrassType(COMBOTYPE(x+11,y+15)), g3 = isGrassType(COMBOTYPE(x+4,y+9)), g4 = isGrassType(COMBOTYPE(x+11,y+9));
			if(get_qr(qr_BUSHESONLAYERS1AND2))
			{
				g1 = g1 || isGrassType(COMBOTYPEL(1,x+4,y+15)) || isGrassType(COMBOTYPEL(2,x+4,y+15));
				g2 = g2 || isGrassType(COMBOTYPEL(1,x+11,y+15)) || isGrassType(COMBOTYPEL(2,x+11,y+15));
				g3 = g3 || isGrassType(COMBOTYPEL(1,x+4,y+9)) || isGrassType(COMBOTYPEL(2,x+4,y+9));
				g4 = g4 || isGrassType(COMBOTYPEL(1,x+11,y+9)) || isGrassType(COMBOTYPEL(2,x+11,y+9));
			}
			if(g1 && g2 && g3 && g4)
			{
				int grasscid = MAPCOMBO(x+8,y+12);
				newcombo const& cmb = combobuf[grasscid];
				if(decorations.idCount(dTALLGRASS)==0)
				{
					decorations.add(new dTallGrass(x, y, dTALLGRASS, 0, cmb.attribytes[6]));
				}
				int32_t thesfx = cmb.attribytes[3];
				if (action==walking)
					sfx_no_repeat(thesfx,pan((int32_t)x));
			}
		}
		else
		{
			bool g1 = isGrassType(COMBOTYPE(x,y+15)), g2 = isGrassType(COMBOTYPE(x+15,y+15));
			if(get_qr(qr_BUSHESONLAYERS1AND2))
			{
				g1 = g1 || isGrassType(COMBOTYPEL(1,x,y+15)) || isGrassType(COMBOTYPEL(2,x,y+15));
				g2 = g2 || isGrassType(COMBOTYPEL(1,x+15,y+15)) || isGrassType(COMBOTYPEL(2,x+15,y+15));
			}
			if(g1 && g2)
			{
				int grasscid = MAPCOMBO(x+8,y+15);
				newcombo const& cmb = combobuf[grasscid];
				if(decorations.idCount(dTALLGRASS)==0)
				{
					decorations.add(new dTallGrass(x, y, dTALLGRASS, 0, cmb.attribytes[6]));
				}
				int32_t thesfx = cmb.attribytes[3];
				if (action==walking )
					sfx_no_repeat(thesfx,pan((int32_t)x));
			}
		}
	}
	if(z==0 && fakez==0) //Shallow Water / Custom Walk SFX
	{
		if (get_qr(qr_SHALLOW_SENSITIVE))
		{
			if (action != swimming && action != isdiving && action != drowning && action!=lavadrowning && action!=sidedrowning && action!=rafting && action != falling && !IsSideSwim() && !(ladderx+laddery) && !pull_hero && !toogam)
			{
				bool b1 = iswaterex_z3(FFORCOMBO(x+11,y+15), -1, x+11, y+15, false, false, true, true);
				bool b2 = iswaterex_z3(FFORCOMBO(x+4,y+15), -1, x+4, y+15, false, false, true, true);
				bool b3 = iswaterex_z3(FFORCOMBO(x+11,y+9), -1, x+11, y+9, false, false, true, true);
				bool b4 = iswaterex_z3(FFORCOMBO(x+4,y+9), -1, x+4, y+9, false, false, true, true);

				if (b1 && b2 && b3 && b4)
				{
					int watercheck_x = x.getInt()+7.5, watercheck_y = y.getInt()+12;
					auto combined_handle = get_combined_handle_for_world_xy(watercheck_x, watercheck_y, 0);
					int waterid = combined_handle.data();
					if(waterid)
						waterid = iswaterex(waterid, currmap, currscr, -1, watercheck_x,watercheck_y, false, false, true, true);
					if(waterid)
					{
						newcombo const& watercmb = combobuf[waterid];
						auto ripplesprite = watercmb.attribytes[6];
						if(decorations.idCount(dRIPPLES)==0)
							decorations.add(new dRipples(x, y, dRIPPLES, 0, ripplesprite));
						if (watercmb.usrflags&cflag2)
						{
							if (!(current_item(watercmb.attribytes[2]) > 0 && current_item(watercmb.attribytes[2]) >= watercmb.attribytes[3]))
							{
								onpassivedmg = true;
								if (!damageovertimeclk)
								{
									int32_t curhp = game->get_life();
									auto dmg = watercmb.attributes[1]/10000L;
									auto hitsfx = watercmb.attributes[2]/10000L;
									bool hitstun = dmg < 0 && (watercmb.usrflags&cflag7);
									
									if(game->get_life() == curhp && (watercmb.usrflags&cflag6))
										hitsfx = 0;
									
									std::vector<int32_t> &ev = FFCore.eventData;
									ev.clear();
									ev.push_back(-dmg*10000);
									ev.push_back(-1*10000);
									ev.push_back(0);
									ev.push_back(0);
									ev.push_back(48*10000);
									ev.push_back(ZSD_COMBODATA*10000);
									ev.push_back(waterid);
									ev.push_back((combined_handle.is_ffc() ? ZSD_FFC : ZSD_COMBOPOS)*10000);
									ev.push_back(combined_handle.id()*10000);
									
									throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
									
									if(watercmb.usrflags & cflag5)
										ev[0] = ringpower(ev[0]/10000) * 10000;
									
									throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
									dmg = -ev[0]/10000;
									
									if(!ev[2]) //nullify
									{
										game->set_life(vbound(game->get_life()+dmg, 0, game->get_maxlife()));
										if (hitsfx)
											sfx(hitsfx);
										if (hitstun)
										{
											hclk = ev[4]/10000;
											hitdir = ev[1]/10000;
											action = gothit; FFCore.setHeroAction(gothit);
										}
									}
								}
								if (watercmb.attribytes[1] > 0)
								{
									if (!damageovertimeclk || damageovertimeclk > watercmb.attribytes[1]) damageovertimeclk = watercmb.attribytes[1];
									else --damageovertimeclk;
								}
								else damageovertimeclk = 0;
							}
							else damageovertimeclk = 0;
						}
						else damageovertimeclk = 0;
						int32_t thesfx = watercmb.attribytes[0];
						if (watercmb.type != cSHALLOWWATER || !get_qr(qr_OLD_SHALLOW_SFX))
						{
							thesfx = watercmb.attribytes[5];
						}
						if (action==walking)
							sfx_no_repeat(thesfx,pan((int32_t)x));
					}
				}
			}
		}
		else
		{
			if((COMBOTYPE(x,y+15)==cSHALLOWWATER)&&(COMBOTYPE(x+15,y+15)==cSHALLOWWATER))
			{
				int32_t watercheck = FFORCOMBO(x+7.5,y.getInt()+15);
				auto ripplesprite = combobuf[watercheck].attribytes[6];
				if(decorations.idCount(dRIPPLES)==0)
				{
					decorations.add(new dRipples(x, y, dRIPPLES, 0, ripplesprite));
				}
				if (combobuf[watercheck].usrflags&cflag2)
				{
					if (!(current_item(combobuf[watercheck].attribytes[2]) > 0 && current_item(combobuf[watercheck].attribytes[2]) >= combobuf[watercheck].attribytes[3]))
					{
						onpassivedmg = true;
						if (!damageovertimeclk)
						{
							int32_t curhp = game->get_life();
							if (combobuf[watercheck].usrflags&cflag5) game->set_life(vbound(game->get_life()+ringpower(combobuf[watercheck].attributes[1]/10000L), 0, game->get_maxlife())); //Affected by rings
							else game->set_life(vbound(game->get_life()+(combobuf[watercheck].attributes[1]/10000L), 0, game->get_maxlife()));
							if ((combobuf[watercheck].attributes[2]/10000L) && (game->get_life() != curhp || !(combobuf[watercheck].usrflags&cflag6))) sfx(combobuf[watercheck].attributes[2]/10000L);
						}
						if (combobuf[watercheck].attribytes[1] > 0)
						{
							if (!damageovertimeclk || damageovertimeclk > combobuf[watercheck].attribytes[1]) damageovertimeclk = combobuf[watercheck].attribytes[1];
							else --damageovertimeclk;
						}
						else damageovertimeclk = 0;
					}
					else damageovertimeclk = 0;
				}
				else damageovertimeclk = 0;
				int32_t thesfx = combobuf[watercheck].attribytes[0];
				if (action==walking )
					sfx_no_repeat(thesfx,pan((int32_t)x));
			}
		}
		
		auto cpos = COMBOPOS(x+8,y+(sideview_mode()?16:12));
		for(int q = 0; q < 7; ++q)
		{
			mapscr* lyr = FFCore.tempScreens[q];
			auto cid = lyr->data[cpos];
			newcombo const& cmb = combobuf[cid];
			byte csfx = action == walking ? cmb.sfx_walking : cmb.sfx_standing;
			byte cspr = action == walking ? cmb.spr_walking : cmb.spr_standing;
			if(csfx)
				sfx_no_repeat(csfx);
			auto indx = decorations.idFirst(dCUSTOMWALK);
			if(cspr)
			{
				if(indx < 0)
				{
					if(decorations.add(new customWalkSprite(x, y, dCUSTOMWALK, 0, -1)))
						indx = decorations.Count()-1;
				}
				if(indx > -1)
				{
					if(customWalkSprite* spr = dynamic_cast<customWalkSprite*>(decorations.spr(indx)))
						spr->run_sprite(cspr);
				}
			}
		}
	}
	
	if(stomping)
		stomping = false;
	
	if(getOnSideviewLadder())
	{
		if(!canSideviewLadder() || jumping<0 || fall!=0 || fakefall!=0)
		{
			setOnSideviewLadder(false);
		}
		else if(CANFORCEFACEUP)
		{
			setDir(up);
		}
	}
	
	if(action!=inwind && action!=drowning && action!=lavadrowning && action!= sidedrowning)
	{
		if(!get_qr(qr_OLD_CHEST_COLLISION))
		{
			checkchest(cCHEST);
			checkchest(cLOCKEDCHEST);
			checkchest(cBOSSCHEST);
		}
		if(!get_qr(qr_OLD_LOCKBLOCK_COLLISION))
		{
			checkchest(cLOCKBLOCK);
			checkchest(cBOSSLOCKBLOCK);
		}
	}
	checksigns();
	checkgenpush();
	
	if(isStanding(true) && fall == 0)
	{
		if(extra_jump_count > 0)
			extra_jump_count = 0;
		coyotetime = 0;
	}
	else if(coyotetime < 65535)
	{
		++coyotetime;
	}
	if(can_use_item(itype_hoverboots,i_hoverboots))
	{
		int32_t hoverid = current_item_id(itype_hoverboots);
		if(!(itemsbuf[hoverid].flags & ITEM_FLAG1))
		{
			if(hoverclk < 0) hoverclk = 0;
			hoverflags &= ~HOV_OUT;
		}
	}
	bool platformfell2 = false;
	int32_t gravity3 = (zinit.gravity2/100);
	int32_t termv = (zinit.terminalv);
	int32_t rocs = getRocsPressed();
	if (rocs != -1)
	{
		itemdata const& itm = itemsbuf[rocs];
		if (itm.flags & ITEM_FLAG2) 
		{
			if ((!(itm.flags & ITEM_FLAG3) || fall < 0) && 
				(!(itm.flags & ITEM_FLAG4) || fall > 0)) gravity3 = itm.misc3;
		}
		if (itm.flags & ITEM_FLAG5) 
		{
			termv = itm.misc4;
			if (fall > termv) fall = termv;
		}
	}
	if(sideview_mode())  // Sideview gravity
	{
		//Handle falling through a platform
		bool platformfell = false;
		if (on_sideview_solid_oldpos(x,y,old_x,old_y,true,3) && !on_sideview_solid_oldpos(x,y,old_x,old_y,false,3))
		{
			if (!(!on_sideview_slope(Hero.x, Hero.y,Hero.old_x,Hero.old_y) && (on_sideview_slope(Hero.x,Hero.y+1,Hero.old_x,Hero.old_y) || on_sideview_slope(Hero.x, Hero.y + 2, Hero.old_x, Hero.old_y)) && Down())) platformfell = true;
			y+=1; //Fall down a pixel instantly, through the platform.
			if(fall < 0) fall = 0;
			if(jumping < 0) jumping = 0;
			platformfell2 = true;
		}
		//Unless using old collision, run this check BEFORE moving Hero, to prevent clipping into the ceiling.
		if(!get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON))
		{
			if(fall < 0 && (_walkflag(x+4,y+((bigHitbox||!diagonalMovement)?(fall/100):(fall/100)+8),1,SWITCHBLOCK_STATE) || _walkflag(x+12,y+((bigHitbox||!diagonalMovement)?(fall/100):(fall/100)+8),1,SWITCHBLOCK_STATE)
				|| ((y+(fall/100)<=0) &&
				// Extra checks if Smart Screen Scrolling is enabled
				// TODO z3
				 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)) &&
											   !(tmpscr->flags2&wfUP)) && (nextcombo_solid(up)))))))
			{
				fall = jumping = 0; // Bumped his head
				if(get_qr(qr_OLD_SIDEVIEW_LANDING_CODE))
					y -= y.getInt()%8; //fix coords
				// ... maybe on spikes //this is the change from 2.50.1RC3 that Saffith made, that breaks some old quests. -Z
				if ( !get_qr(qr_OLDSIDEVIEWSPIKES) ) //fix for older sideview quests -Z
				{
					checkdamagecombos(x+4, x+12, y-1, y-1);
				}
			}
		}
		// Fall, unless on a ladder, sideview ladder, rafting, using the hookshot, drowning, sideswimming or cheating.
		if(!(toogam && Up()) && !drownclk && action!=rafting && !IsSideSwim() && !pull_hero && !((ladderx || laddery) && fall>0) && !getOnSideviewLadder())
		{
			int32_t ydiff = fall/(spins && fall<0 ? 200:100);
			//zprint2("ydif is: %d\n", ydiff);
			//zprint2("ydif is: %d\n", (int32_t)fall);
			falling_oldy = y; // Stomp Boots-related variable
			
			if(fall > 0 && (checkSVLadderPlatform(x+4,y+ydiff+15)||checkSVLadderPlatform(x+12,y+ydiff+15)) && (TRUNCATE_TILE(y.getInt()+ydiff+15) != TRUNCATE_TILE(y.getInt()+15)) && !platform_fallthrough())
			{
				ydiff -= (y.getInt()+ydiff)%16;
			}
			if(ydiff && !get_qr(qr_OLD_SIDEVIEW_LANDING_CODE))
			{
				if(ydiff > 0)
				{
					for(auto q = 0; q < ydiff; ++q)
					{
						if(on_sideview_solid_oldpos(x,y+q,old_x,old_y))
						{
							ydiff = q;
							break;
						}
					}
				}
				else if(ydiff < 0)
				{
					for(auto q = 0; q > ydiff; --q)
					{
						if(_walkflag(x+4,y+(bigHitbox?0:8)+q-1,1)
							|| _walkflag(x+12,y+(bigHitbox?0:8)+q,1))
						{
							ydiff = q;
							break;
						}
					}
				}
			}
			y+=ydiff;
			hs_starty+=ydiff;
			
			for(int32_t j=0; j<chainlinks.Count(); j++)
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
		else if(IsSideSwim() && action != sidewaterhold1 && action != sidewaterhold2 && action != sideswimcasting && action != sideswimfreeze)
		{
			fall = hoverclk = jumping = 0;
			inair = false;
			hoverflags = 0;
			if(!DrunkUp() && !DrunkDown() && !DrunkLeft() && !DrunkRight() && !autostep)
			{
				WalkflagInfo info;
				if (game->get_watergrav()<0)
				{
					info = walkflag(x,y+8-(bigHitbox*8)-2,2,up);
					execute(info);
				}
				else
				{
					info = walkflag(x,y+15+2,2,down);
					execute(info);
				}

			    if(!info.isUnwalkable() && (game->get_watergrav() > 0 || iswaterex_z3(MAPCOMBO(x,y+8-(bigHitbox*8)-2), -1, x, y+8-(bigHitbox*8)-2, true, false)))
				{
					y += (game->get_watergrav()/10000.0);
				}
			}
		}
		// Stop hovering/falling if you land on something.
		bool needFall = false;
		if((on_sideview_solid_oldpos(x,y,old_x,old_y) || getOnSideviewLadder())  && !(pull_hero && dir==down) && action!=rafting && !platformfell2)
		{
			stop_item_sfx(itype_hoverboots);
			if(get_qr(qr_OLD_SIDEVIEW_LANDING_CODE))
			{
				if(!getOnSideviewLadder() && (fall > 0 || get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON)))
				{
					y.doFloor();
					y-=(int32_t)y%8; //fix position
				}
			}
			else
			{
				snap_platform();
			}
			fall = hoverclk = jumping = 0;
			inair = false;
			hoverflags = 0;
			
			if(y>=world_h-16 && currscr>=0x70 && !(get_screen_for_world_xy(x, y)->flags2&wfDOWN))  // Landed on the bottommost screen.
				y = world_h-16;
		}
		// Stop hovering if you press down.
		else if((hoverclk>0 || ladderx || laddery) && DrunkDown())
		{
			stop_item_sfx(itype_hoverboots);
			hoverclk = -hoverclk;
			reset_ladder();
			fall = gravity3;
			inair = false;
		}
		else if (hoverclk < 1 && !inair && fall == 0 && !platformfell && !IsSideSwim() && justmoved <= 0)
		{
			zfix my = y + 4;
			needFall = true;
			for (zfix ty = y+1; ty < my; ++ty)
			{
				if (on_sideview_solid_oldpos(x, ty,old_x,old_y, false, 0))
				{
					y = ty;
					if (check_new_slope(x, ty + 1, 16, 16, old_x, old_y, false) < 0)
					{
						if(!slopeid)
							slopeid = get_new_slope(x, ty + 1, 16, 16, old_x, old_y).get_info().slope();
						onplatid = 5;
					}
					needFall = false;
					break;
				}
			}
		}
		else needFall = true;
		// Continue falling.
		
		if(fall <= termv && needFall)
		{
			inair = true;
			if(fall != 0 || hoverclk>0)
				jumping++;
				
			// Bump head if: hit a solid combo from beneath, or hit a solid combo in the screen above this one.
			if(get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON))
			{
				if((_walkflag(x+4,y-(bigHitbox?9:1),0,SWITCHBLOCK_STATE)
					|| (y<=(bigHitbox?9:1) &&
					// Extra checks if Smart Screen Scrolling is enabled
					 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)) &&
												   !(tmpscr->flags2&wfUP)) && (nextcombo_solid(up))))))
						&& fall < 0)
				{
					fall = jumping = 0; // Bumped his head
					
					// ... maybe on spikes //this is the change from 2.50.1RC3 that Saffith made, that breaks some old quests. -Z
					if ( !get_qr(qr_OLDSIDEVIEWSPIKES) ) //fix for older sideview quests -Z
					{
						checkdamagecombos(x+4, x+12, y-1, y-1);
					}
				}
			}
			else
			{
				if((_walkflag(x+4,y+((bigHitbox||!diagonalMovement)?-1:7),1,SWITCHBLOCK_STATE) || _walkflag(x+12,y+((bigHitbox||!diagonalMovement)?-1:7),1,SWITCHBLOCK_STATE)
					|| ((y<=0) &&
					// Extra checks if Smart Screen Scrolling is enabled
					 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(tmpscr->flags&fMAZE)) &&
												   !(tmpscr->flags2&wfUP)) && (nextcombo_solid(up))))))
						&& fall < 0)
				{
					fall = jumping = 0; // Bumped his head
					y -= y.getInt()%8; //fix coords
					// ... maybe on spikes //this is the change from 2.50.1RC3 that Saffith made, that breaks some old quests. -Z
					if ( !get_qr(qr_OLDSIDEVIEWSPIKES) ) //fix for older sideview quests -Z
					{
						checkdamagecombos(x+4, x+12, y-1, y-1);
					}
				}
			}
			
			if(hoverclk > 0)
			{
				if(hoverclk > 0 && !(hoverflags&HOV_INF))
				{
					--hoverclk;
				}
				
				if(!hoverclk && !ladderx && !laddery)
				{
					fall += gravity3;
					hoverflags |= HOV_OUT | HOV_PITFALL_OUT;
				}
			}
			else if(fall+gravity3 > 0 && fall<=0 && can_use_item(itype_hoverboots,i_hoverboots) && !ladderx && !laddery && !(hoverflags & HOV_OUT))
			{
				int32_t itemid = current_item_id(itype_hoverboots);
				if(hoverclk < 0)
					hoverclk = -hoverclk;
				else
				{
					fall = jumping = 0;
					if(itemsbuf[itemid].misc1)
						hoverclk = itemsbuf[itemid].misc1;
					else
					{
						hoverclk = 1;
						hoverflags |= HOV_INF;
					}
					
						
					sfx(itemsbuf[itemid].usesound,pan(x.getInt()));
				}
				if(itemsbuf[itemid].wpn)
					decorations.add(new dHover(x, y, dHOVER, 0));
			}
			else if(!ladderx && !laddery && !getOnSideviewLadder() && !IsSideSwim())
			{
				fall += gravity3;
				
			}
		}
	}
	else // Topdown gravity
	{
		if (!(moveflags & FLAG_NO_FAKE_Z)) fakez-=fakefall/(spins && fakefall>0 ? 200:100);
		if (!(moveflags & FLAG_NO_REAL_Z)) z-=fall/(spins && fall>0 ? 200:100);
		if(z>0||fakez>0)
		{
			switch(action)
			{
				case swimming:
				{
					diveclk=0;
					action=walking; FFCore.setHeroAction(walking);
				
					break;
				}
				case waterhold1:
				{
					action=landhold1; FFCore.setHeroAction(landhold1);
					break;
				}
					
				case waterhold2:
				{
					action=landhold2; FFCore.setHeroAction(landhold2);
					break;
				}
				
				default:
					break;
			}
		}
		
		for(int32_t j=0; j<chainlinks.Count(); j++)
		{
			chainlinks.spr(j)->z=z;
			chainlinks.spr(j)->fakez=fakez;
		}
		
		if(Lwpns.idFirst(wHookshot)>-1)
		{
			Lwpns.spr(Lwpns.idFirst(wHookshot))->z=z;
			Lwpns.spr(Lwpns.idFirst(wHookshot))->fakez=fakez;
		}
		
		if(Lwpns.idFirst(wHSHandle)>-1)
		{
			Lwpns.spr(Lwpns.idFirst(wHSHandle))->z=z;
			Lwpns.spr(Lwpns.idFirst(wHSHandle))->fakez=fakez;
		}
		
		if(z<=0&&!(moveflags & FLAG_NO_REAL_Z))
		{
			if (fakez <= 0 || (moveflags & FLAG_NO_FAKE_Z)) 
			{
				if(fall > 0)
				{
					if((iswaterex_z3(MAPCOMBO(x,y+8), -1, x, y+8, true, false) && ladderx<=0 && laddery<=0) || COMBOTYPE(x,y+8)==cSHALLOWWATER)
						sfx(WAV_ZN1SPLASH,x.getInt());
						
					stomping = true;
				}
			}
			z = fall = 0;
			if (fakez <= 0 || (moveflags & FLAG_NO_FAKE_Z)) 
			{
				jumping = 0;
				if(check_pitslide(true) == -1)
				{
					hoverclk = 0;
					hoverflags = 0;
				}
				else if(hoverclk > 0 && !(hoverflags&HOV_INF))
				{
					if(!--hoverclk)
					{
						hoverflags |= HOV_OUT | HOV_PITFALL_OUT;
					}
				}
			}
		}
		if(fakez<=0&&!(moveflags & FLAG_NO_FAKE_Z))
		{
			if (z <= 0 || (moveflags & FLAG_NO_REAL_Z))
			{
				if(fakefall > 0)
				{
					if((iswaterex_z3(MAPCOMBO(x,y+8), -1, x, y+8, true, false) && ladderx<=0 && laddery<=0) || COMBOTYPE(x,y+8)==cSHALLOWWATER)
						sfx(WAV_ZN1SPLASH,x.getInt());
						
					stomping = true;
				}
			}
			fakez = fakefall = 0;
			if (z <= 0 || (moveflags & FLAG_NO_REAL_Z)) 
			{
				jumping = 0;
				if(check_pitslide(true) == -1)
				{
					hoverclk = 0;
					hoverflags = 0;
				}
				else if(hoverclk > 0 && !(hoverflags&HOV_INF))
				{
					if(!--hoverclk)
					{
						hoverflags |= HOV_OUT | HOV_PITFALL_OUT;
					}
				}
			}
		}
		if(fall <= termv && !(moveflags & FLAG_NO_REAL_Z) && z>0 || fakefall <= termv && !(moveflags & FLAG_NO_FAKE_Z) && fakez > 0)
		{
			if(fall != 0 || fakefall != 0 || hoverclk>0)
				jumping++;
				
			if(hoverclk > 0)
			{
				if(hoverclk > 0 && !(hoverflags&HOV_INF))
				{
					--hoverclk;
				}
				
				if(!hoverclk)
				{
					if (fall <= termv && !(moveflags & FLAG_NO_REAL_Z) && z > 0) fall += gravity3;
					if (fakefall <= termv && !(moveflags & FLAG_NO_FAKE_Z) && fakez > 0) fakefall += gravity3;
					hoverflags |= HOV_OUT | HOV_PITFALL_OUT;
				}
			}
			else if(((fall+(int32_t)(zinit.gravity2 / 100) > 0 && fall<=0 && !(moveflags & FLAG_NO_REAL_Z) && z > 0) || (fakefall+gravity3 > 0 && fakefall<=0 && !(moveflags & FLAG_NO_FAKE_Z) && fakez > 0)) && can_use_item(itype_hoverboots,i_hoverboots) && !(hoverflags & HOV_OUT))
			{
				if(hoverclk < 0)
					hoverclk = -hoverclk;
				else
				{
					fall = 0;
					fakefall = 0;
					int32_t itemid = current_item_id(itype_hoverboots);
					if(itemsbuf[itemid].misc1)
						hoverclk = itemsbuf[itemid].misc1;
					else
					{
						hoverclk = 1;
						hoverflags |= HOV_INF;
					}
					sfx(itemsbuf[current_item_id(itype_hoverboots)].usesound,pan(x.getInt()));
				}
				decorations.add(new dHover(x, y, dHOVER, 0));
			}
			else 
			{
				if (fall <= termv && !(moveflags & FLAG_NO_REAL_Z) && z > 0) fall += gravity3;
				if (fakefall <= termv && !(moveflags & FLAG_NO_FAKE_Z) && fakez > 0) fakefall += gravity3;
			}
		}
		if (fakez<0) fakez = 0;
		if (z<0) z = 0;
	}
	
	if(drunkclk)
	{
		--drunkclk;
	}
	
	if(lstunclock > 0)
	{
		// also cancel Hero's attack
		attackclk = 0;
		
		if( FFCore.getHeroAction() != stunned )
		{
			tempaction=action; //update so future checks won't do this
			//action=freeze; //setting this makes the player invincible while stunned -V
			FFCore.setHeroAction(stunned);
		}
		--lstunclock;
	}
	//if the stun action is still set in FFCore, but he isn't stunned, then the timer reached 0
	//, so we unfreeze him here, and return him to the action that he had when he was stunned. 
	if ( FFCore.getHeroAction() == stunned && !lstunclock )
	{
		action=tempaction; FFCore.setHeroAction(tempaction);
		//zprint("Unfreezing hero to action: %d\n", (int32_t)tempaction);
		//action=none; FFCore.setHeroAction(none);
	}
	
	if( lbunnyclock > 0 )
	{
		--lbunnyclock;
	}
	if(DMaps[currdmap].flags&dmfBUNNYIFNOPEARL)
	{
		int32_t itemid = current_item_id(itype_pearl);
		if(itemid > -1)
		{
			if(lbunnyclock == -1) //cure dmap-caused bunny effect
				lbunnyclock = 0;
		}
		else if(lbunnyclock > -1) //No pearl, force into bunny mode
		{
			lbunnyclock = -1;
		}
	}
	else if(lbunnyclock == -1) //dmap-caused bunny effect
	{
		lbunnyclock = 0;
	}
	
	if(!is_on_conveyor && !(diagonalMovement||NO_GRIDLOCK) && (fall==0 || fakefall==0 || z>0 || fakez>0) && charging==0 && spins<=5
			&& action != gothit)
	{
		switch(dir)
		{
		case up:
		case down:
			x = TRUNCATE_HALF_TILE(x.getInt() + 4);
			break;
			
		case left:
		case right:
			y = TRUNCATE_HALF_TILE(y.getInt() + 4);
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
			
			for (auto it : clock_zoras)
			{
				int screen_index = it.first;
				int id = it.second;
				addenemy(screen_index,0,0,id,0);
			}
		}
	}
	
	if(hookshot_frozen || switch_hooked)
	{
		if(hookshot_used || switch_hooked)
		{
			if (IsSideSwim()) {action=sideswimfreeze; FFCore.setHeroAction(sideswimfreeze);} 
			else {action=freeze; FFCore.setHeroAction(freeze);} //could be LA_HOOKSHOT for FFCore. -Z
			
			if(pull_hero || switch_hooked)
			{
				if(hs_switcher || switch_hooked)
				{
					hs_fix = false;
					if(switchhookclk)
					{
						--switchhookclk;
						if(switchhookclk==switchhookmaxtime/2) //Perform swaps
						{
							if(switchhook_cost_item > -1 && !checkmagiccost(switchhook_cost_item))
								reset_hookshot();
							else
							{
								weapon *w = (weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)),
									*hw = (weapon*)Lwpns.spr(Lwpns.idFirst(wHSHandle));
								
								if(hooked_comborpos != rpos_t::None) //Switching combos
								{
									rpos_t targrpos = hooked_comborpos, plrpos = COMBOPOS_REGION(x+8,y+8);
									int32_t target_pos = RPOS_TO_POS(targrpos);
									int32_t player_pos = RPOS_TO_POS(plrpos);

									if (targrpos <= region_max_rpos && plrpos <= region_max_rpos && hooked_layerbits)
									{
										int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
										for(int q = max_layer; q > -1; --q)
										{
											if(!(hooked_layerbits & (1<<q)))
												continue; //non-switching layer
											
											auto target_pos_handle = get_rpos_handle(targrpos, q);
											auto player_pos_handle = get_rpos_handle(plrpos, q);
											
											mapscr* player_scr = player_pos_handle.screen;
											mapscr* target_scr = target_pos_handle.screen;

											newcombo const& cmb = combobuf[target_scr->data[target_pos]];
											int32_t srcfl = target_scr->sflag[target_pos];
											newcombo const& comb2 = combobuf[player_scr->data[player_pos]];
											int32_t c = player_scr->data[player_pos],
													cs = player_scr->cset[player_pos],
													fl = player_scr->sflag[player_pos];
											//{Check push status
											bool isFakePush = false;
											if(cmb.type == cSWITCHHOOK)
											{
												if(cmb.usrflags&cflag7) //counts as 'pushblock'
													isFakePush = true;
											}
											bool isPush = isFakePush;
											if(!isPush) switch(srcfl)
											{
												case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
												case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
												case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
												case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
												case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
												case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
												case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
													isPush = true;
											}
											if(!isPush) switch(cmb.flag)
											{
												case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
												case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
												case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
												case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
												case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
												case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
												case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
													isPush = true;
											}
											if(srcfl==mfPUSHED) isPush = false;
											//}
											if(cmb.type == cSWITCHHOOK) //custom flags and such
											{
												if(cmb.usrflags&cflag3) //Breaks on swap
												{
													int32_t it = -1;
													int32_t thedropset = -1;
													if(cmb.usrflags&cflag4) //drop item
													{
														if(cmb.usrflags&cflag5)
															it = cmb.attribytes[2];
														else
														{
															it = select_dropitem(cmb.attribytes[2]); 
															thedropset = cmb.attribytes[2];
														}
													}
													
													breakable* br = new breakable(x, y, 0_zf,
														cmb, target_scr->cset[target_pos], it, thedropset, cmb.attribytes[2],
														cmb.attribytes[1] ? -1 : 0, cmb.attribytes[1], switchhookclk);
													br->switch_hooked = true;
													decorations.add(br);
													hooked_layerbits &= ~(0x101<<q); //this swap completed entirely
													hooked_undercombos[q] = -1;
													
													if(cmb.usrflags&cflag6)
													{
														target_scr->data[target_pos]++;
													}
													else
													{
														target_scr->data[target_pos] = target_scr->undercombo;
														target_scr->cset[target_pos] = target_scr->undercset;
														if(cmb.usrflags&cflag2)
															target_scr->sflag[target_pos] = 0;
													}
												}
												else if(isPush)
												{
													//Simulate a block clicking into place
													movingblock mtemp;
													mtemp.clear();

													int mx, my;
													COMBOXY_REGION(plrpos, mx, my);

													mtemp.set(mx,my,target_scr->data[target_pos],target_scr->cset[target_pos],q,target_scr->sflag[target_pos]);
													mtemp.dir = getPushDir(target_scr->sflag[target_pos]);
													if(mtemp.dir < 0)
														mtemp.dir = getPushDir(cmb.flag);
													mtemp.clk = 1;
													mtemp.force_many = isFakePush;
													mtemp.no_icy = true;
													mtemp.animate(0);
													if((mtemp.bhole || mtemp.trigger)
														&& (fl == mfBLOCKTRIGGER || fl == mfBLOCKHOLE
															|| comb2.flag == mfBLOCKTRIGGER
															|| comb2.flag == mfBLOCKHOLE))
													{
														target_scr->data[target_pos] = target_scr->undercombo;
														target_scr->cset[target_pos] = target_scr->undercset;
														target_scr->sflag[target_pos] = 0;
													}
													else
													{
														target_scr->data[target_pos] =  c;
														target_scr->cset[target_pos] =  cs;
														if(cmb.usrflags&cflag2)
															target_scr->sflag[target_pos] = fl;
														else
															target_scr->sflag[target_pos] = 0;
													}
												}
												else
												{
													player_scr->data[player_pos] = target_scr->data[target_pos];
													player_scr->cset[player_pos] = target_scr->cset[target_pos];
													if(cmb.usrflags&cflag2)
														player_scr->sflag[player_pos] = target_scr->sflag[target_pos];
													target_scr->data[target_pos] =  c;
													target_scr->cset[target_pos] =  cs;
													if(cmb.usrflags&cflag2)
														target_scr->sflag[target_pos] = fl;
												}
											}
											else if(isCuttableType(cmb.type)) //Break and drop effects
											{
												int32_t breakcs = target_scr->cset[target_pos];
												if(isCuttableNextType(cmb.type)) //next instead of undercmb
												{
													target_scr->data[target_pos]++;
												}
												else
												{
													target_scr->data[target_pos] = target_scr->undercombo;
													target_scr->cset[target_pos] = target_scr->undercset;
													target_scr->sflag[target_pos] = 0;
												}
												
												int32_t it = -1;
												int32_t thedropset = -1;
												if(isCuttableItemType(cmb.type)) //Drop an item
												{
													if ( (cmb.usrflags&cflag2) )
													{
														if(cmb.usrflags&cflag11)
															it = cmb.attribytes[1];
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
												}
												
												byte breaksfx = 0;
												if(get_qr(qr_MORESOUNDS)) //SFX
												{
													if (cmb.usrflags&cflag3)
													{
														breaksfx = cmb.attribytes[2];
													}
													else if(isBushType(cmb.type)
														|| isFlowersType(cmb.type)
													|| isGrassType(cmb.type))
													{
														breaksfx = QMisc.miscsfx[sfxBUSHGRASS];
													}
												}
												
												//Clipping sprite
												int16_t decotype = (cmb.usrflags & cflag1) ?
													((cmb.usrflags & cflag10)
														? (cmb.attribytes[0])
														: (-1))
													: (0);
												if(decotype > 3) decotype = 0;
												if(!decotype)
													decotype = (isBushType(cmb.type) ? 1
														: (isFlowersType(cmb.type) ? 2
														: (isGrassType(cmb.type) ? 3
														: ((cmb.usrflags & cflag1) ? -1
														: -2))));
												
												breakable* br = new breakable(x, y, 0_zf,
													cmb, breakcs, it, thedropset, breaksfx,
													decotype, cmb.attribytes[0], switchhookclk);
												br->switch_hooked = true;
												decorations.add(br);
												hooked_layerbits &= ~(0x101<<q); //this swap completed entirely
												hooked_undercombos[q] = -1;
											}
											else //Unknown type, just swap combos.
											{
												if(isPush)
												{
													//Simulate a block clicking into place
													movingblock mtemp;
													mtemp.clear();

													int mx, my;
													COMBOXY_REGION(plrpos, mx, my);

													mtemp.set(mx,my,target_scr->data[target_pos],target_scr->cset[target_pos],q,target_scr->sflag[target_pos]);
													mtemp.dir = getPushDir(target_scr->sflag[target_pos]);
													if(mtemp.dir < 0)
														mtemp.dir = getPushDir(cmb.flag);
													mtemp.clk = 1;
													mtemp.animate(0);
													if(mtemp.bhole || mtemp.trigger)
													{
														target_scr->data[target_pos] = target_scr->undercombo;
														target_scr->cset[target_pos] = target_scr->undercset;
														target_scr->sflag[target_pos] = 0;
													}
													else
													{
														target_scr->data[target_pos] =  c;
														target_scr->cset[target_pos] =  cs;
														target_scr->sflag[target_pos] = 0;
													}
												}
												else
												{
													player_scr->data[player_pos] = target_scr->data[target_pos];
													player_scr->cset[player_pos] = target_scr->cset[target_pos];
													target_scr->data[target_pos] = c;
													target_scr->cset[target_pos] = cs;
												}
											}
										}
										if(switchhook_cost_item > -1)
											paymagiccost(switchhook_cost_item);
										zfix tx = x, ty = y;
										//Position the player at the combo

										{
											int hx, hy;
											COMBOXY_REGION(targrpos, hx, hy);
											x = hx;
											y = hy;
										}

										dir = oppositeDir[dir];
										if(w && hw)
										{
											//Calculate chain shift
											zfix dx = (x-tx);
											zfix dy = (y-ty);
											if(w->dir < 4)
											{
												if(w->dir & 2)
													dx = 0;
												else dy = 0;
											}
											//Position the hook head at the handle
											w->x = hw->x + dx;
											w->y = hw->y + dy;
											w->dir = oppositeDir[w->dir];
											w->doAutoRotate(true);
											byte hflip = (w->dir > 3 ? 3 : ((w->dir & 2) ? 1 : 2));
											w->flip ^= hflip;
											//Position the handle appropriately
											hw->x = x-(hw->x-tx);
											hw->y = y-(hw->y-ty);
											hw->dir = oppositeDir[hw->dir];
											hw->doAutoRotate(true);
											hw->flip ^= hflip;
											//Move chains
											for(int32_t j=0; j<chainlinks.Count(); j++)
											{
												chainlinks.spr(j)->x += dx;
												chainlinks.spr(j)->y += dy;
											}
										}
										hooked_comborpos = plrpos; //flip positions
									}
									else reset_hookshot();
								}
								else if(switching_object) //Switching an object
								{
									if(switchhook_cost_item > -1)
										paymagiccost(switchhook_cost_item);
									zfix tx = x, ty = y;
									//Position the player at the object
									x = switching_object->x;
									y = switching_object->y;
									dir = oppositeDir[dir];
									//Position the object at the player
									switching_object->x = tx;
									switching_object->y = ty;
									if(switching_object->dir == dir || switching_object->dir == oppositeDir[dir])
										switching_object->dir = oppositeDir[switching_object->dir];
									solid_update(false);
									switching_object->solid_update(false);
									if(item* it = dynamic_cast<item*>(switching_object))
									{
										if(itemsbuf[it->id].family == itype_fairy && itemsbuf[it->id].misc3)
										{
											movefairynew2(it->x, it->y, *it);
										}
									}
									if(w && hw) //!TODO No fucking clue if diagonals work
									{
										//Calculate chain shift
										zfix dx = (x-tx);
										zfix dy = (y-ty);
										if(w->dir < 4)
										{
											if(w->dir & 2)
												dx = 0;
											else dy = 0;
										}
										//Position the hook head at the handle
										w->x = hw->x + dx;
										w->y = hw->y + dy;
										w->dir = oppositeDir[w->dir];
										w->doAutoRotate(true);
										byte hflip = (w->dir > 3 ? 3 : ((w->dir & 2) ? 1 : 2));
										w->flip ^= hflip;
										w->solid_update(false);
										//Position the handle appropriately
										hw->x = x-(hw->x-tx);
										hw->y = y-(hw->y-ty);
										hw->dir = oppositeDir[hw->dir];
										hw->doAutoRotate(true);
										hw->flip ^= hflip;
										hw->solid_update(false);
										//Move chains
										for(int32_t j=0; j<chainlinks.Count(); j++)
										{
											chainlinks.spr(j)->x += dx;
											chainlinks.spr(j)->y += dy;
										}
									}
								}
							}
						}
						else if(!switchhookclk)
						{
							reset_hookshot();
						}
					}
					else reset_hookshot();
				}
				else
				{
					sprite *t;
					int32_t i;
					
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
							if (abs((s->y) - y) >= 1)
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
							}
							else 
							{
								y = (s->y);
							}
							if (abs((s->x) - x) >= 1)
							{
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
							else 
							{
								x = (s->x);
							}
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
			if(dir==up || dir==down)
			{
				y = TRUNCATE_TILE(int32_t(y+7));
			}
			
			if(dir==left || dir==right)
			{
				x = TRUNCATE_TILE(int32_t(x+7));
			}
			
			hs_fix=false;
		}
		
	}
	
	if(!get_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP))
	{
		if(DrunkrLbtn())
			selectNextBWpn(SEL_LEFT);
		else if(DrunkrRbtn())
			selectNextBWpn(SEL_RIGHT);
	}
	if (get_qr(qr_SELECTAWPN) && get_qr(qr_USE_EX1_EX2_INVENTORYSWAP))
	{
		if (rEx3btn())
			selectNextAWpn(SEL_LEFT);
		else if (rEx4btn())
			selectNextAWpn(SEL_RIGHT);
	}
		
	if(rPbtn())
	{
		if( !FFCore.runOnMapScriptEngine() ) //OnMap script replaces the 'onViewMap()' call
			onViewMap();
	}   
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon *w = ((weapon*)Lwpns.spr(i));
		
		if(w->id == wArrow || w->id == wBrang || w->id == wCByrna)
			addsparkle(i);
	}
	
	if(Lwpns.idCount(wPhantom))
	{
		addsparkle2(pDIVINEFIREROCKET,pDIVINEFIREROCKETTRAIL);
		addsparkle2(pDIVINEFIREROCKETRETURN,pDIVINEFIREROCKETTRAILRETURN);
		addsparkle2(pDIVINEPROTECTIONROCKET1,pDIVINEPROTECTIONROCKETTRAIL1);
		addsparkle2(pDIVINEPROTECTIONROCKET2,pDIVINEPROTECTIONROCKETTRAIL2);
		addsparkle2(pDIVINEPROTECTIONROCKETRETURN1,pDIVINEPROTECTIONROCKETTRAILRETURN1);
		addsparkle2(pDIVINEPROTECTIONROCKETRETURN2,pDIVINEPROTECTIONROCKETTRAILRETURN2);
	}
	
	// Pay magic cost for Byrna beams
	
	//Byrna needs a secondary timer, for 254+, as do all items that reduce MP on a per-frae basis. Essentially, we will do % divisor == 0 for that. -Z
	if(Lwpns.idCount(wCByrna))
	{
		weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wCByrna)));
		int32_t itemid = ew->parentitem;
		
		if(!(checkbunny(itemid) && checkmagiccost(itemid)))
		{
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				weapon *w = ((weapon*)Lwpns.spr(i));
				
				if(w->id==wCByrna)
				{
					w->dead=1;
				}
		
			}
			//kill the sound effect for the orbits -Z 14FEB2019
			stop_sfx(itemsbuf[itemid].usesound);
		}
		else paymagiccost(itemid);
	}
	
	if(z==0&&fakez==0)
	{
		switchblock_z = 0;
		if(switchblock_offset)
		{
			switchblock_offset=false;
			yofs += 8;
		}
	}
	if(!isSideViewHero())
	{
		int32_t tx = x.getInt()+8,
		    ty = y.getInt()+8;//(bigHitbox?8:12);
		if (unsigned(ty) < world_h && unsigned(tx) < world_w)
		{
			for(int32_t q = 0; q < 3; ++q)
			{
				mapscr* s = get_layer_scr_for_xy(tx, ty, q - 1);
				if (q && !s->valid) continue;

				newcombo const& cmb = combobuf[s->data[COMBOPOS(tx%256,ty%176)]];
				if(cmb.type != cCSWITCHBLOCK || !(cmb.usrflags&cflag9)) continue;
				int32_t b = 1;
				if(tx&8) b <<= 2;
				if(ty&8) b <<= 1;
				b |= (b<<4); //check equivalent effect flag too
				if((cmb.walk&b)==b) //solid and effecting
				{
					if(z==0&&fakez==0)
					{
						if(cmb.usrflags&cflag10)
						{
							if(!switchblock_offset)
							{
								switchblock_offset=true;
								yofs -= 8;
							}
						}
						else
						{
							if(switchblock_offset)
							{
								switchblock_offset=false;
								yofs += 8;
							}
						}
					}
					if(cmb.attributes[2]>0 && switchblock_z>=0)
					{
						if(z==0&&fakez==0)
							switchblock_z = zc_max(switchblock_z,zslongToFix(cmb.attributes[2]));
						else if(SWITCHBLOCK_STATE < zslongToFix(cmb.attributes[2]))
						{
							switchblock_z += zslongToFix(cmb.attributes[2])-SWITCHBLOCK_STATE;
						}
					}
					else switchblock_z = -1;
					break;
				}
			}
		}
	}
	ClearhitHeroUIDs(); //clear them before we advance. 
	checkhit();
	
	bool forcedeath = dying_flags&DYING_FORCED;
	bool norev = (dying_flags&DYING_NOREV);
	if(forcedeath || (game->get_life()<=0 && !immortal))
	{
		if(forcedeath)
			game->set_life(0);
		if(!norev)
			for(size_t slot = 0; slot < 256; ++slot)
			{
				if(size_t bind = game->get_bottle_slot(slot))
				{
					bottletype const* bt = &QMisc.bottle_types[bind-1];
					if(!(bt->flags&BTFLAG_AUTOONDEATH))
						continue;
					word toFill[3] = { 0 };
					for(size_t q = 0; q < 3; ++q)
					{
						char c = bt->counter[q];
						if(c > -1)
						{
							if(bt->flags & (1<<q))
							{
								toFill[q] = (bt->amount[q]==100)
									? game->get_maxcounter(c)
									: word((game->get_maxcounter(c)/100.0)*bt->amount[q]);
							}
							else toFill[q] = bt->amount[q];
							if(toFill[q] + game->get_counter(c) > game->get_maxcounter(c))
							{
								toFill[q] = game->get_maxcounter(c) - game->get_counter(c);
							}
						}
					}
					if(bt->flags & BTFLAG_CURESWJINX)
					{
						swordclk = 0;
						verifyAWpn();
					}
					if(bt->flags & BTFLAG_CUREITJINX)
						itemclk = 0;
					if(word max = std::max(toFill[0], std::max(toFill[1], toFill[2])))
					{
						int32_t itemid = find_bottle_for_slot(slot,true);
						stop_sfx(QMisc.miscsfx[sfxLOWHEART]); //stop heart beep!
						if(itemid > -1)
							sfx(itemsbuf[itemid].usesound,pan(x.getInt()));
						for(size_t q = 0; q < 20; ++q)
							do_death_refill_waitframe();
						double inc = max/60.0; //1 second
						double xtra[3]{ 0 };
						for(size_t q = 0; q < 60; ++q)
						{
							if(!(q%6) && (toFill[0]||toFill[1]||toFill[2]))
								sfx(QMisc.miscsfx[sfxREFILL]);
							for(size_t j = 0; j < 3; ++j)
							{
								xtra[j] += inc;
								word f = floor(xtra[j]);
								xtra[j] -= f;
								if(toFill[j] > f)
								{
									toFill[j] -= f;
									game->change_counter(f,bt->counter[j]);
								}
								else if(toFill[j])
								{
									game->change_counter(toFill[j],bt->counter[j]);
									toFill[j] = 0;
								}
							}
							do_death_refill_waitframe();
						}
						for(size_t j = 0; j < 3; ++j)
						{
							if(toFill[j])
							{
								game->change_counter(toFill[j],bt->counter[j]);
								toFill[j] = 0;
							}
						}
						for(size_t q = 0; q < 20; ++q)
							do_death_refill_waitframe();
					}
					game->set_bottle_slot(slot,bt->next_type);
					if(game->get_life() > 0)
					{
						dying_flags = 0;
						forcedeath = false;
						break; //Revived! Stop drinking things...
					}
				}
			}
		
		if ( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			if(forcedeath || (game->get_life()<=0 && !immortal)) //Not saved by fairy
			{
				// So scripts can have one frame to handle hp zero events
				if(norev || false == (last_hurrah = !last_hurrah))
				{
					dying_flags = 0;
					drunkclk=0;
					lstunclock = 0;
					is_conveyor_stunned = 0;
					FFCore.setHeroAction(dying);
					FFCore.deallocateAllScriptOwned(ScriptType::Global, GLOBAL_SCRIPT_GAME);
					FFCore.deallocateAllScriptOwned(ScriptType::Player, SCRIPT_PLAYER_ACTIVE);
					ALLOFF(true,true);
					GameFlags |= GAMEFLAG_NO_F6;
					if(!debug_enabled)
					{
						Paused=false;
					}
					if(!get_qr(qr_ONDEATH_RUNS_AFTER_DEATH_ANIM))
					{
						FFCore.runOnDeathEngine();
						FFCore.deallocateAllScriptOwned(ScriptType::Player, SCRIPT_PLAYER_DEATH);
					}
					Playing = false;
					heroDeathAnimation();
					if(get_qr(qr_ONDEATH_RUNS_AFTER_DEATH_ANIM))
					{
						Playing = true;
						FFCore.runOnDeathEngine();
						FFCore.deallocateAllScriptOwned(ScriptType::Player, SCRIPT_PLAYER_DEATH);
						Playing = false;
					}
					GameFlags &= ~GAMEFLAG_NO_F6;
					ALLOFF(true,true);
					return true;
				}
			}
		}
		else //2.50.x
		{
			// So scripts can have one frame to handle hp zero events
			if(false == (last_hurrah = !last_hurrah))
			{
				drunkclk=0;
				heroDeathAnimation();
				
				return true;
			}
		}
	}
	else last_hurrah=false;
	
	if(swordclk>0)
	{
		--swordclk;
		if(!swordclk) verifyAWpn();
	}
	if(itemclk>0)
		--itemclk;
	if(shieldjinxclk>0)
		--shieldjinxclk;
		
	if(inwallm)
	{
		attackclk=0;
		herostep();
		
		if(CarryHero()==false)
			restart_level();
		
		solid_update(false);
		return false;
	}
	
	if(ewind_restart)
	{
		attackclk=0;
		restart_level();
		xofs=0;
		action=none; FFCore.setHeroAction(none);
		ewind_restart=false;
		solid_update(false);
		return false;
	}
	
	if(hopclk)
	{
		action=hopping; FFCore.setHeroAction(hopping);
	}
	if(fallclk)
	{
		action=falling; FFCore.setHeroAction(falling);
	}
	
	handle_passive_buttons();
	if(liftclk)
	{
		if(lift_wpn)
		{
			action=lifting; FFCore.setHeroAction(lifting);
		}
		else
		{
			liftclk = 0;
			tliftclk = 0;
		}
	}
	else if(lift_wpn)
	{
		handle_lift(false);
	}
	
		
	// get user input or do other animation
	freeze_guys=false;										// reset this flag, set it again if holding
	
	if(action != landhold1 && action != landhold2 && action != waterhold1 && action != waterhold2 && action != sidewaterhold1 && action != sidewaterhold2 && fairyclk==0 && holdclk>0)
	{
		holdclk=0;
	}
	
	active_shield_id = refreshActiveShield();
	bool sh = active_shield_id > -1;
	itemdata const& shield = itemsbuf[active_shield_id];
	//Handle direction forcing. This runs every frame so that scripts can interact with dir still.
	shield_forcedir = -1;
	if(sh && action != rafting && (shield.flags & ITEM_FLAG11)) //Lock Dir
	{
		shield_forcedir = dir;
	}
	if(sh != shield_active) //Toggle active shield on/off
	{
		shield_active = sh;
		if(sh) //Toggle active shield on
		{
			sfx(shield.usesound2); //'Activate' sfx
		}
	}
	
	bool isthissolid = false;
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
	case lavadrowning:
	case sidedrowning:
	{
		herostep(); // maybe this line should be elsewhere?
		
		//!DROWN
		// Helpful comment to find drowning -Dimi
		
		drop_liftwpn();
		if(--drownclk==0)
		{
			action=none; FFCore.setHeroAction(none);
			int32_t water = drownCombo ? drownCombo : iswaterex_z3(MAPCOMBO(x.getInt()+7.5,y.getInt()+12), currmap, currscr, -1, x.getInt()+7.5,y.getInt()+12, true, false);
			
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(water*10000);
			
			throwGenScriptEvent(GENSCR_EVENT_PLAYER_DROWN);
			water = ev[0]/10000;
			newcombo const& watercmb = combobuf[water];
			
			int32_t damage = 4;
			if (watercmb.type == cWATER)
				damage = watercmb.attributes[0]/10000L;
			else water = 0;
			
			if(cheat_superman && damage > 0)
				damage = 0;
			if(damage)
				game->set_life(vbound(game->get_life()-damage,0, game->get_maxlife()));
			drownCombo = 0;
			go_respawn_point();
			hclk=48;
		}
		
		break;
	}
	case falling:
	{
		herostep();
		pitfall();
		break;
	}
	case freeze:
	case sideswimfreeze:
	case scrolling:
		break;
		
	case casting:
	case sideswimcasting:
	{
		if(magicitem==-1)
		{
			action=none; FFCore.setHeroAction(none);
		}
		
		break;
	}
	case landhold1:
	case landhold2:
	{
		if(--holdclk <= 0)
		{
			//restart music
			if(get_qr(qr_HOLDNOSTOPMUSIC) == 0 && (specialcave < GUYCAVE))
				playLevelMusic();
				
			action=none; FFCore.setHeroAction(none);
			post_item_collect();
		}
		else
			freeze_guys=true;
			
		break;
	}
	case waterhold1:
	case waterhold2:
	case sidewaterhold1:
	case sidewaterhold2:
	{
		diveclk=0;
		
		if(--holdclk <= 0)
		{
			//restart music
			if(get_qr(qr_HOLDNOSTOPMUSIC) == 0  && (specialcave < GUYCAVE))
				playLevelMusic();
				
			SetSwim();
			post_item_collect();
		}
		else
			freeze_guys=true;
			
		break;
	}
	case hopping:
	{
		if(DRIEDLAKE)
		{
			action=none; FFCore.setHeroAction(none);
			hopclk = 0;
			diveclk = 0;
			break;
		}
		
		do_hopping();
		break;
	}
	case inwind:
	{
		int32_t i=Lwpns.idFirst(wWind);
		
		if(i<0)
		{
			bool exit=false;
			
			if(whirlwind==255)
			{
				exit=true;
			}
			else if(y<=0 && dir==up) y=-1;
			else if(y>=world_h-16 && dir==down) y=world_h-16+1;
			else if(x<=0 && dir==left) x=-1;
			else if(x>=world_w-16 && dir==right) x=world_w-16+1;
			else exit=true;
			
			if(exit)
			{
				action=none; FFCore.setHeroAction(none);
				xofs=0;
				whirlwind=0;
				lstep=0;
				if ( dontdraw < 2 ) dontdraw=0;
				set_respawn_point();
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
	case lifting:
		handle_lift();
		break;
	
	case sideswimming:
	case sideswimattacking:
	case sideswimhit:
	case swimhit:
	case swimming:
	{	
		if(DRIEDLAKE)
		{
			action=none; FFCore.setHeroAction(none);
			hopclk=0;
			break;
		}
		
		bool shouldbreak = (action == sideswimhit || action == swimhit); //!DIMITODO: "Can walk while hurt" compat needs to be added here.
		
		if((frame&1) && !shouldbreak)
			herostep();
		
		if (_walkflag(x+7,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
                || _walkflag(x+7,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE)
		|| _walkflag(x+8,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
                || _walkflag(x+8,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE)) isthissolid = true;
		if ((get_qr(qr_NO_HOPPING) || CanSideSwim()) && !isthissolid) //Since hopping won't be set with this on, something needs to kick Hero out of water...
		{
			if(!iswaterex_z3(MAPCOMBO(x.getInt()+4,y.getInt()+9), -1, x.getInt()+4,y.getInt()+9, true, false)||!iswaterex_z3(MAPCOMBO(x.getInt()+4,y.getInt()+15), -1, x.getInt()+4,y.getInt()+15, true, false)
			|| !iswaterex_z3(MAPCOMBO(x.getInt()+11,y.getInt()+9), -1, x.getInt()+11,y.getInt()+9, true, false)||!iswaterex_z3(MAPCOMBO(x.getInt()+11,y.getInt()+15), -1, x.getInt()+11,y.getInt()+15, true, false))
			{
				hopclk=0;
				diveclk=0;
				if (action != sideswimattacking && action != attacking) {action=none; FFCore.setHeroAction(none);}
				else {action=attacking; FFCore.setHeroAction(attacking);}
				hopdir=-1;
			}
		}
		if (shouldbreak) break;
		if (action == swimming || action == sideswimming || action == sideswimattacking)
		{
			int32_t watercheck = iswaterex_z3(MAPCOMBO(x.getInt()+7.5,y.getInt()+12), -1, x.getInt()+7.5,y.getInt()+12, true, false);
			if (combobuf[watercheck].usrflags&cflag2)
			{
				if (current_item(combobuf[watercheck].attribytes[2]) < combobuf[watercheck].attribytes[3])
				{
					onpassivedmg = true;
					if (damageovertimeclk == 0)
					{
						int32_t curhp = game->get_life();
						if (combobuf[watercheck].usrflags&cflag5) game->set_life(vbound(game->get_life()+ringpower(combobuf[watercheck].attributes[1]/10000L), 0, game->get_maxlife())); //Affected by rings
						else game->set_life(vbound(game->get_life()+(combobuf[watercheck].attributes[1]/10000L), 0, game->get_maxlife()));
						if ((combobuf[watercheck].attributes[2]/10000L) && (game->get_life() != curhp || !(combobuf[watercheck].usrflags&cflag6))) sfx(combobuf[watercheck].attributes[2]/10000L);
						if (game->get_life() < curhp && combobuf[watercheck].usrflags&cflag7)
						{
							hclk = 48;
							hitdir = -1;
							if (IsSideSwim()) {action = sideswimhit; FFCore.setHeroAction(sideswimhit);}
							else {action = swimhit; FFCore.setHeroAction(swimhit);}
						}
					}
					if (combobuf[watercheck].attribytes[1] > 0)
					{
						if (!damageovertimeclk || damageovertimeclk > combobuf[watercheck].attribytes[1]) damageovertimeclk = combobuf[watercheck].attribytes[1];
						else --damageovertimeclk;
					}
					else damageovertimeclk = 0;
				}
				else damageovertimeclk = 0;
			}
			else damageovertimeclk = 0;
			//combobuf[watercheck].attributes[0]
		}

	}
	[[fallthrough]];
	default:
		// call the main movement routine
		if(get_qr(qr_NEW_HERO_MOVEMENT2))
		{
			if(premove())
				movehero();
		}
		else moveheroOld();
	}
	
	if(shield_forcedir > -1 && action != rafting)
		dir = shield_forcedir;
	
	
	// check for ladder removal
	if(diagonalMovement)
	{
		if(ladderx+laddery)
		{
			if(ladderdir==up)
			{
				if((laddery-y.getInt()>=(16+(ladderstart==dir?ladderstart==down?1:0:0))) || (laddery-y.getInt()<=(-16-(ladderstart==dir?ladderstart==up?1:0:0))) || (abs(ladderx-x.getInt())>8))
				{
					reset_ladder();
				}
			}
			else
			{
				if((abs(laddery-y.getInt())>8) || (ladderx-x.getInt()>=(16+(ladderstart==dir?ladderstart==right?1:0:0))) || (ladderx-x.getInt()<=(-16-(ladderstart==dir?ladderstart==left?1:0:0))))
				{
					reset_ladder();
				}
			}
		}
	}
	else
	{
		if((abs(laddery-y.getInt())>=16) || (abs(ladderx-x.getInt())>=16))
		{
			reset_ladder();
		}
	}
	
	if(!get_qr(qr_OLD_RESPAWN_POINTS)) //needs to happen after ladder removal so the respawn point is set when laddering over multiple 1 tile gaps.
		set_respawn_point(false); //Keep the 'last safe location' updated!
	
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
	
	if(frame - newscr_clk >= 2)
	{
		loadenemies();
	}
	
	// check lots of other things
	checkscroll();
	
	if(action!=inwind && action!=drowning && action != sidedrowning && action!=lavadrowning)
	{
		checkspecial();
		checkitems();
		checklocked(); //This has issues if Hero's action is WALKING, in 8-way moveent. 
		if(get_qr(qr_OLD_LOCKBLOCK_COLLISION))
		{
			oldchecklockblock();
			oldcheckbosslockblock();
		}
		if(get_qr(qr_OLD_CHEST_COLLISION))
		{
			oldcheckchest(cCHEST);
			oldcheckchest(cLOCKEDCHEST);
			oldcheckchest(cBOSSCHEST);
		}
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
	
	// Somehow Hero was displaced from the fairy flag...
	if(fairyclk && action != freeze && action != sideswimfreeze)
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
			
			int32_t index2 = 0;
			
			if(tmpscr->flags5 & fRANDOMTIMEDWARP) index2=zc_oldrand()%4;
			
			sdir = dir;
			dowarp(1,index2);
		}
	}
	
	bool awarp = false;
	//!DIMI: Global Combo Effects (AUTO STUFF)
	for(auto& p : slopes)
	{
		slope_object& s = p.second;
		s.updateslope(); //sets old x/y poses
	}
	for(int32_t i=0; i<176; ++i)
	{
		for(int32_t layer=0; layer<7; ++layer)
		{
			int32_t cid = ( layer ) ? MAPCOMBOL(layer,COMBOX(i),COMBOY(i)) : MAPCOMBO(COMBOX(i),COMBOY(i));
			newcombo const& cmb = combobuf[cid];
			
			if(!get_qr(qr_AUTOCOMBO_ANY_LAYER))
			{
				if(layer > 2) break;
				if (layer == 1 && !get_qr(qr_AUTOCOMBO_LAYER_1)) continue;
				if (layer == 2 && !get_qr(qr_AUTOCOMBO_LAYER_2)) continue;
			}
			int32_t ind=0;
			
			//AUTOMATIC TRIGGER CODE
			if (cmb.triggerflags[1]&combotriggerAUTOMATIC)
			{
				// TODO z3
				do_trigger_combo(layer, i);
			}
			
			//AUTO WARP CODE
			if(!(cmb.triggerflags[0] & combotriggerONLYGENTRIG))
			{
				if(cmb.type==cAWARPA)
				{
					awarp=true;
					ind=0;
				}
				else if(cmb.type==cAWARPB)
				{
					awarp=true;
					ind=1;
				}
				else if(cmb.type==cAWARPC)
				{
					awarp=true;
					ind=2;
				}
				else if(cmb.type==cAWARPD)
				{
					awarp=true;
					ind=3;
				}
				else if(cmb.type==cAWARPR)
				{
					awarp=true;
					ind=zc_oldrand()%4;
				}
			}
			if(awarp)
			{
				// TODO z3
				if(tmpscr->flags5&fDIRECTAWARP)
				{
					didpit=true;
					pitx=x;
					pity=y;
				}
				
				sdir = dir;
				dowarp(1,ind);
				break;
			}
		}
		if(awarp) break;
	}
	
	awarp=false;
	
	for_some_ffcs_in_region([&](const ffc_handle_t& ffc_handle) {
		int32_t ind=0;
		
		newcombo const& cmb = combobuf[ffc_handle.data()];
		
		if (cmb.triggerflags[1]&combotriggerAUTOMATIC)
		{
			do_trigger_combo_ffc(ffc_handle);
		}
		
		if(!(cmb.triggerflags[0] & combotriggerONLYGENTRIG))
		{
			if(cmb.type==cAWARPA)
			{
				awarp=true;
				ind=0;
			}
			else if(cmb.type==cAWARPB)
			{
				awarp=true;
				ind=1;
			}
			else if(cmb.type==cAWARPC)
			{
				awarp=true;
				ind=2;
			}
			else if(cmb.type==cAWARPD)
			{
				awarp=true;
				ind=3;
			}
			else if(cmb.type==cAWARPR)
			{
				awarp=true;
				ind=zc_oldrand()%4;
			}
		}
		
		if(awarp)
		{
			if(ffc_handle.screen->flags5&fDIRECTAWARP)
			{
				didpit=true;
				pitx=x;
				pity=y;
			}
			
			sdir = dir;
			// TODO z3 warp
			dowarp(1,ind);
			return false;
		}

		return true;
	});

	zfix dx, dy;
	if (sideview_mode() && !on_sideview_solid_oldpos(x, y,old_x,old_y, false, 1) && on_sideview_solid_oldpos(x, y,old_x,old_y, false, 2) && !toogam)
	{
		if (slide_slope(this, dx, dy, slopeid))
		{
			onplatid = 5;
			if (dx || dy) push_move(dx, dy);
		}
	}
	if (onplatid <= 0) slopeid = 0;
	else --onplatid;
	bool onplatform = (on_sideview_solid_oldpos(x, y,old_x,old_y, false, 1) && !Up());
	for (auto q = 0; (check_slope(this, true) && !toogam) && q < 2; ++q)
	{
		if (check_slope(this, true) && !toogam)
		{
			slope_info const& s = get_slope(this, true).get_info();
			bool staircheck = false;
			if (s.slope() != slopeid && slopeid) staircheck = true;
			if (onplatform) staircheck = true;
			slope_push_int(s, this, dx, dy, staircheck, platformfell2);
			
			if (dx || dy) 
			{
				int32_t pushret = push_move(dx,dy);
				onplatform = (on_sideview_solid_oldpos(x, y,old_x,old_y, false, 1) && !Up());
				if (s.slope() != slopeid && slopeid) staircheck = true;
				if (onplatform) staircheck = true;
				if(sideview_mode() && slopeid)
					onplatid = 5;
				if (pushret == 1)
				{
					dx = -1;
					dy = 0;
					slope_push_int(s, this, dx, dy, staircheck);
					push_move(dx,dy);
				}
				if (pushret == 2)
				{
					dx = 0;
					dy = -1;
					slope_push_int(s, this, dx, dy, staircheck);
					push_move(dx,dy);
				}
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
	
	//Hero->WarpEx
	if ( FFCore.warpex[wexActive] )
	{
		if(DEVLOGGING) zprint("Running warpex from hero.cpp\n");
		FFCore.warpex[wexActive] = 0;
		int32_t temp_warpex[wexActive] = {0}; //to hold the values as we clear the FFCore array. -Z
		for ( int32_t q = 0; q < wexActive; q++ ) 
		{
			temp_warpex[q] = FFCore.warpex[q];
			FFCore.warpex[q] = 0;
		}
		FFCore.warp_player( temp_warpex[wexType], temp_warpex[wexDMap], temp_warpex[wexScreen], temp_warpex[wexX],
			temp_warpex[wexY], temp_warpex[wexEffect], temp_warpex[wexSound], temp_warpex[wexFlags], temp_warpex[wexDir]); 
	}
	
	if(z == 0 || !get_qr(qr_NO_SCROLL_WHILE_IN_AIR))
	{
		// walk through bombed doors and fake walls
		bool walk=false;
		int32_t dtype=dBOMBED;
		
		if(pushing>=24) dtype=dWALK;
		
		if(isdungeon() && action!=freeze && action != sideswimfreeze && loaded_guys && !inlikelike && !diveclk && action!=rafting && !lstunclock && !is_conveyor_stunned)
		{
			if(((dtype==dBOMBED)?DrunkUp():dir==up) && ((diagonalMovement||NO_GRIDLOCK)?x.getInt()>112&&x.getInt()<128:x.getInt()==120) && y<=32 && tmpscr->door[0]==dtype)
			{
				walk=true;
				dir=up;
			}
			
			if(((dtype==dBOMBED)?DrunkDown():dir==down) && ((diagonalMovement||NO_GRIDLOCK)?x.getInt()>112&&x.getInt()<128:x.getInt()==120) && y>=128 && tmpscr->door[1]==dtype)
			{
				walk=true;
				dir=down;
			}
			
			if(((dtype==dBOMBED)?DrunkLeft():dir==left) && x<=32 && ((diagonalMovement||NO_GRIDLOCK)?y.getInt()>72&&y.getInt()<88:y.getInt()==80) && tmpscr->door[2]==dtype)
			{
				walk=true;
				dir=left;
			}
			
			if(((dtype==dBOMBED)?DrunkRight():dir==right) && x>=208 && ((diagonalMovement||NO_GRIDLOCK)?y.getInt()>72&&y.getInt()<88:y.getInt()==80) && tmpscr->door[3]==dtype)
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
				if(!get_qr(qr_WALKTHROUGHWALL_NO_DOORSTATE))
				{
					auto si = (currmap<<7) + currscr;
					auto di = nextscr(dir);
					setmapflag(si, mDOOR_UP<<dir);
					if(di != 0xFFFF)
						setmapflag(di, mDOOR_UP<<oppositeDir[dir]);
				}
			}
			
			action=none; FFCore.setHeroAction(none);
			attackclk = 0;
			stepforward(29, true);
			action=scrolling; FFCore.setHeroAction(scrolling);
			pushing=false;
		}
	}
	
	if( game->get_life() <= (game->get_hp_per_heart()) && !(game->get_maxlife() <= (game->get_hp_per_heart())) && (heart_beep_timer > -3))
	{
		if(heart_beep)
		{
			cont_sfx(QMisc.miscsfx[sfxLOWHEART]);
		}
		else
		{
			if ( heart_beep_timer == -1 )
			{
				heart_beep_timer = 70;
			}
			
			if ( heart_beep_timer > 0 )
			{
				--heart_beep_timer;
				cont_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
			else
			{
				stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
			}
		}
	}
	else 
	{
	if ( heart_beep_timer > -2 )
	{
		heart_beep_timer = -1;
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
	}
	}
	
	if(rSbtn())
	{
		int32_t tmp_subscr_clk = frame;
		
		int32_t save_type = 0;
		switch(lsave)
		{
		case 0:
		{
			if( FFCore.runScriptedActiveSusbcreen() )
			{
				break;
			}
			else if ( !stopSubscreenFalling() )
			{
				conveyclk=3;
				dosubscr();
				newscr_clk += frame - tmp_subscr_clk;
			}
			break;
		}
		
		case 2:
			save_type = 1;
			[[fallthrough]];
		case 1:
			if(last_savepoint_id)
				trigger_save(combobuf[last_savepoint_id], hero_screen);
			else save_game((tmpscr->flags4&fSAVEROOM) != 0, save_type); //sanity? 
			break;
		}
	}
	
	if (!checkstab() )
	{
	}
	
	check_conveyor();
	PhantomsCleanup();
	//Try to time the hammer pound so that Hero can;t change direction while it occurs. 
	if(attack==wHammer)
	{
		if(attackclk==12 && z==0 && fakez==0 && sideviewhammerpound())
		{
			switch(dir) //Hero's dir
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
	}
	
	handleSpotlights();
	
	if(getOnSideviewLadder())
	{
		if(!canSideviewLadder() || jumping<0 || fall!=0 || fakefall!=0)
		{
			setOnSideviewLadder(false);
		}
		else if(CANFORCEFACEUP)
		{
			setDir(up);
		}
	}
	if (justmoved > 0) --justmoved;
	
	return false;
}

bool HeroClass::push_pixel(zfix dx, zfix dy)
{
	ASSERT(abs(dx) <= 1 && abs(dy) <= 1);
	if(dx && dy)
	{
		bool r = push_pixel(0,dy);
		bool r2 = push_pixel(dx,0);
		return r && r2;
	}
	if(dx < 0)
	{
		if(!(solpush_walkflag(x+dx,y+(bigHitbox?0:8),1,this)
			|| solpush_walkflag(x+dx,y+8,1,this)
			|| (y.getInt()&7?solpush_walkflag(x+dx,y+16,1,this):0)))
		{
			x += dx;
			return true;
		}
		return false;
	}
	else if(dx > 0)
	{
		if(!(solpush_walkflag(x+15+dx,y+(bigHitbox?0:8),1,this)
			|| solpush_walkflag(x+15+dx,y+8,1,this)
			|| (y.getInt()&7?solpush_walkflag(x+15+dx,y+16,1,this):0)))
		{
			x += dx;
			return true;
		}
		return false;
	}
	else if(dy < 0)
	{
		if(!(solpush_walkflag(x,y+(bigHitbox?0:8)+dy,2,this)
			|| (x.getInt()&7?solpush_walkflag(x+16,y+(bigHitbox?0:8)+dy,1,this):0)))
		{
			y += dy;
			return true;
		}
		return false;
	}
	else if(dy > 0)
	{
		if(!(solpush_walkflag(x,y+15+dy,2,this)
			|| (x.getInt()&7?solpush_walkflag(x+16,y+15+dy,1,this):0)))
		{
			y += dy;
			return true;
		}
		return false;
	}
	return false;
}
int32_t HeroClass::push_move(zfix dx, zfix dy)
{
	int32_t ret = 0;
	while(dx || dy)
	{
		if(check_pitslide() != -1)
			break;
		if(dy)
		{
			zfix cy = (abs(dy) >= 1) ? sign(dy) : dy;
			dy -= cy;
			if(!push_pixel(0,cy))
			{
				dy = 0;
				ret |= 2;
			}
		}
		if(dx)
		{
			zfix cx = (abs(dx) >= 1) ? sign(dx) : dx;
			dx -= cx;
			if(!push_pixel(cx,0))
			{
				dx = 0;
				ret |= 1;
			}
		}
	}
	return ret;
}

bool HeroClass::setSolid(bool set)
{
	bool actual = set && !toogam; //not solid when noclipping
	bool ret = solid_object::setSolid(actual);
	solid = set;
	return ret;
}

void HeroClass::solid_push(solid_object* obj)
{
	if(obj == this) return; //can't push self
	
	zfix dx, dy;
	int32_t hdir = -1;
	solid_push_int(obj,dx,dy,hdir);
	
	if(!dx && !dy) return;
	
	obj->doContactDamage(hdir);
	
	bool t = obj->getTempNonsolid();
	obj->setTempNonsolid(true);
	
	push_move(dx,dy);
	
	obj->setTempNonsolid(t);
}

#define COND_AWPN (get_qr(qr_SELECTAWPN) ? game->awpn : 255)
#define COND_BWPN (game->bwpn)
#define COND_XWPN (get_qr(qr_SET_XBUTTON_ITEMS) ? game->xwpn : 255)
#define COND_YWPN (get_qr(qr_SET_YBUTTON_ITEMS) ? game->ywpn : 255)
//Helper function
static void deselectbombsWPN(word& wpos, int32_t& BTNwpn, int32_t& directItemBTN,
	word f1 = 255, word f2 = 255, word f3 = 255)
{
	byte pgn = wpos&0xFF, pos = wpos>>8;
	bool empty = pgn==255;
	if(empty && get_qr(qr_NO_BUTTON_VERIFY)) return; //intentional nothingness
	SubscrPage* pg = new_subscreen_active->get_page(pgn==255?new_subscreen_active->curpage:pgn);
	if(!pg)
	{
		wpos = 255; //set to nothingness
		return;
	}
	
	auto fp1 = ((f1&0xFF)==255) ? 255 : ((empty || (f1&0xFF)==(wpos&0xFF)) ? f1 : 255);
	auto fp2 = ((f2&0xFF)==255) ? 255 : ((empty || (f2&0xFF)==(wpos&0xFF)) ? f2 : 255);
	auto fp3 = ((f3&0xFF)==255) ? 255 : ((empty || (f3&0xFF)==(wpos&0xFF)) ? f3 : 255);
	auto temp = pg->movepos_legacy(SEL_VERIFY_LEFT, wpos, fp1, fp2, fp3);
	if((temp&0xFF) == 0xFF)
		BTNwpn = -1;
	else BTNwpn = pg->get_item_pos(temp>>8);
	directItemBTN = NEG_OR_MASK(BTNwpn,0xFFF);
	wpos = temp;
}
// A routine used exclusively by startwpn,
// to switch Hero's weapon if his current weapon (bombs) was depleted.
void HeroClass::deselectbombs(int32_t super)
{
    if ( get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN) || itemsbuf[game->forced_awpn].family == itype_bomb || itemsbuf[game->forced_bwpn].family == itype_bomb || itemsbuf[game->forced_xwpn].family == itype_bomb || itemsbuf[game->forced_ywpn].family == itype_bomb) return;
    if(getItemFamily(itemsbuf,Bwpn)==(super? itype_sbomb : itype_bomb) && (directWpn<0 || Bwpn==directWpn))
    {
		if(!new_subscreen_active)
			return;
		deselectbombsWPN(game->bwpn, Bwpn, directItemB, COND_AWPN, COND_XWPN, COND_YWPN);
    }
    else if (getItemFamily(itemsbuf,Xwpn)==(super? itype_sbomb : itype_bomb) && (directWpn<0 || Xwpn==directWpn))
    {
		if(!new_subscreen_active)
			return;
        deselectbombsWPN(game->xwpn, Xwpn, directItemX, COND_AWPN, COND_BWPN, COND_YWPN);
    }
    else if (getItemFamily(itemsbuf,Ywpn)==(super? itype_sbomb : itype_bomb) && (directWpn<0 || Ywpn==directWpn))
    {
		if(!new_subscreen_active)
			return;
        deselectbombsWPN(game->ywpn, Ywpn, directItemY, COND_AWPN, COND_XWPN, COND_BWPN);
    }
    else if (getItemFamily(itemsbuf,Awpn)==(super? itype_sbomb : itype_bomb) && (directWpn<0 || Awpn==directWpn))
    {
		if(!new_subscreen_active)
			return;
        deselectbombsWPN(game->awpn, Awpn, directItemA, COND_BWPN, COND_XWPN, COND_YWPN);
    }
}

int32_t potion_life=0;
int32_t potion_magic=0;

bool HeroClass::onWater(bool drownonly)
{
	int32_t water = 0;
	int32_t types[4] = {0};
	int32_t x1 = x+4, x2 = x+11,
		y1 = y+9, y2 = y+15;
	if (get_qr(qr_SMARTER_WATER))
	{
		if (iswaterex_z3(0, -1, x1, y1, true, false) &&
			iswaterex_z3(0, -1, x1, y2, true, false) &&
			iswaterex_z3(0, -1, x2, y1, true, false) &&
			iswaterex_z3(0, -1, x2, y2, true, false)) water = iswaterex_z3(0, -1, (x2+x1)/2,(y2+y1)/2, true, false);
	}
	else
	{
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
			
		int32_t typec = COMBOTYPE((x2+x1)/2,(y2+y1)/2);
		if(MAPFFCOMBO((x2+x1)/2,(y2+y1)/2))
			typec = FFCOMBOTYPE((x2+x1)/2,(y2+y1)/2);
			
		if(combo_class_buf[types[0]].water && combo_class_buf[types[1]].water &&
				combo_class_buf[types[2]].water && combo_class_buf[types[3]].water && combo_class_buf[typec].water)
			water = typec;
	}
	if(water > 0)
	{
		if(!drownonly) return true;
		if(current_item(itype_flippers) <= 0 || current_item(itype_flippers) < combobuf[water].attribytes[0] || ((combobuf[water].usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))) 
		{
			return true;
		}
	}
	return false;
}

bool HeroClass::mirrorBonk()
{
	zfix tx = x, ty = y, tz = z;
	WalkflagInfo info = walkflag(x,y+(bigHitbox?0:8),2,up);
	info = info || walkflagMBlock(x+8,y+(bigHitbox?0:8));
	execute(info);
	bool fail = info.isUnwalkable();
	
	if(!fail) //not solid, but check for water/pits...
	{
		if(onWater(true))
			fail = true;
		if(pitslide() || fallclk)
			fail = true;
		fallclk = 0;
	}
	x = tx; y = ty; z = tz;
	return fail;
}

void HeroClass::doMirror(int32_t mirrorid)
{
	if(z > 0 || fakez > 0) return; //No mirror in air
	if(mirrorid < 0)
		mirrorid = current_item_id(itype_mirror);
	if(mirrorid < 0) return;
	
	if((tmpscr->flags9&fDISABLE_MIRROR) || !(checkbunny(mirrorid) && checkmagiccost(mirrorid)))
	{
		item_error();
		return;
	}
	static const int32_t sens = 4; //sensitivity of 'No Mirror' combos (0 most, 8 least)
	rpos_t rposes[] = {COMBOPOS_REGION(x+sens,y+sens), COMBOPOS_REGION(x+sens,y+15-sens),
		COMBOPOS_REGION(x+15-sens,y+sens), COMBOPOS_REGION(x+15-sens,y+15-sens)};
	for(auto rpos : rposes)
	{
		if(HASFLAG_ANY(mfNOMIRROR, rpos)) //"No Mirror" flag touching the player
		{
			item_error();
			return;
		}
	}
	
	itemdata const& mirror = itemsbuf[mirrorid];
	if(DMaps[currdmap].flags & dmfMIRRORCONTINUE)
	{
		paymagiccost(mirrorid);
		if(mirror.usesound2) sfx(mirror.usesound2);
		
		doWarpEffect(mirror.misc2, true);
		if(mirror.flags & ITEM_FLAG2) //Act as F6->Continue
		{
			Quit = qCONT;
			skipcont = 1;
		}
		else //Act as Divine Escape
		{
            int32_t div_prot_temp=div_prot_item;
            restart_level();
            div_prot_item=div_prot_temp;
            magicitem=-1;
            magiccastclk=0;
            if ( Hero.getDontDraw() < 2 ) { Hero.setDontDraw(0); }
		}
	}
	else
	{
		int32_t destdmap = DMaps[currdmap].mirrorDMap;
		int32_t offscr = currscr - DMaps[currdmap].xoff;
		if(destdmap < 0)
			return;
		int32_t destscr = DMaps[destdmap].xoff + offscr;
		if((offscr%16 != destscr%16) || unsigned(destscr) >= 0x80)
			return;
		paymagiccost(mirrorid);
		
		//Store some values to restore if 'warp fails'
		int32_t tLastEntrance = lastentrance,
				tLastEntranceDMap = lastentrance_dmap,
				tContScr = game->get_continue_scrn(),
				tContDMap = game->get_continue_dmap(),
				tPortalDMap = game->saved_mirror_portal.srcdmap;
		int32_t sourcescr = currscr, sourcedmap = currdmap;
		zfix tx = x, ty = y, tz = z;
		game->saved_mirror_portal.srcdmap = -1;
		action = none; FFCore.setHeroAction(none);
		
		//Warp to new dmap
		FFCore.warp_player(wtIWARP, destdmap, destscr, -1, -1, mirror.misc1,
			mirror.usesound, 0, -1);
		
		//Check for valid landing location
		if(mirrorBonk()) //Invalid landing, warp back!
		{
			action = none; FFCore.setHeroAction(none);
			lastentrance = tLastEntrance;
			lastentrance_dmap = tLastEntranceDMap;
			game->set_continue_scrn(tContScr);
			game->set_continue_dmap(tContDMap);
			x = tx;
			y = ty;
			z = tz;
			game->saved_mirror_portal.srcdmap = tPortalDMap;
			FFCore.warp_player(wtIWARP, sourcedmap, sourcescr, -1, -1, mirror.misc1,
				mirror.usesound, 0, -1);
		}
		else if(mirror.flags & ITEM_FLAG1) //Place portal!
		{
			//Place the portal
			game->set_portal(sourcedmap, destdmap, offscr, x.getZLong(), y.getZLong(), mirror.usesound, mirror.misc1, mirror.wpn);
			//Since it was placed after loading this screen, load the portal object now
			game->load_portal();
			//Don't immediately trigger the warp back
			mirror_portal.prox_active = false;
			
			//Set continue point
			if(currdmap != game->get_continue_dmap())
			{
				game->set_continue_scrn(DMaps[currdmap].cont + DMaps[currdmap].xoff);
			}
			game->set_continue_dmap(currdmap);
			lastentrance_dmap = currdmap;
			lastentrance = game->get_continue_scrn();
		}
	}
}

void HeroClass::handle_passive_buttons()
{
	do_liftglove(-1,true);
	do_jump(-1,true);
}

static bool did_passive_jump = false;
bool HeroClass::do_jump(int32_t jumpid, bool passive)
{
	if(passive) did_passive_jump = false;
	else if(did_passive_jump) return false; //don't jump twice in the same frame
	
	if(nomove_action(action)) return false; //can't jump while ex. drowning
	if(onWater(true)) return false; //Don't allow jumping off of water frame-perfectly...
	
	if(jumpid < 0)
		jumpid = current_item_id(itype_rocs,true,true);
	
	if(unsigned(jumpid) >= MAXITEMS) return false;
	if(inlikelike || charging) return false;
	if(!checkitem_jinx(jumpid)) return false;
	
	itemdata const& itm = itemsbuf[jumpid];
	
	bool standing = isStanding(true);
	bool coyotejump = !standing && coyotetime < (zc_min(65535,itm.misc5));
	if(!(coyotejump || standing || extra_jump_count < itm.misc1)) return false;
	if(!(checkbunny(jumpid) && checkmagiccost(jumpid)))
	{
		item_error();
		return false;
	}
	
	byte intbtn = byte(itm.misc2&0xFF);
	if(passive)
	{
		if(!getIntBtnInput(intbtn, true, true, false, false, true))
			return false; //not pressed
	}
	
	paymagiccost(jumpid);
	
	if(!standing)
	{
		++extra_jump_count;
		fall = 0;
		fakefall = 0;
		if(hoverclk > 0)
			hoverclk = -hoverclk;
	}
	if(itm.flags & ITEM_FLAG1)
		setFall(fall - itm.power);
	else setFall(fall - (FEATHERJUMP*(itm.power+2)));
	coyotetime = 65535; //jumped, so no coyotetime
	setOnSideviewLadder(false);
	
	// Reset the ladder, unless on an unwalkable combo
	if((ladderx || laddery) && !(_walkflag(ladderx,laddery,0,SWITCHBLOCK_STATE)))
		reset_ladder();
	
	if(itm.usesound)
		sfx(itm.usesound,pan(x.getInt()));
	
	if(passive)
	{
		did_passive_jump = true;
		getIntBtnInput(intbtn, true, true, false, false, false); //eat buttons
	}
	return true;
}
void HeroClass::drop_liftwpn()
{
	if(!lift_wpn) return;
	
	handle_lift(false); //sets position properly, accounting for large weapons
	auto liftid = current_item_id(itype_liftglove,true,true);
	itemdata const& glove = itemsbuf[liftid];
	if(isSideViewGravity())
	{
		lift_wpn->moveflags |= FLAG_NO_FAKE_Z;
	}
	else
	{
		auto lheight = liftheight+z+fakez;
		if(glove.flags & ITEM_FLAG1)
		{
			lift_wpn->z = 0;
			lift_wpn->fakez = lheight;
			lift_wpn->moveflags |= FLAG_NO_REAL_Z;
		}
		else
		{
			lift_wpn->z = lheight;
			lift_wpn->moveflags |= FLAG_NO_FAKE_Z;
		}
	}
	lift_wpn->dir = dir;
	lift_wpn->step = 0;
	lift_wpn->fakefall = 0;
	lift_wpn->fall = 0;
	Lwpns.add(lift_wpn);
	lift_wpn = nullptr;
}
void HeroClass::do_liftglove(int32_t liftid, bool passive)
{
	if(liftid < 0)
		liftid = current_item_id(itype_liftglove,true,true);
	if(!can_lift(liftid)) return;
	itemdata const& glove = itemsbuf[liftid];
	byte intbtn = byte(glove.misc1&0xFF);
	if(passive)
	{
		if(!getIntBtnInput(intbtn, true, true, false, false, true))
			return; //not pressed
	}
	
	bool had_weapon = lift_wpn;
	
	if(!(had_weapon || //Allow throwing while bunnied/don't charge magic for throwing
		(checkbunny(liftid) && checkmagiccost(liftid))))
	{
		item_error();
		return;
	}
	if(glove.script!=0 && (FFCore.doscript(ScriptType::Item, liftid) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
		return;
	
	bool paidmagic = had_weapon; //don't pay to throw, only to lift
	if(glove.script)
	{
		if(!paidmagic)
		{
			paidmagic = true;
			paymagiccost(liftid);
		}
		
		int i = liftid;
		FFCore.reset_script_engine_data(ScriptType::Item, i);
		ZScriptVersion::RunScript(ScriptType::Item, glove.script, i);
		FFCore.deallocateAllScriptOwned(ScriptType::Item,i);
		
		bool has_weapon = lift_wpn;
		if(has_weapon != had_weapon) //Item action script changed the lift information
		{
			if(passive)
			{
				getIntBtnInput(intbtn, true, true, false, false, false); //eat buttons
			}
			return;
		}
	}
	
	if(lift_wpn)
	{
		if(!paidmagic)
		{
			paidmagic = true;
			paymagiccost(liftid);
		}
		if(!liftclk)
		{
			//Throw the weapon!
			//hero's direction and position
			handle_lift(false); //sets position properly, accounting for large weapons
			
			lift_wpn->dir = dir;
			//Configured throw speed in both axes
			auto basestep = glove.misc2;
			lift_wpn->step = zfix(basestep)/100;
			
			if(isSideViewGravity())
			{
				lift_wpn->fall = -glove.misc3;
				switch(dir)
				{
					case left: case right:
						break; //nothing special for sideways
					case up: //step converts to upwards fall
						lift_wpn->fall -= basestep;
						lift_wpn->step = 0;
						break;
					case down: //step converts into straight down fall
						lift_wpn->fall = zc_min(basestep,zinit.terminalv);
						lift_wpn->step = 0;
						break;
				}
				lift_wpn->moveflags |= FLAG_NO_FAKE_Z;
			}
			else
			{
				auto lheight = liftheight+z+fakez;
				if(glove.flags & ITEM_FLAG1)
				{
					lift_wpn->z = 0;
					lift_wpn->fakez = lheight;
					lift_wpn->fakefall = -glove.misc3;
					lift_wpn->moveflags |= FLAG_NO_REAL_Z;
				}
				else
				{
					lift_wpn->z = lheight;
					lift_wpn->fall = -glove.misc3;
					lift_wpn->moveflags |= FLAG_NO_FAKE_Z;
				}
			}
			Lwpns.add(lift_wpn);
			lift_wpn = nullptr;
			if(glove.usesound2)
				sfx(glove.usesound2,pan(int32_t(x)));
		}
		
		if(passive)
		{
			getIntBtnInput(intbtn, true, true, false, false, false); //eat buttons
		}
		return;
	}
	
	bool lifted = false;
	//Check for a liftable weapon
	//if(!lifted)
	{
		zfix hx, hy, hw, hh;
		switch(dir)
		{
			case up:
				hx = x;
				hy = y-8;
				hw = 16;
				hh = bigHitbox ? 8 : 16;
				break;
			case down:
				hx = x;
				hy = y+16;
				hw = 16;
				hh = 8;
				break;
			case left:
				hx = x-8;
				hy = y;
				hw = 8;
				hh = 16;
				break;
			case right:
				hx = x+16;
				hy = y;
				hw = 8;
				hh = 16;
				break;
		}
		for(int32_t q = 0; q < Lwpns.Count(); ++q)
		{
			weapon* w = (weapon*)Lwpns.spr(q);
			if((w->lift_level && w->lift_level <= glove.fam_type))
			{
				if(!w->hit(hx,hy,0,hw,hh,1))
					continue;
				lift(w, w->lift_time, w->lift_height);
				Lwpns.remove(w);
				lifted = true;
				break;
			}
		}
	}
	if(!lifted) //Check for a liftable combo
	{
		zfix bx, by;
		zfix bx2, by2;
		switch(dir)
		{
			case up:
				by = y + (bigHitbox ? -2 : 6);
				by2 = by;
				bx = x + 4;
				bx2 = bx + 8;
				break;
			case down:
				by = y + 17;
				by2 = by;
				bx = x + 4;
				bx2 = bx + 8;
				break;
			case left:
				by = y + (bigHitbox ? 0 : 8);
				by2 = y + 8;
				bx = x - 2;
				bx2 = x - 2;
				break;
			case right:
				by = y + (bigHitbox ? 0 : 8);
				by2 = y + 8;
				bx = x + 17;
				bx2 = x + 17;
				break;
		}
		rpos_t rpos = COMBOPOS_REGION_CHECK_BOUNDS(bx, by);
		rpos_t rpos2 = COMBOPOS_REGION_CHECK_BOUNDS(bx2, by2);
		
		for(auto lyr = 6; lyr >= 0; --lyr)
		{
			if(rpos != rpos_t::None)
			{
				auto rpos_handle = get_rpos_handle(rpos, lyr);
				newcombo const& cmb = combobuf[rpos_handle.data()];
				if(cmb.liftflags & LF_LIFTABLE)
				{
					if(do_lift_combo(rpos_handle,liftid))
					{
						lifted = true;
						break;
					}
				}
			}
			if(rpos != rpos2 && rpos2 != rpos_t::None)
			{
				auto rpos_handle_2 = get_rpos_handle(rpos2, lyr);
				newcombo const& cmb2 = combobuf[rpos_handle_2.data()];
				if(cmb2.liftflags & LF_LIFTABLE)
				{
					if(do_lift_combo(rpos_handle_2,liftid))
					{
						lifted = true;
						break;
					}
				}
			}
		}
	}
	if(!lifted) return;
	if(!paidmagic)
	{
		paidmagic = true;
		paymagiccost(liftid);
	}
	set_liftflags(liftid);
	if(passive)
		getIntBtnInput(intbtn, true, true, false, false, false); //eat buttons
	return;
}
void HeroClass::handle_lift(bool dec)
{
	if(lift_wpn)
		lift_wpn->fakez = 0;
	else liftclk = 0;
	if(liftclk <= (dec?1:0))
	{
		liftclk = 0;
		tliftclk = 0;
		if(lift_wpn)
		{
			if(lift_wpn->txsz > 1 || lift_wpn->tysz > 1)
			{
				lift_wpn->x = x+8 - (lift_wpn->txsz*8);
				lift_wpn->y = y+8 - (lift_wpn->tysz*8);
			}
			else
			{
				lift_wpn->x = x;
				lift_wpn->y = y;
			}
			if(isSideViewGravity())
				lift_wpn->y -= liftheight;
			else lift_wpn->z = liftheight;
		}
		if(action == lifting)
		{
			action = none; FFCore.setHeroAction(none);
		}
		return;
	}
	if(dec) --liftclk;
	double xdist, ydist;
	double perc = (liftclk/double(tliftclk));
	switch(dir)
	{
		case up:
		{
			xdist = 0;
			ydist = -16;
			if(lift_wpn->txsz > 1)
			{
				xdist = -((lift_wpn->txsz*8)-8);
			}
			else xdist = 0;
			if(lift_wpn->tysz > 1)
			{
				ydist = -(lift_wpn->tysz*16);
			}
			ydist *= perc;
			break;
		}
		case down:
		{
			xdist = 0;
			ydist = 16;
			if(lift_wpn->txsz > 1)
			{
				xdist = -((lift_wpn->txsz*8)-8);
			}
			else xdist = 0;
			ydist *= perc;
			break;
		}
		case left:
		{
			xdist = -16;
			ydist = 0;
			if(lift_wpn->txsz > 1)
			{
				xdist = -(lift_wpn->txsz*16);
			}
			if(lift_wpn->tysz > 1)
			{
				ydist = -((lift_wpn->tysz*8)-8);
			}
			else ydist = 0;
			xdist *= perc;
			break;
		}
		case right:
		{
			xdist = 16;
			ydist = 0;
			if(lift_wpn->tysz > 1)
			{
				ydist = -((lift_wpn->tysz*8)-8);
			}
			else ydist = 0;
			xdist *= perc;
			break;
		}
	}
	
	lift_wpn->x = x + xdist;
	lift_wpn->y = y + ydist;
	if(isSideViewGravity())
		lift_wpn->y -= liftheight*(1.0-perc);
	else lift_wpn->z = liftheight*(1.0-perc);
}
bool HeroClass::can_lift(int32_t gloveid)
{
	if(unsigned(gloveid) >= MAXITEMS) return false;
	if(lstunclock) return false;
	if(!checkitem_jinx(gloveid)) return false;
	itemdata const& glove = itemsbuf[gloveid];
	switch(action)
	{
		case none: case walking:
			break;
		
		case swimming:
			if(glove.flags & ITEM_FLAG2)
				break;
			return false;
		
		default:
			return false;
	}
	return true;
}
void HeroClass::lift(weapon* w, byte timer, zfix height)
{
	lift_wpn = w;
	liftclk = timer;
	tliftclk = timer;
	if(height < 0)
		liftheight = 0;
	else liftheight = height;
}

void HeroClass::doSwitchHook(byte style)
{
	hs_switcher = true;
	pull_hero = true;
	//{ Load hook weapons, set them to obey special drawing
	weapon *w = (weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot)),
		*hw = (weapon*)Lwpns.spr(Lwpns.idFirst(wHSHandle));
		
	if(w)
		w->switch_hooked = true;
	if(hw)
		hw->switch_hooked = true;
	for(int32_t j=0; j<chainlinks.Count(); j++)
	{
		chainlinks.spr(j)->switch_hooked = true;
	}
	//}
	if(hooked_comborpos != rpos_t::None)
	{
		rpos_t plrpos = COMBOPOS_REGION(x+8, y+8);
		int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
		hooked_layerbits = 0;
		for(auto q = 0; q < 7; ++q)
			hooked_undercombos[q] = -1;
		
		byte target_pos = RPOS_TO_POS(hooked_comborpos);
		byte player_pos = RPOS_TO_POS(plrpos);
		
		for(auto q = max_layer; q > -1; --q)
		{
			auto target_pos_handle = get_rpos_handle(hooked_comborpos, q);
			auto player_pos_handle = get_rpos_handle(plrpos, q);
			
			mapscr* player_scr = player_pos_handle.screen;
			mapscr* target_scr = target_pos_handle.screen;

			newcombo const& cmb = combobuf[target_scr->data[target_pos]];
			newcombo const& comb2 = combobuf[player_scr->data[player_pos]];
			int32_t fl1 = target_scr->sflag[target_pos],
					fl2 = player_scr->sflag[player_pos];
			bool isPush = false;
			if(isSwitchHookable(cmb))
			{
				if(cmb.type == cSWITCHHOOK)
				{
					if((cmb.usrflags&cflag1) && player_scr->data[player_pos])
						continue; //don't swap with non-zero combo
					if(zc_max(1,itemsbuf[(w && w->parentitem>-1) ? w->parentitem : current_item_id(itype_switchhook)].fam_type) < cmb.attribytes[0])
						continue; //too low a switchhook level
					hooked_layerbits |= 1<<q; //Swapping
					if(cmb.usrflags&cflag3)
					{
						if(cmb.usrflags&cflag6)
						{
							hooked_undercombos[q] = target_scr->data[target_pos]+1;
							hooked_undercombos[q+7] = target_scr->cset[target_pos];
						}
						else
						{
							hooked_undercombos[q] = target_scr->undercombo;
							hooked_undercombos[q+7] = target_scr->undercset;
						}
					}
					else
					{
						hooked_layerbits |= 1<<(q+8); //Swapping BACK
						if(cmb.usrflags&cflag7) //counts as 'pushblock'
							isPush = true;
					}
				}
				else if(isCuttableType(cmb.type))
				{
					if(isCuttableNextType(cmb.type))
					{
						hooked_undercombos[q] = target_scr->data[target_pos]+1;
						hooked_undercombos[q+7] = target_scr->cset[target_pos];
					}
					else
					{
						hooked_undercombos[q] = target_scr->undercombo;
						hooked_undercombos[q+7] = target_scr->undercset;
					}
					hooked_layerbits |= 1<<q; //Swapping
				}
				else
				{
					hooked_layerbits |= 1<<q; //Swapping
					hooked_layerbits |= 1<<(q+8); //Swapping BACK
				}
			}
			if(hooked_layerbits & (1<<(q+8))) //2-way swap, check for pushblocks
			{
				if((cmb.type==cPUSH_WAIT || cmb.type==cPUSH_HW || cmb.type==cPUSH_HW2)
					&& hasMainGuy())
				{
					hooked_layerbits &= ~(0x101<<q); //Can't swap yet
					continue;
				}
				if(fl1 == mfPUSHED)
				{
					hooked_layerbits &= ~(0x101<<q); //Can't swap at all, locked in place
					continue;
				}
				if(!isPush) switch(fl1)
				{
					case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
					case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
					case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
					case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
					case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
					case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
					case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
						isPush = true;
				}
				if(!isPush) switch(cmb.flag)
				{
					case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
					case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
					case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
					case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
					case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
					case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
					case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
						isPush = true;
				}
				if(isPush) //Check for block holes / triggers
				{
					if(comb2.flag == mfBLOCKHOLE || fl2 == mfBLOCKHOLE
						|| comb2.flag == mfBLOCKTRIGGER || fl2 == mfBLOCKTRIGGER)
					{
						hooked_layerbits &= ~(1<<(q+8)); //Don't swap the hole/trigger back
					}
					else if(!get_qr(qr_BLOCKHOLE_SAME_ONLY))
					{
						auto maxLayer = get_qr(qr_PUSHBLOCK_LAYER_1_2) ? 2 : 0;
						for(auto lyr = 0; lyr < maxLayer; ++lyr)
						{
							if(lyr == q) continue;
							switch(player_scr->sflag[player_pos])
							{
								case mfBLOCKHOLE: case mfBLOCKTRIGGER:
									hooked_layerbits &= ~(1<<(q+8)); //Don't swap the hole/trigger back
									lyr=7;
									break;
							}
							switch(combobuf[player_scr->data[player_pos]].flag)
							{
								case mfBLOCKHOLE: case mfBLOCKTRIGGER:
									hooked_layerbits &= ~(1<<(q+8)); //Don't swap the hole/trigger back
									lyr=7;
									break;
							}
						}
					}
				}
			}
		}
	}
	switch_hooked = true;
	switchhookstyle = style;
	switch(style)
	{
		default: case swPOOF:
		{
			wpndata const& spr = wpnsbuf[QMisc.sprites[sprSWITCHPOOF]];
			switchhookmaxtime = switchhookclk = zc_max(spr.frames,1) * zc_max(spr.speed,1);
			decorations.add(new comboSprite(x, y, dCOMBOSPRITE, 0, QMisc.sprites[sprSWITCHPOOF]));
			if(hooked_comborpos != rpos_t::None)
			{
				int decx, decy;
				COMBOXY_REGION(hooked_comborpos, decx, decy);
				decorations.add(new comboSprite(decx, decy, dCOMBOSPRITE, 0, QMisc.sprites[sprSWITCHPOOF]));
			}
			else if(switching_object)
				decorations.add(new comboSprite(switching_object->x, switching_object->y, dCOMBOSPRITE, 0, QMisc.sprites[sprSWITCHPOOF]));
			break;
		}
		case swFLICKER:
		{
			switchhookmaxtime = switchhookclk = 64;
			break;
		}
		case swRISE:
		{
			switchhookmaxtime = switchhookclk = 64;
			break;
		}
	}
}

bool HeroClass::startwpn(int32_t itemid)
{
	if(itemid < 0) return false;
	itemdata const& itm = itemsbuf[itemid];
	if(((dir==up && y<24) || (dir==down && y>world_h-48) ||
			(dir==left && x<32) || (dir==right && x>world_w-48)) && !(get_qr(qr_ITEMSONEDGES) || inlikelike))
		return false;
	
	bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
	if(liftonly)
	{
		dowpn = -1;
		switch(itm.family)
		{
			case itype_bomb:
			case itype_sbomb:
				if(itm.flags & ITEM_FLAG4)
					do_liftglove(-1,false);
				break;
			case itype_liftglove:
				do_liftglove(-1,false);
				break;
		}
		return false;
	}
	
	int32_t wx=x;
	int32_t wy=y-fakez;
	int32_t wz=z;
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
	if (IsSideSwim() && (itm.flags & ITEM_SIDESWIM_DISABLED)) return false;
	
	switch(itm.family)
	{
		case itype_liftglove:
		{
			do_liftglove(itemid,false);
			dowpn = -1;
			ret = false;
			break;
		}
		case itype_potion:
		{
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			
			if(itm.misc1 || itm.misc2)
			{
				refill_what=REFILL_ALL;
				refill_why=itemid;
				StartRefill(REFILL_ALL);
				potion_life = game->get_life();
				potion_magic = game->get_magic();
				
				//add a quest rule or an item option that lets you specify whether or not to pause music during refilling
				//music_pause();
				stop_sfx(QMisc.miscsfx[sfxLOWHEART]); //stop heart beep!
				while(refill())
				{
					do_refill_waitframe();
				}
				
				//add a quest rule or an item option that lets you specify whether or not to pause music during refilling
				//music_resume();
				ret = false;
			}
			
			break;
		}
		case itype_bottle:
		{
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			if(itm.script!=0 && (FFCore.doscript(ScriptType::Item, itemid) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
				return false;
			
			size_t bind = game->get_bottle_slot(itm.misc1);
			bool paidmagic = false;
			if(itm.script)
			{
				paidmagic = true;
				paymagiccost(itemid);

				int i = itemid;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itm.script, i);
				bind = game->get_bottle_slot(itm.misc1);
			}
			bottletype const* bt = bind ? &(QMisc.bottle_types[bind-1]) : NULL;
			if(bt)
			{
				word toFill[3] = { 0 };
				for(size_t q = 0; q < 3; ++q)
				{
					char c = bt->counter[q];
					if(c > -1)
					{
						if(bt->flags & (1<<q))
						{
							toFill[q] = (bt->amount[q]==100)
								? game->get_maxcounter(c)
								: word((game->get_maxcounter(c)/100.0)*bt->amount[q]);
						}
						else toFill[q] = bt->amount[q];
						if(toFill[q] + game->get_counter(c) > game->get_maxcounter(c))
						{
							toFill[q] = game->get_maxcounter(c) - game->get_counter(c);
						}
					}
				}
				word max = std::max(toFill[0], std::max(toFill[1], toFill[2]));
				bool run = max > 0;
				if(get_qr(qr_NO_BOTTLE_IF_ANY_COUNTER_FULL))
					run = ((bt->counter[0] > -1 && !toFill[0]) || (bt->counter[1] > -1 && !toFill[1]) || (bt->counter[2] > -1 && !toFill[2]));
				else
				{
					if((bt->flags & BTFLAG_CURESWJINX) && swordclk)
						run = true;
					else if((bt->flags & BTFLAG_CUREITJINX) && itemclk)
						run = true;
				}
				if(run || (bt->flags&BTFLAG_ALLOWIFFULL))
				{
					if(bt->flags & BTFLAG_CURESWJINX)
					{
						swordclk = 0;
						verifyAWpn();
					}
					if(bt->flags & BTFLAG_CUREITJINX)
						itemclk = 0;
					if(!paidmagic)
						paymagiccost(itemid);
					stop_sfx(QMisc.miscsfx[sfxLOWHEART]); //stop heart beep!
					sfx(itm.usesound,pan(x.getInt()));
					for(size_t q = 0; q < 20; ++q)
						do_refill_waitframe();
					double inc = max/60.0; //1 second
					double xtra[3]{ 0 };
					for(size_t q = 0; q < 60; ++q)
					{
						if(!(q%6) && (toFill[0]||toFill[1]||toFill[2]))
							sfx(QMisc.miscsfx[sfxREFILL]);
						for(size_t j = 0; j < 3; ++j)
						{
							xtra[j] += inc;
							word f = floor(xtra[j]);
							xtra[j] -= f;
							if(toFill[j] > f)
							{
								toFill[j] -= f;
								game->change_counter(f,bt->counter[j]);
							}
							else if(toFill[j])
							{
								game->change_counter(toFill[j],bt->counter[j]);
								toFill[j] = 0;
							}
						}
						do_refill_waitframe();
					}
					for(size_t j = 0; j < 3; ++j)
					{
						if(toFill[j])
						{
							game->change_counter(toFill[j],bt->counter[j]);
							toFill[j] = 0;
						}
					}
					for(size_t q = 0; q < 20; ++q)
						do_refill_waitframe();
					game->set_bottle_slot(itm.misc1, bt->next_type);
				}
			}
			
			dowpn = -1;
			ret = false;
			break;
		}
		
		case itype_note:
		{
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			if(!msg_active && itm.misc1 > 0 && itm.misc1 < MAXMSGS)
			{
				sfx(itm.usesound);
				donewmsg(itm.misc1);
				paymagiccost(itemid);
			}
			dowpn = -1;
			ret = false;
			break;
		}
		
		case itype_mirror:
			doMirror(itemid);
			if(Quit)
				return false;
			ret = false;
			break;
		
		case itype_rocs:
		{
			if(!do_jump(itemid,false)) return false;
			ret = false;
		}
		break;
		
		case itype_letter:
		{
			if(current_item(itype_letter)==i_letter &&
					(currscr >= 128 ? special_warp_return_screen : *tmpscr).room==rP_SHOP &&
					(currscr >= 128 ? special_warp_return_screen : *tmpscr).guy &&
					((currscr<128&&!(DMaps[currdmap].flags&dmfGUYCAVES))
						||(currscr>=128&&DMaps[currdmap].flags&dmfGUYCAVES)) &&
					checkbunny(itemid)
				)
			{
				int32_t usedid = getItemID(itemsbuf, itype_letter,i_letter+1);
				
				if(usedid != -1)
					getitem(usedid, true, true);
					
				sfx((currscr >= 128 ? special_warp_return_screen : *tmpscr).secretsfx);
				setupscreen();
				action=none; FFCore.setHeroAction(none);
			}
			
			ret = false;
		}
		break;
		
		case itype_whistle:
		{
			bool whistleflag;
			
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			sfx(itm.usesound);
			
			if(dir==up || dir==right)
				++blowcnt;
			else
				--blowcnt;
			
			int sfx_count = 0;
			while ((!replay_is_active() && sfx_allocated(itm.usesound)) || (replay_is_active() && sfx_count < 180))
			{
				sfx_count += 1;
				advanceframe(true);
				
				if(Quit)
					return false;
			}
			
			Lwpns.add(new weapon(x,y-fakez,z,wWhistle,0,0,dir,itemid,getUID(),false,0,1,0));
			
			if((whistleflag=trigger_secrets_if_flag(x,y,mfWHISTLE,get_qr(qr_PERMANENT_WHISTLE_SECRETS))))
				didstuff |= did_whistle;
				
			if((didstuff&did_whistle && itm.flags&ITEM_FLAG1) || currscr>=128)
				return false;
				
			if(itm.flags&ITEM_FLAG1) didstuff |= did_whistle;
			
			// TODO z3
			if((tmpscr->flags&fWHISTLE) || (tmpscr->flags7 & fWHISTLEWATER)
					|| (tmpscr->flags7&fWHISTLEPAL))
			{
				whistleclk=0;                                       // signal to start drying lake or doing other stuff
			}
			else
			{
				int32_t where = itm.misc1;
				
				if(where>right) where=dir^1;
				
				if(((DMaps[currdmap].flags&dmfWHIRLWIND && TriforceCount()) || DMaps[currdmap].flags&dmfWHIRLWINDRET) &&
						itm.misc2 >= 0 && itm.misc2 <= 8 && !whistleflag)
					Lwpns.add(new weapon((zfix)(where==left?240_zf:where==right?0_zf:x),
				(zfix)(where==down?0_zf:where==up?160_zf:y),
				(zfix)0,
				wWind,
				0, //type
				0,
				where,
				itemid,getUID(),false,false,true,0)); //last arg is byte special, used to override type for wWind for now. -Z 18JULY2020
										 
				whistleitem=itemid;
			}
			
			ret = false;
		}
		break;
		
		case itype_bomb:
		{
			//Remote detonation
			if(Lwpns.idCount(wLitBomb) >= zc_max(itm.misc2,1))
			{
				weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitBomb)));
				
				 while(Lwpns.idCount(wLitBomb) && ew->misc == 0)
				{
					//If this ever needs a version check, in the future. -z
					if ( FFCore.getQuestHeaderInfo(vZelda) > 0x250 || ( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) > 31 ) )
					{
						if ( ew->power > 1 ) //Don't reduce 1 to 0. -Z
							ew->power *= 0.5; //Remote bombs were dealing double damage. -Z
					}
					ew->misc=50;
					ew->clk=ew->misc-3;
					ew->id=wBomb;
					ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wLitBomb)));
				}
				
				deselectbombs(false);
				return false;
			}
			
			if((itm.flags & ITEM_FLAG4) && lift_wpn)
			{
				do_liftglove(-1,false); //Throw the already-held weapon
				return false;
			}
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
				
			if(itm.misc1>0) // If not remote bombs
				deselectbombs(false);
				
			if(isdungeon())
			{
				wy=zc_max(wy,16);
			}
			
			weapon* wpn = new weapon((zfix)wx,(zfix)wy,(zfix)wz,wLitBomb,itm.fam_type,
				itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true);
			bool lifted = false;
			if(itm.flags & ITEM_FLAG4)
			{
				auto liftid = current_item_id(itype_liftglove);
				itemdata const& glove = itemsbuf[liftid];
				if(liftid > -1 && (!itm.misc4 || itm.misc4 <= glove.fam_type))
				{
					lift(wpn,itm.misc5,itm.misc6);
					set_liftflags(liftid);
					lifted = true;
				}
			}
			if(!lifted)
			{
				Lwpns.add(wpn);
				sfx(WAV_PLACE,pan(wx));
			}
		}
		break;
		
		case itype_sbomb:
		{
			//Remote detonation
			if(Lwpns.idCount(wLitSBomb) >= zc_max(itm.misc2,1))
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
			
			if((itm.flags & ITEM_FLAG4) && lift_wpn)
			{
				do_liftglove(-1,false); //Throw the already-held weapon
				return false;
			}
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
				
			if(itm.misc1>0) // If not remote bombs
				deselectbombs(true);
			
			weapon* wpn = new weapon((zfix)wx,(zfix)wy,(zfix)wz,wLitSBomb,itm.fam_type,itm.power*game->get_hero_dmgmult(),dir, itemid,getUID(),false,false,true);
			bool lifted = false;
			if(itm.flags & ITEM_FLAG4)
			{
				auto liftid = current_item_id(itype_liftglove);
				itemdata const& glove = itemsbuf[liftid];
				if(liftid > -1 && (!itm.misc4 || itm.misc4 <= glove.fam_type))
				{
					lift(wpn,itm.misc5,itm.misc6);
					set_liftflags(liftid);
					lifted = true;
				}
			}
			if(!lifted)
			{
				Lwpns.add(wpn);
				sfx(WAV_PLACE,pan(wx));
			}
		}
		break;
		
		case itype_wand:
		{
			if(Lwpns.idCount(wMagic))
			{
				misc_internal_hero_flags &= ~LF_PAID_WAND_COST;
				return false;
			}
				
			int32_t bookid = current_item_id(itype_book);
			bool paybook = (bookid>-1 && checkbunny(bookid) && checkmagiccost(bookid));
			
			if(!(itm.flags&ITEM_FLAG1) && !paybook)  //Can the wand shoot without the book?
			{
				misc_internal_hero_flags &= ~LF_PAID_WAND_COST;
				return false;
			}
				
			if(!checkbunny(itemid) || !(misc_internal_hero_flags & LF_PAID_WAND_COST || checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			if(Lwpns.idCount(wBeam))
				Lwpns.del(Lwpns.idFirst(wBeam));
			
			int32_t type, pow;
			if ( get_qr(qr_BROKENBOOKCOST) )
			{
				type = bookid != -1 ? current_item(itype_book) : itm.fam_type;
				pow = (bookid != -1 ? current_item_power(itype_book) : itm.power)*game->get_hero_dmgmult();
			}
			else
			{
				type = (bookid != -1 && paybook) ? current_item(itype_book) : itm.fam_type;
				pow = ((bookid != -1 && paybook) ? current_item_power(itype_book) : itm.power)*game->get_hero_dmgmult();
			}
			for(int32_t i=(spins==1?up:dir); i<=(spins==1 ? right:dir); i++)
				if(dir!=(i^1))
			{
				weapon *magic = new weapon((zfix)wx,(zfix)wy,(zfix)wz,wMagic,type,pow,i, itemid,getUID(),false,false,true);
				if(paybook)
					magic->linkedItem = bookid;
				//magic->dir = this->dir; //Save player dir for special weapons. 
				Lwpns.add(magic);
			}
			if(!(misc_internal_hero_flags & LF_PAID_WAND_COST))
				paymagiccost(itemid);
			else misc_internal_hero_flags &= ~LF_PAID_WAND_COST;
			
			if(paybook)
				paymagiccost(current_item_id(itype_book));
				
			if(bookid != -1)
			{
				if (( itemsbuf[bookid].flags & ITEM_FLAG4 ))
				{
					sfx(itemsbuf[bookid].misc2,pan(wx));
				}
				else 
				{
					sfx(itm.usesound,pan(wx));
				}
			}
			else
				sfx(itm.usesound,pan(wx));
		}
		/*
		//    Fireball Wand
		Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wRefFireball,0,2*game->get_hero_dmgmult(),dir));
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
			if(!(checkbunny(itemid) || !(misc_internal_hero_flags & LF_PAID_SWORD_COST || checkmagiccost(itemid))))
			{
				return item_error();
			}
				
			if((Lwpns.idCount(wBeam) && spins==0)||Lwpns.idCount(wMagic))
			{
				misc_internal_hero_flags &= ~LF_PAID_SWORD_COST;
				return false;
			}
				
			if(!(misc_internal_hero_flags & LF_PAID_SWORD_COST))//If already paid to use sword melee, don't charge again
				paymagiccost(itemid);
			else misc_internal_hero_flags &= ~LF_PAID_SWORD_COST;
			float temppower;
			
			if(itm.flags & ITEM_FLAG2)
			{
				temppower=game->get_hero_dmgmult()*itm.power;
				temppower=temppower*itm.misc2;
				temppower=temppower/100;
			}
			else
			{
				temppower = game->get_hero_dmgmult()*itm.misc2;
			}
			
			//Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBeam,itm.fam_type,int32_t(temppower),dir,itemid,getUID()));
			//Add weapon script to sword beams.
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBeam,itm.fam_type,int32_t(temppower),dir,itemid,getUID(),false,false,true));
			//weapon *w = (weapon*)Lwpns.spr(Lwpns.Count()-1); //the pointer to this beam
			//w->weaponscript = itm.weaponscript;
			//w->canrunscript = 0;
			sfx(WAV_BEAM,pan(wx));
		}
		break;
		
		case itype_candle:
		{
			int32_t countid = itemid;
			if(get_qr(qr_CANDLES_SHARED_LIMIT))
				countid = -itype_candle;
			if(itm.flags&ITEM_FLAG1 && usecounts[countid] >= zc_max(1, itm.misc3))
			{
				return false;
			}
			
			if(Lwpns.idCount(wFire) >= (itm.misc2 < 1 ? 2 : itm.misc2))
			{
				return false;
			}
			
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			
			if(itm.flags&ITEM_FLAG1) ++usecounts[countid];
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wFire,
								 //(itm.fam_type > 1), //To do with combo flags ... Needs to be changed to fix ->Level for wFire
								 (itm.fam_type), //To do with combo flags ... Needs to be changed to fix ->Level for wFire
								 itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
			sfx(itm.usesound,pan(wx));
			attack=wFire;
		}
		break;
		
		case itype_script1: case itype_script2: case itype_script3: case itype_script4: case itype_script5:
		case itype_script6: case itype_script7: case itype_script8: case itype_script9: case itype_script10:
		{
			int32_t wtype = wScript1 + (itm.family-itype_script1);
			if(Lwpns.idCount(wtype))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			if(!get_qr(qr_CUSTOMWEAPON_IGNORE_COST))
				paymagiccost(itemid);
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wtype,itm.fam_type,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
			((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itm.misc1/100;
			sfx(itm.usesound,pan(wx));
		}
		break;
		
		case itype_icerod:
		{
			if(Lwpns.idCount(wIce))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			if(!get_qr(qr_CUSTOMWEAPON_IGNORE_COST))
				paymagiccost(itemid);
		
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wIce,itm.fam_type,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
			((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = itm.misc1/100;
			sfx(itm.usesound,pan(wx));
		}
		break;
		
		case itype_arrow:
		{
			if(Lwpns.idCount(wArrow) >= (itm.misc2 < 1 ? 1 : itm.misc2))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wArrow,itm.fam_type,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
			((weapon*)Lwpns.spr(Lwpns.Count()-1))->step*=(current_item_power(itype_bow)+1)/2;
			sfx(itm.usesound,pan(wx));
		}
		break;
		
		case itype_bait:
		{
			if(Lwpns.idCount(wBait)) //TODO: More than one Bait per screen?
				return false;
			
			if(!checkbunny(itemid))
				return item_error();
			
			// TODO z3
			bool grumble = (tmpscr->room==rGRUMBLE && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)));
			bool checkcost = grumble || !(itm.flags & ITEM_FLAG4);
			bool paycost = grumble || !(itm.flags & (ITEM_FLAG4|ITEM_FLAG5));
			
			if(!grumble && (itm.flags & ITEM_FLAG2))
				return item_error(); //Only usable for grumble rooms
			
			if(checkcost && !checkmagiccost(itemid))
				return item_error();
				
			if(paycost)
				paymagiccost(itemid);
			sfx(itm.usesound,pan(wx));
			
			if(grumble)
			{
				items.add(new item((zfix)wx,(zfix)wy,(zfix)0,itemid,ipDUMMY+ipFADE,0));
				fadeclk=66;
				dismissmsg();
				clear_bitmap(pricesdisplaybuf);
				set_clip_state(pricesdisplaybuf, 1);
				//    putscr(scrollbuf,0,0,tmpscr);
				setmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
				if(!(itm.flags & ITEM_FLAG3)) //"Don't remove when feeding" flag
				{
					removeItemsOfFamily(game,itemsbuf,itype_bait);
					verifyBothWeapons();
				}
				sfx(tmpscr->secretsfx);
				return false;
			}
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBait,0,0,dir,itemid,getUID(),false,false,true));
			break;
		}
			
		case itype_brang:
		{
			if(Lwpns.idCount(wBrang) > itm.misc2)
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			current_item_power(itype_brang);
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBrang,itm.fam_type,(itm.power*game->get_hero_dmgmult()),dir,itemid,getUID(),false,false,true));
		}
		break;
		
		case itype_hookshot:
		case itype_switchhook:
		{
			if(inlikelike || Lwpns.idCount(wHookshot))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			bool sw = itm.family == itype_switchhook;
			
			if(sw && (itm.flags&ITEM_FLAG8))
				switchhook_cost_item = itemid;
			else paymagiccost(itemid);
			
			bool use_hookshot=true;
			bool hit_hs = false, hit_solid = false, insta_switch = false;
			int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
			rpos_t cpos = rpos_t::None;
			ffcdata* ffc = nullptr;
			for(int32_t i=0; i<=max_layer && !hit_hs; ++i)
			{
				if(dir==up)
				{
					if(check_hshot(i,x+2,y-7,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==down)
				{
					if(check_hshot(i,x+12,y+23,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==left)
				{
					if(check_hshot(i,x-7,y+12,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==right)
				{
					if(check_hshot(i,x+23,y+12,sw, &cpos, &ffc))
						hit_hs = true;
				}
				//Diagonal Hookshot (6)
				else if(dir==r_down)
				{
					if(check_hshot(i,x+9,y+13,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==l_down)
				{
					if(check_hshot(i,x+6,y+13,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==r_up)
				{
					if(check_hshot(i,x+9,y+13,sw, &cpos, &ffc))
						hit_hs = true;
				}
				else if(dir==l_up)
				{
					if(check_hshot(i,x+6,y+13,sw, &cpos, &ffc))
						hit_hs = true;
				}
			}
			if(dir==up && _walkflag(x+2,y+4,1,SWITCHBLOCK_STATE) && !ishookshottable(x.getInt(),int32_t(y+4)))
				hit_solid = true;
			if(hit_hs)
			{
				if(sw)
					insta_switch = true; //switch immediately
				else use_hookshot = false; //No hooking against grabbable
			}
			if(hit_solid && !insta_switch)
				use_hookshot = false;
			if(use_hookshot)
			{
				int32_t hookitem = itm.fam_type;
				int32_t hookpower = itm.power;
				byte allow_diagonal = (itm.flags & ITEM_FLAG2) ? 1 : 0; 
			
				if(!Lwpns.has_space())
				{
					Lwpns.del(0);
				}
				
				if(!Lwpns.has_space(2))
				{
					Lwpns.del(0);
				}
				
				switch(dir)
				{
					case up:
					{
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy-4,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx;
						hs_starty=wy-4;
					}
					break;
				
					case down:
					{
						int32_t offset=get_qr(qr_HOOKSHOTDOWNBUG)?4:0;
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy+offset,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx;
						hs_starty=wy;
					}
					break;
				
					case left:
					{
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx-4;
						hs_starty=wy;
					}
					break;
				
					case right:
					{
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx+4;
						hs_starty=wy;
					}
					break;
					//Diagonal Hookshot (7)
					case r_down:
					{
						hookshot_used=true;
						hs_switcher = sw;
						int32_t offset=get_qr(qr_HOOKSHOTDOWNBUG)?4:0;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy+offset,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx+4;
						hs_starty=wy;
					}
					break;
					
					case r_up:
					{
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx+4;
						hs_starty=wy;
					}
					break;
					
					case l_down:
					{
						hookshot_used=true;
						hs_switcher = sw;
						int32_t offset=get_qr(qr_HOOKSHOTDOWNBUG)?4:0;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy+offset,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx+4;
						hs_starty=wy;
					}
					break;
					
					case l_up:
					{
						hookshot_used=true;
						hs_switcher = sw;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.family;
						hs_startx=wx+4;
						hs_starty=wy;
					}
					break;
				}
				hookshot_frozen=true;
			}
			if(insta_switch)
			{
				weapon* w = (weapon*)Lwpns.spr(Lwpns.idFirst(wHookshot));
				if (cpos != rpos_t::None) hooked_comborpos = cpos;
				if (ffc)
				{
					ffc->switch_hooked = true;
					switching_object = ffc;
				}
				w->misc=2;
				w->step=0;
				doSwitchHook(itm.misc5);
				if(itm.usesound2)
					sfx(itm.usesound2,pan(int32_t(x)));
				else if(QMisc.miscsfx[sfxSWITCHED])
					sfx(QMisc.miscsfx[sfxSWITCHED],int32_t(x));
				stop_sfx(itm.usesound);
			}
		}
		break;
			
		case itype_divinefire:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(x,y,old_x,old_y) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			if (IsSideSwim()) {action=sideswimcasting; FFCore.setHeroAction(sideswimcasting);}
			else {action=casting; FFCore.setHeroAction(casting);}
			magicitem=itemid;
			break;
			
		case itype_divineescape:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(x,y,old_x,old_y) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			if (IsSideSwim()) {action=sideswimcasting; FFCore.setHeroAction(sideswimcasting);}
			else {action=casting; FFCore.setHeroAction(casting);}
			magicitem=itemid;
			break;
			
		case itype_divineprotection:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(x,y,old_x,old_y) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			if (IsSideSwim()) {action=sideswimcasting; FFCore.setHeroAction(sideswimcasting);}
			else {action=casting; FFCore.setHeroAction(casting);}
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
			
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				stop_sfx(itm.usesound); //if we can't pay the cost, kill the sound. 
				//last_cane_of_byrna_item_id = -1; //no, we'd do this in a byrna cleanup function. 
				return false;
			}
				
			paymagiccost(itemid);
			last_cane_of_byrna_item_id = itemid; 
			//zprint("itm.misc3: %d\n", itm.misc3);
			for(int32_t i=0; i<itm.misc3; i++)
			{
				//byrna weapons are added here
				//space them apart
				//zprint("Added byrna weapon %d.\n", i);
				//the iterator isn passed to 'type'. weapons.cpp converts thisd to
				//'quantity_iterator' pn construction; and this is used for orbit initial spacing.
				Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wCByrna,i,itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
					//Lwpns.add(new weapon((zfix)wx+cos(2 * PI / (i+1)),(zfix)wy+sin(2 * PI / (i+1)),(zfix)wz,wCByrna,i,itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
				//wx += cos(2 * PI / (itm.misc3-i));
				//wy += sin(2 * PI / (itm.misc3-i));
			}
			if(!(Lwpns.idCount(wCByrna)))
				stop_sfx(itm.usesound); //If we can't create the beams, kill the sound. 
		}
		break;
		
		case itype_clock:
		{
			ret = false;
			if(!(itm.flags & ITEM_FLAG1))
				break; //Passive clock, don't use
			if((itm.flags & ITEM_FLAG2) && watch) //"Can't activate while clock active"
				break;
			if(!(checkbunny(itemid) && checkmagiccost(itemid))) //cost/bunny check
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			
			setClock(watch=true);
			
			clock_zoras.clear();
				
			clockclk=itm.misc1;
			sfx(itm.usesound);
			break;
		}
		case itype_killem:
		{
			ret = false;
			if(!(itm.flags & ITEM_FLAG1))
				break; //Passive killemall, don't use
			
			if(!(checkbunny(itemid) && checkmagiccost(itemid))
				|| !can_kill_em_all()) //No enemies onscreen
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			
			kill_em_all();
			sfx(itm.usesound);
			break;
		}
		case itype_refill:
		{
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			bool did_something = false;
			
			if(itm.flags & ITEM_FLAG1) //Cure sword jinx
			{
				if(swordclk)
					did_something = true;
				swordclk = 0;
				verifyAWpn();
			}
			for(auto q = 0; q < 5; ++q)
			{
				auto ctr = itm.misc(q);
				if(unsigned(ctr) >= MAX_COUNTERS)
					continue;
				int16_t amnt = vbound(itm.misc(q+5),-32768,32767);
				if(!amnt) continue;
				bool gradual = itm.flags & ITEM_FLAG2;
				if(amnt > 0)
				{
					if(game->get_counter(ctr) + game->get_dcounter(ctr) >= game->get_maxcounter(ctr))
					{
						//Can't *do* anything... skip
						continue;
					}
					if(game->get_counter(ctr) >= game->get_maxcounter(ctr))
					{
						//Can't do anything unless affecting dcounter
						gradual = true;
					}
				}
				else //Negative
				{
					if(game->get_counter(ctr) + game->get_dcounter(ctr) <= 0)
					{
						//Can't *do* anything... skip
						continue;
					}
					if(game->get_counter(ctr) <= 0)
					{
						//Can't do anything unless affecting dcounter
						gradual = true;
					}
				}
				did_something = true;
				if(gradual) //Gradual
				{
					game->change_dcounter(amnt, ctr);
				}
				else
				{
					game->change_counter(amnt, ctr);
				}
			}
			if(!did_something)
			{
				return item_error();
			}
			paymagiccost(itemid);
			sfx(itm.usesound);
			ret = false;
			break;
		}
		
		default:
			ret = false;
	}
	
	if(itm.flags & ITEM_DOWNGRADE)
	{
		game->set_item(itemid,false);
		
		// Maybe Item Override has allowed the same item in both slots?
		if(Bwpn == itemid)
		{
			Bwpn = -1;
			game->forced_bwpn = -1;
			verifyBWpn();
		}
		
		if(Awpn == itemid)
		{
			Awpn = -1;
			game->forced_awpn = -1;
			verifyAWpn();
		}
		
		if(Xwpn == itemid)
		{
			Xwpn = -1;
			game->forced_xwpn = -1;
			verifyXWpn();
		}
		
		if(Ywpn == itemid)
		{
			Ywpn = -1;
			game->forced_ywpn = -1;
			verifyYWpn();
		}
	}

	return ret;
}


bool HeroClass::doattack()
{
	if(lift_wpn && (liftflags & LIFTFL_DIS_ITEMS))
		return false;
	//int32_t s = BSZ ? 0 : 11;
	int32_t s = (zinit.heroAnimationStyle==las_bszelda) ? 0 : 11;
	
	int32_t bugnetid = (directWpn>-1 && itemsbuf[directWpn].family==itype_bugnet) ? directWpn : current_item_id(itype_bugnet);
	if(attack==wBugNet && bugnetid!=-1)
	{
		if(++attackclk >= NET_CLK_TOTAL)
			return false;
		
		return true;
	}
	
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
	
	int32_t candleid = (directWpn>-1 && itemsbuf[directWpn].family==itype_candle) ? directWpn : current_item_id(itype_candle);
	int32_t byrnaid = (directWpn>-1 && itemsbuf[directWpn].family==itype_cbyrna) ? directWpn : current_item_id(itype_cbyrna);
	// An attack can be "walked out-of" after 8 frames, unless it's:
	// * a sword stab
	// * a hammer pound
	// * a wand thrust
	// * a candle thrust
	// * a cane thrust
	// In which case it should continue.
	if((attack==wCatching && attackclk>4)||(attack!=wWand && attack!=wSword && attack!=wHammer
											&& (attack!=wFire || (candleid!=-1 && !(itemsbuf[candleid].wpn)))
											&& (attack!=wCByrna || (byrnaid!=-1 && !(itemsbuf[byrnaid].wpn)))
											&& (attack != wBugNet) && attackclk>7))
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
	int32_t magiccharge = 192, normalcharge = 64;
	int32_t itemid = current_item_id(itype_chargering);
	
	if(itemid>=0)
	{
		normalcharge = itemsbuf[itemid].misc1;
		magiccharge = itemsbuf[itemid].misc2;
	}
	
	int scrollid = current_item_id(attack==wHammer ? itype_quakescroll : itype_spinscroll);
	int scroll2id = current_item_id(attack==wHammer ? itype_quakescroll2 : itype_spinscroll2);
	
	bool doCharge=true;
	if(z!=0 && fakez != 0)
		doCharge=false;
	if(attack==wSword)
	{
		if(!(attackclk==SWORDCHARGEFRAME && isWpnPressed(itype_sword)))
			doCharge=false;
		else if(charging<=normalcharge)
		{
			if(scrollid<0 || !(checkbunny(scrollid) && checkmagiccost(scrollid)))
				doCharge=false;
		}
	}
	else if(attack==wHammer)
	{
		if(!(attackclk==HAMMERCHARGEFRAME && isWpnPressed(itype_hammer)))
			doCharge=false;
		else if(charging<=normalcharge)
		{
			if(scrollid<0 || !(checkbunny(scrollid) && checkmagiccost(scrollid)))
				doCharge=false;
		}
	}
	else
		doCharge=false;
	
	// charging up weapon...
	if(doCharge)
	{
		// Increase charging while holding down button.
		if(spins==0 && charging<magiccharge)
			charging++;
			
		// Once a charging threshold is reached, play the sound.
		if(charging==normalcharge)
		{
			if(!(itemsbuf[scrollid].flags&ITEM_FLAG1))
				paymagiccost(scrollid);
			sfx(itemsbuf[scrollid].usesound2,pan(x.getInt()));
		}
		else if(charging==magiccharge)
		{
			if(scroll2id>-1 && checkbunny(scroll2id) && checkmagiccost(scroll2id))
			{
				if(!(itemsbuf[scroll2id].flags&ITEM_FLAG1))
					paymagiccost(scroll2id);
				charging++; // charging>magiccharge signifies a successful supercharge.
				sfx(itemsbuf[scroll2id].usesound2,pan(x.getInt()));
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
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
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
				bool super = charging>magiccharge && scroll2id > -1;
				int id = super ? scroll2id : scrollid;
				itemdata const& spinscroll = itemsbuf[id];
				bool paid = !(spinscroll.flags&ITEM_FLAG1);
				if(!paid && checkbunny(id) && checkmagiccost(id))
				{
					paid = true;
					paymagiccost(id);
				}
				if(paid)
				{
					currentscroll = id;
					spins=(spinscroll.misc1*4) + (super ? -3 : 1);
					attackclk=1;
					if(!get_qr(qr_BROKEN_SWORD_SPIN_TRIGGERS))
						if(weapon* w = find_first_wtype(wSword))
							w->reset_wgrids();
					sfx(spinscroll.usesound,pan(x.getInt()));
					if(spinscroll.flags&ITEM_FLAG1)
						paymagiccost(id);
				}
			}
			else if(attack==wHammer && sideviewhammerpound())
			{
				bool super = charging>magiccharge && scroll2id > -1;
				int id = super ? scroll2id : scrollid;
				itemdata const& quakescroll = itemsbuf[id];
				bool paid = !(quakescroll.flags&ITEM_FLAG1);
				if(!paid && checkbunny(id) && checkmagiccost(id))
				{
					paid = true;
					paymagiccost(id);
				}
				if(paid)
				{
					currentscroll = id;
					spins = super ? 2 : 1;
					sfx(quakescroll.usesound,pan(x.getInt()));
					quakeclk=quakescroll.misc1;
					
					// general area stun
					for(int32_t i=0; i<GuyCount(); i++)
					{
						if(!isflier(GuyID(i)))
						{
							StunGuy(i,quakescroll.misc2-distance(x,y,GuyX(i),GuyY(i)));
						}
					}
					
					int hmrid = (directWpn>-1 && itemsbuf[directWpn].family==itype_hammer) ? directWpn : current_item_id(itype_hammer);
					int hmrlvl = hmrid < 0 ? 1 : itemsbuf[hmrid].fam_type;
					if(hmrlvl < 1) hmrlvl = 1;
					int rad = quakescroll.misc2;
					// TODO z3
					for(int pos = 0; pos < 176; ++pos)
					{
						if(distance(x,y,COMBOX(pos),COMBOY(pos)) > rad) continue;
						for(int lyr = 0; lyr < 7; ++lyr)
						{
							int cid = FFCore.tempScreens[lyr]->data[pos];
							newcombo const& cmb = combobuf[cid];
							if(cmb.triggerflags[2] & ((super?combotriggerSQUAKESTUN:0)|combotriggerQUAKESTUN))
							{
								if((cmb.triggerflags[0]&combotriggerINVERTMINMAX)
									? hmrlvl <= cmb.triggerlevel
									: hmrlvl >= cmb.triggerlevel)
									do_trigger_combo(lyr,pos);
							}
						}
					}
					for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
						newcombo const& cmb = combobuf[ffc_handle.data()];
						if(distance(x,y,ffc_handle.ffc->x,ffc_handle.ffc->y) > rad) return;

						if(cmb.triggerflags[2] & ((super?combotriggerSQUAKESTUN:0)|combotriggerQUAKESTUN))
						{
							if((cmb.triggerflags[0]&combotriggerINVERTMINMAX)
								? hmrlvl <= cmb.triggerlevel
								: hmrlvl >= cmb.triggerlevel)
								do_trigger_combo_ffc(ffc_handle);
						}
					});
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
	
	int32_t crossid = current_item_id(itype_crossscroll);  //has Cross Beams scroll
	
	if(attackclk==13 || (attackclk==7 && spins>1 && attack != wHammer && crossid >=0 && checkbunny(crossid) && checkmagiccost(crossid)))
	{
	
		int32_t wpnid = (directWpn>-1 && itemsbuf[directWpn].family==itype_sword) ? directWpn : current_item_id(itype_sword);
		int64_t templife = wpnid>=0? itemsbuf[wpnid].misc1 : 0;
		
		if(wpnid>=0 && itemsbuf[wpnid].flags & ITEM_FLAG1)
		{
			templife=templife*game->get_maxlife();
			templife=templife/100;
		}
		else
		{
			templife*=game->get_hp_per_heart();
		}
		
		bool normalbeam = (int64_t(game->get_life())+(get_qr(qr_QUARTERHEART)?((game->get_hp_per_heart()/4)-1):((game->get_hp_per_heart()/2)-1))>=templife);
		int32_t perilid = current_item_id(itype_perilscroll);
		bool perilbeam = (perilid>=0 && wpnid>=0 && game->get_life()<=itemsbuf[perilid].misc1*game->get_hp_per_heart()
						  && checkbunny(perilid) && checkmagiccost(perilid)
						  // Must actually be able to shoot sword beams
						  && ((itemsbuf[wpnid].flags & ITEM_FLAG1)
							  || itemsbuf[wpnid].misc1 <= game->get_maxlife()/game->get_hp_per_heart()));
							  
		if(attack==wSword && !tapping)
		{
			if(perilbeam || normalbeam)
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
			else misc_internal_hero_flags &= ~LF_PAID_SWORD_COST;
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

bool HeroClass::can_attack()
{
	int32_t currentSwordOrWand = (itemsbuf[dowpn].family == itype_wand || itemsbuf[dowpn].family == itype_sword)?dowpn:-1;
    if(action==hopping || action==swimming || action==freeze || action==sideswimfreeze
		|| lstunclock > 0 || is_conveyor_stunned || spins>0 || usingActiveShield()
		|| ((action==attacking||action==sideswimattacking)
			&& ((attack!=wSword && attack!=wWand) || !(itemsbuf[currentSwordOrWand].flags & ITEM_FLAG5))
			&& charging!=0))
    {
        return false;
    }
    
    int32_t r = (isdungeon()) ? 16 : 0;
    int32_t r2 = get_qr(qr_NOBORDER) ? 0 : 8;
    
    if(!get_qr(qr_ITEMSONEDGES)) switch(dir)
        {
        case up:
        case down:
            return !(y<(r2+r) || y>(world_h-16-r-r2));
            
        case left:
        case right:
            return !(x<(r2+r) || x>(world_w-16-r-r2));
        }
        
    return true;
}

bool isRaftFlag(int32_t flag)
{
    return (flag==mfRAFT || flag==mfRAFT_BRANCH || flag==mfRAFT_BOUNCE);
}

void handle_lens_triggers(int32_t l_id)
{
	bool enabled = l_id >= 0 && (itemsbuf[l_id].flags & ITEM_FLAG6);

	for_every_rpos_in_region([&](const rpos_handle_t& rpos_handle) {
		newcombo const& cmb = combobuf[rpos_handle.data()];
		if (enabled ? (cmb.triggerflags[1] & combotriggerLENSON)
			: (cmb.triggerflags[1] & combotriggerLENSOFF))
		{
			do_trigger_combo(rpos_handle);
		}
	});

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			newcombo const& cmb = combobuf[ffc_handle.data()];
			if (enabled ? (cmb.triggerflags[1] & combotriggerLENSON)
				: (cmb.triggerflags[1] & combotriggerLENSOFF))
			{
				do_trigger_combo_ffc(ffc_handle);
			}
		});
	}
}

void do_lens()
{
	if ( FFCore.getQuestHeaderInfo(vZelda) < 0x250 ) //2.10 or earlier
	{
		do_210_lens();
		return;
	}
	
	int32_t wpnPressed = getWpnPressed(itype_lens);
	int32_t itemid = lensid >= 0 ? lensid : wpnPressed>0 ? wpnPressed : Hero.getLastLensID()>0 ? Hero.getLastLensID() : current_item_id(itype_lens);
	if(itemid >= 0)
	{
		if(isWpnPressed(itype_lens) && checkitem_jinx(itemid) && !lensclk && checkbunny(itemid) && checkmagiccost(itemid))
		{
			if(lensid<0)
			{
				lensid=itemid;
				if(itemsbuf[itemid].family == itype_lens)
				Hero.setLastLensID(itemid);
				if(get_qr(qr_MORESOUNDS)) sfx(itemsbuf[itemid].usesound);
			}
			
			paymagiccost(itemid, true); //Needs to ignore timer cause lensclk is our timer.
			
			if(itemid>=0 && itemsbuf[itemid].script != 0 && !did_scriptl && !(FFCore.doscript(ScriptType::Item, itemid) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				//clear the item script stack for a new script
				int i = itemid;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
				did_scriptl=true;
			}
			
			if (itemsbuf[itemid].magiccosttimer[0]) lensclk = itemsbuf[itemid].magiccosttimer[0];
			else lensclk = 12;
		}
		else
		{
			did_scriptl=false;
			if(!lensclk)
			{
				
				if(lensid>-1)
				{
					lensid=-1;
					lensclk = 0;
					
					if(get_qr(qr_MORESOUNDS)) sfx(WAV_ZN1LENSOFF);
				}
			}
		}
	}
	handle_lens_triggers(lensid);
}

void do_210_lens()
{
	int32_t wpnPressed = getWpnPressed(itype_lens);
	int32_t itemid = lensid >= 0 ? lensid : wpnPressed>-1 ? wpnPressed : current_item_id(itype_lens);
	if(itemid<0)
		return;
	
	if(isWpnPressed(itype_lens) && checkitem_jinx(itemid) && !lensclk && checkmagiccost(itemid))
	{
		if(lensid<0)
		{
			lensid=itemid;
			
			if(get_qr(qr_MORESOUNDS)) sfx(itemsbuf[itemid].usesound);
		}
		
		paymagiccost(itemid, true);
		
		if(itemid>=0 && itemsbuf[itemid].script != 0 && !did_scriptl && !(FFCore.doscript(ScriptType::Item, itemid) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
		{
			//clear the item script stack for a new script
			int i = itemid;
			FFCore.reset_script_engine_data(ScriptType::Item, i);
			ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
			did_scriptl=true;
		}
		
		if (itemsbuf[itemid].magiccosttimer[0]) lensclk = itemsbuf[itemid].magiccosttimer[0];
		else lensclk = 12;
	}
	else
	{
		did_scriptl=false;
		
		if(lensid>-1 && !(isWpnPressed(itype_lens) && checkitem_jinx(itemid) && checkmagiccost(itemid)))
		{
			lensid=-1;
			lensclk = 0;
			
			if(get_qr(qr_MORESOUNDS)) sfx(WAV_ZN1LENSOFF);
		}
	}
}

void HeroClass::do_hopping()
{
    do_lens();
    
    if(hopclk==0xFF) //|| (diagonalMovement && hopclk >= 0xFF) ))                                         // swimming
			//Possible fix for exiting water in diagonal movement. -Z
    {
		int32_t flippers_id = current_item_id(itype_flippers);
        if(diveclk>0)
		{
            --diveclk;
			if(flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG2 && DrunkrAbtn()) //Cancellable Diving -V
			{
				diveclk = itemsbuf[flippers_id].misc2;
			}
		}
        else if(DrunkrAbtn())
        {
            bool global_diving=(flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG1);
            bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
            
            if(global_diving==screen_diving)
                diveclk = (flippers_id < 0 ? 80 : (itemsbuf[flippers_id].misc1 + itemsbuf[flippers_id].misc2));
        }
        
        if((!(x.getInt()&7) && !(y.getInt()&7)) || (diagonalMovement||NO_GRIDLOCK))
        {
		SetSwim();
		hopclk = 0;
		if (!IsSideSwim()) 
		{
			charging = attackclk = 0;
			tapping = false;
		}
        }
        else
        {
            herostep();
            
            if(!isDiving() || (frame&1))
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
        if((diagonalMovement||NO_GRIDLOCK))
        {
            if(hopclk==1) //hopping out
		    //>= 1 possible fix for getting stuck on land edges. 
		//No, this is not a clock. it's a type. 1 == out, 2 == in. 
            {
                if(hopdir!=-1) dir=hopdir;
                
                landswim=0;
                
                if(dir==up)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)-1), -1, x,y+(bigHitbox?0:8)-1, true, false) && !iswaterex_z3(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1), -1, x+8,y+(bigHitbox?0:8)-1, true, false) && !iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1), -1, x+15,y+(bigHitbox?0:8)-1, true, false))
                        sidestep=1;
                    else if(!iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)-1), -1, x,y+(bigHitbox?0:8)-1, true, false) && !iswaterex_z3(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1), -1, x+7,y+(bigHitbox?0:8)-1, true, false) && iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1), -1, x+15,y+(bigHitbox?0:8)-1, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y--;
                    
                    if(!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+15), -1, x.getInt(),y.getInt()+15, true, false))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setHeroAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==down)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(iswaterex_z3(MAPCOMBO(x,y+16), -1, x,y+16, true, false) && !iswaterex_z3(MAPCOMBO(x+8,y+16), -1, x+8,y+16, true, false) && !iswaterex_z3(MAPCOMBO(x+15,y+16), -1, x+15,y+16, true, false))
                        sidestep=1;
                    else if(!iswaterex_z3(MAPCOMBO(x,y+16), -1, x,y+16, true, false) && !iswaterex_z3(MAPCOMBO(x+8,y+16), -1, x+8,y+16, true, false) && iswaterex_z3(MAPCOMBO(x+15,y+16), -1, x+15,y+16, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y++;
                    
                    if(!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+15), -1, x.getInt(),y.getInt()+15, true, false))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setHeroAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==left)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?0:8)), -1, x-1,y+(bigHitbox?0:8), true, false) && !iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?8:12)), -1, x-1,y+(bigHitbox?8:12), true, false) && !iswaterex_z3(MAPCOMBO(x-1,y+15), -1, x-1,y+15, true, false))
                        sidestep=1;
                    else if(!iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?0:8)), -1, x-1,y+(bigHitbox?0:8), true, false) && !iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?7:11)), -1, x-1,y+(bigHitbox?7:11), true, false) && iswaterex_z3(MAPCOMBO(x-1,y+15), -1, x-1,y+15, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x--;
                    
                    if(!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&!iswaterex_z3(MAPCOMBO(x.getInt()+15,y.getInt()+8), -1, x.getInt()+15,y.getInt()+8, true, false))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setHeroAction(none);
                        hopdir=-1;
                    }
                }
                
                if(dir==right)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?0:8)), -1, x+16,y+(bigHitbox?0:8), true, false) && !iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?8:12)), -1, x+16,y+(bigHitbox?8:12), true, false) && !iswaterex_z3(MAPCOMBO(x+16,y+15), -1, x+16,y+15, true, false))
                        sidestep=1;
                    else if(!iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?0:8)), -1, x+16,y+(bigHitbox?0:8), true, false) && !iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?7:11)), -1, x+16,y+(bigHitbox?7:11), true, false) && iswaterex_z3(MAPCOMBO(x+16,y+15), -1, x+16,y+15, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x++;
                    
                    if(!iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&!iswaterex_z3(MAPCOMBO(x.getInt()+15,y.getInt()+8), -1, x.getInt()+15,y.getInt()+8, true, false))
                    {
                        hopclk=0;
                        diveclk=0;
                        action=none; FFCore.setHeroAction(none);
                        hopdir=-1;
                    }
                }
            }
            
            if(hopclk==2) //hopping in
            {
                landswim=0;
                
                if(dir==up)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(!iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)-1), -1, x,y+(bigHitbox?0:8)-1, true, false) && iswaterex_z3(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1), -1, x+8,y+(bigHitbox?0:8)-1, true, false) && iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1), -1, x+15,y+(bigHitbox?0:8)-1, true, false))
                        sidestep=1;
                    else if(iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)-1), -1, x,y+(bigHitbox?0:8)-1, true, false) && iswaterex_z3(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1), -1, x+7,y+(bigHitbox?0:8)-1, true, false) && !iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)-1), -1, x+15,y+(bigHitbox?0:8)-1, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y--;
                    
		    if(iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+15), -1, x.getInt(),y.getInt()+15, true, false))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        SetSwim();
                    }
                }
                
                if(dir==down)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(!iswaterex_z3(MAPCOMBO(x,y+16), -1, x,y+16, true, false) && iswaterex_z3(MAPCOMBO(x+8,y+16), -1, x+8,y+16, true, false) && iswaterex_z3(MAPCOMBO(x+15,y+16), -1, x+15,y+16, true, false))
                        sidestep=1;
                    else if(iswaterex_z3(MAPCOMBO(x,y+16), -1, x,y+16, true, false) && iswaterex_z3(MAPCOMBO(x+8,y+16), -1, x+8,y+16, true, false) && !iswaterex_z3(MAPCOMBO(x+15,y+16), -1, x+15,y+16, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) x++;
                    else if(sidestep==2) x--;
                    else y++;
                    
		    if(iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+15), -1, x.getInt(),y.getInt()+15, true, false))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        SetSwim();
                        if (!IsSideSwim()) reset_swordcharge();
                    }
                }
                
                if(dir==left)
                {
                    herostep();
                    herostep();
                    int32_t sidestep=0;
                    
                    if(!iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?0:8)), -1, x-1,y+(bigHitbox?0:8), true, false) && iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?8:12)), -1, x-1,y+(bigHitbox?8:12), true, false) && iswaterex_z3(MAPCOMBO(x-1,y+15), -1, x-1,y+15, true, false))
                        sidestep=1;
                    else if(iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?0:8)), -1, x-1,y+(bigHitbox?0:8), true, false) && iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?7:11)), -1, x-1,y+(bigHitbox?7:11), true, false) && !iswaterex_z3(MAPCOMBO(x-1,y+15), -1, x-1,y+15, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x--;
                    
		    if(iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&iswaterex_z3(MAPCOMBO(x.getInt()+15,y.getInt()+8), -1, x.getInt()+15,y.getInt()+8, true, false))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        SetSwim();
                    }
                }
                
                if(dir==right)
                {
                    herostep();
                    herostep();
                    
                    int32_t sidestep=0;
                    
                    if(!iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?0:8)), -1, x+16,y+(bigHitbox?0:8), true, false) && iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?8:12)), -1, x+16,y+(bigHitbox?8:12), true, false) && iswaterex_z3(MAPCOMBO(x+16,y+15), -1, x+16,y+15, true, false))
                        sidestep=1;
                    else if(iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?0:8)), -1, x+16,y+(bigHitbox?0:8), true, false) && iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?7:11)), -1, x+16,y+(bigHitbox?7:11), true, false) && !iswaterex_z3(MAPCOMBO(x+16,y+15), -1, x+16,y+15, true, false))
                        sidestep=2;
                        
                    if(sidestep==1) y++;
                    else if(sidestep==2) y--;
                    else x++;
                    
		    if(iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+(bigHitbox?0:8)), -1, x.getInt(),y.getInt()+(bigHitbox?0:8), true, false)&&iswaterex_z3(MAPCOMBO(x.getInt()+15,y.getInt()+8), -1, x.getInt()+15,y.getInt()+8, true, false))
                    {
                        hopclk=0xFF;
                        diveclk=0;
                        SetSwim();
                    }
                }
            }
            
        }
        else
        {
            if((dir<left ? !(x.getInt()&7) && !(y.getInt()&15) : !(x.getInt()&15) && !(y.getInt()&7)))
            {
                action=none; FFCore.setHeroAction(none);
                hopclk = 0;
                diveclk = 0;
                
                if(iswaterex_z3(MAPCOMBO(x.getInt(),y.getInt()+8), -1, x.getInt(),y.getInt()+8, true, false))
                {
                    // hopped in
                    SetSwim();
                    if (!IsSideSwim()) attackclk = charging = spins = 0;
                }
            }
            else
            {
                herostep();
                herostep();
                
                if(++hero_count>(16*hero_animation_speed))
                    hero_count=0;
                    
                int32_t xofs2 = x.getInt()&15;
                int32_t yofs2 = y.getInt()&15;
                int32_t s = 1 + (frame&1);
                
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

void HeroClass::do_rafting()
{

	if(toogam)
	{
		action=none; FFCore.setHeroAction(none);
		return;
	}
	
	FFCore.setHeroAction(rafting);
	
	do_lens();
	
	herostep();
	
	//Calculate rafting speed
	int32_t raft_item = current_item_id(itype_raft);
	int32_t raft_step = (raft_item < 0 ? 1 : itemsbuf[raft_item].misc1);
	raft_step = vbound(raft_step, -8, 5);
	int32_t raft_time = raft_step < 0 ? 1<<(-raft_step) : 1;
	if(raft_step < 0) raft_step = 1;
	int32_t step_inc = 1 << (raft_step - 1);
	// Fix position
	if(raft_step > 1)
	{
		if(x.getInt() & (step_inc-1))
		{
			x = x.getInt() & ~(step_inc-1);
		}
		if(y.getInt() & (step_inc-1))
		{
			y = y.getInt() & ~(step_inc-1);
		}
	}
	// Inc clock, check if we need to move this frame
	++raftclk;
	if((raftclk % raft_time) || raft_step == 0) return; //No movement this frame
	
	if(!(x.getInt()&15) && !(y.getInt()&15))
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
				else if(y>0 && y<world_h-16) 
				{
					action=none; FFCore.setHeroAction(none);
					x = x.getInt();
					y = y.getInt();
				}
			}
			else //going left or right
			{
				if((isRaftFlag(nextflag(x,y,down,false))||isRaftFlag(nextflag(x,y,down,true))))
					dir=down;
				else if((isRaftFlag(nextflag(x,y,up,false))||isRaftFlag(nextflag(x,y,up,true))))
					dir=up;
				else if(x>0 && x<world_w-16)
				{
					action=none; FFCore.setHeroAction(none);
					x = x.getInt();
					y = y.getInt();
				}
			}
		}
	}
	
skip:

	switch(dir)
	{
	case up:
		if(x.getInt()&15)
		{
			if(x.getInt()&8)
				x++;
			else x--;
		}
		else y -= step_inc;
		
		break;
		
	case down:
		if(x.getInt()&15)
		{
			if(x.getInt()&8)
				x++;
			else x--;
		}
		else y += step_inc;
		
		break;
		
	case left:
		if(y.getInt()&15)
		{
		if (get_qr(qr_BETTER_RAFT_2))
		{
			if ((y.getInt() % 16) < 4) y--;
			else y++;
		}
		else
		{
			if(y.getInt()&8)
			y++;
			else y--;
		}
		}
		else x -= step_inc;
		
		break;
		
	case right:
		if(y.getInt()&15)
		{
		if (get_qr(qr_BETTER_RAFT_2))
		{
			if ((y.getInt() % 16) <= 4) y--;
			else y++;
		}
		else
		{
			if(y.getInt()&8)
			y++;
			else y--;
		}
		}
		else x += step_inc;
		
		break;
	}
}

bool HeroClass::try_hover()
{
	if(hoverclk <= 0 && can_use_item(itype_hoverboots,i_hoverboots) && !ladderx && !laddery && !(hoverflags & HOV_OUT))
	{
		int32_t itemid = current_item_id(itype_hoverboots);
		if(hoverclk < 0)
			hoverclk = -hoverclk;
		else
		{
			fall = fakefall = jumping = 0;
			if(itemsbuf[itemid].misc1)
				hoverclk = itemsbuf[itemid].misc1;
			else
			{
				hoverclk = 1;
				hoverflags |= HOV_INF;
			}
			
				
			sfx(itemsbuf[itemid].usesound,pan(x.getInt()));
		}
		if(itemsbuf[itemid].wpn)
			decorations.add(new dHover(x, y, dHOVER, 0));
		return true;
	}
	return false;
}

//Returns bitwise; lower 8 are dir pulled in, next 16 are combo ID, 25th bit is bool for if can be resisted
//Returns '-1' if not being pulled
//Returns '-2' if should be falling in
int32_t HeroClass::check_pitslide(bool ignore_hover)
{
	//Pitfall todo -Emily
	//Iron boots; can't fight slipping, 2px/frame
	//Scripted variables to read pull dir/clk (clk only for non-hero)
	//Implement falling for all sprite types (npc AI)
	//    Fall/slipping tiles for enemies
	//    Fall/slipping SFX for enemies
	//    Fall SFX for items/weapons
	//    Weapons/Misc sprite shared for falling items/weapons
	//Maybe slip SFX for Hero?
	// Weapons/Misc sprite override for falling sprite?
	//Update std.zh with relevant new stuff
	if(can_pitfall(ignore_hover))
	{
		bool can_diag = (diagonalMovement || get_qr(qr_DISABLE_4WAY_GRIDLOCK));
		int32_t ispitul = getpitfall(x,y+(bigHitbox?0:8));
		int32_t ispitbl = getpitfall(x,y+15);
		int32_t ispitur = getpitfall(x+15,y+(bigHitbox?0:8));
		int32_t ispitbr = getpitfall(x+15,y+15);
		int32_t ispitul_50 = getpitfall(x+8,y+(bigHitbox?8:12));
		int32_t ispitbl_50 = getpitfall(x+8,y+(bigHitbox?7:11));
		int32_t ispitur_50 = getpitfall(x+7,y+(bigHitbox?8:12));
		int32_t ispitbr_50 = getpitfall(x+7,y+(bigHitbox?7:11));
		int32_t ispitul_75 = getpitfall(x+12,y+(bigHitbox?12:14));
		int32_t ispitbl_75 = getpitfall(x+12,y+(bigHitbox?3:9));
		int32_t ispitur_75 = getpitfall(x+3,y+(bigHitbox?12:14));
		int32_t ispitbr_75 = getpitfall(x+3,y+(bigHitbox?3:9));
		static const int32_t flag_pit_irresistable = (1<<24);
		switch((ispitul?1:0) + (ispitur?1:0) + (ispitbl?1:0) + (ispitbr?1:0))
		{
			case 4: return -2; //Fully over pit; fall in
			case 3:
			{
				if(ispitul && ispitur && ispitbl) //UL_3
				{
					if(ispitul_50)
					{
						if(!ispitul_75 && (DrunkDown() || DrunkRight())) return -1;
						return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
					}
				}
				else if(ispitul && ispitur && ispitbr) //UR_3
				{
					if(ispitur_50)
					{
						if(!ispitur_75 && (DrunkDown() || DrunkLeft())) return -1;
						return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
					}
				}
				else if(ispitul && ispitbl && ispitbr) //BL_3
				{
					if(ispitbl_50)
					{
						if(!ispitbl_75 && (DrunkUp() || DrunkRight())) return -1;
						return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
					}
				}
				else if(ispitbl && ispitur && ispitbr) //BR_3
				{
					if(ispitbr_50)
					{
						if(!ispitbr_75 && (DrunkUp() || DrunkLeft())) return -1;
						return (can_diag ? r_down : right) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
					}
				}
				break;
			}
			case 2:
			{
				if(ispitul && ispitur) //Up
				{
					if(DrunkDown())
					{
						if(ispitul_75 && ispitur_75) //Straight up
						{
							return up | flag_pit_irresistable | (ispitul << 8);
						}
						else if(ispitul_75)
						{
							return (can_diag ? l_up : left) | flag_pit_irresistable | (ispitul << 8);
						}
						else if(ispitur_75)
						{
							return (can_diag ? r_up : right) | flag_pit_irresistable | (ispitur << 8);
						}
						else return -1;
					}
					else
					{
						if(ispitul_50 && ispitur_50) //Straight up
						{
							return up | ((ispitul_75 || ispitur_75) ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitul_50)
						{
							if(DrunkRight() && !ispitul_75) return -1;
							return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitur_50)
						{
							if(DrunkLeft() && !ispitur_75) return -1;
							return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
						}
					}
				}
				else if(ispitbl && ispitbr) //Down
				{
					if(DrunkUp())
					{
						if(ispitbl_75 && ispitbr_75) //Straight down
						{
							return down | flag_pit_irresistable | (ispitbl << 8);
						}
						else if(ispitbl_75)
						{
							return (can_diag ? l_down : left) | flag_pit_irresistable | (ispitbl << 8);
						}
						else if(ispitbr_75)
						{
							return (can_diag ? r_down : right) | flag_pit_irresistable | (ispitbr << 8);
						}
						else return -1;
					}
					else
					{
						if(ispitbl_50 && ispitbr_50) //Straight down
						{
							return down | ((ispitbl_75 || ispitbr_75) ? flag_pit_irresistable : 0) | (ispitbl << 8);
						}
						else if(ispitbl_50)
						{
							if(DrunkRight() && !ispitbl_75) return -1;
							return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
						}
						else if(ispitbr_50)
						{
							if(DrunkLeft() && !ispitbr_75) return -1;
							return (can_diag ? r_down : right) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
						}
					}
				}
				else if(ispitbl && ispitul) //Left
				{
					if(DrunkRight())
					{
						if(ispitul_75 && ispitbl_75) //Straight left
						{
							return left | flag_pit_irresistable | (ispitul << 8);
						}
						else if(ispitul_75)
						{
							return (can_diag ? l_up : up) | flag_pit_irresistable | (ispitul << 8);
						}
						else if(ispitbl_75)
						{
							return (can_diag ? l_down : down) | flag_pit_irresistable | (ispitbl << 8);
						}
						else return -1;
					}
					else
					{
						if(ispitul_50 && ispitbl_50) //Straight left
						{
							return left | ((ispitul_75 || ispitbl_75) ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitul_50)
						{
							if(DrunkDown() && !ispitul_75) return -1;
							return (can_diag ? l_up : up) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitbl_50)
						{
							if(DrunkUp() && !ispitbl_75) return -1;
							return (can_diag ? l_down : down) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
						}
					}
				}
				else if(ispitbr && ispitur) //Right
				{
					if(DrunkLeft())
					{
						if(ispitur_75 && ispitbr_75) //Straight right
						{
							return right | flag_pit_irresistable | (ispitur << 8);
						}
						else if(ispitur_75)
						{
							return (can_diag ? r_up : up) | flag_pit_irresistable | (ispitur << 8);
						}
						else if(ispitbr_75)
						{
							return (can_diag ? r_down : down) | flag_pit_irresistable | (ispitbr << 8);
						}
						else return -1;
					}
					else
					{
						if(ispitur_50 && ispitbr_50) //Straight right
						{
							return right | ((ispitur_75 || ispitbr_75) ? flag_pit_irresistable : 0) | (ispitur << 8);
						}
						else if(ispitur_50)
						{
							if(DrunkDown() && !ispitur_75) return -1;
							return (can_diag ? r_up : up) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
						}
						else if(ispitbr_50)
						{
							if(DrunkUp() && !ispitbr_75) return -1;
							return (can_diag ? r_down : down) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
						}
					}
				}
				break;
			}
			case 1:
			{
				if(ispitul && ispitul_50) //UL_1
				{
					if(!ispitul_75 && (DrunkDown() || DrunkRight())) return -1;
					return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
				}
				if(ispitur && ispitur_50) //UR_1
				{
					if(!ispitur_75 && (DrunkDown() || DrunkLeft())) return -1;
					return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
				}
				if(ispitbl && ispitbl_50) //BL_1
				{
					if(!ispitbl_75 && (DrunkUp() || DrunkRight())) return -1;
					return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
				}
				if(ispitbr && ispitbr_50) //BR_1
				{
					if(!ispitbr_75 && (DrunkUp() || DrunkLeft())) return -1;
					return (can_diag ? r_down : right) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
				}
				break;
			}
		}
	}
	return -1;
}

bool HeroClass::pitslide() //Runs pitslide movement; returns true if pit is irresistable
{
	pitfall();
	if(fallclk) return true;
	int32_t val = check_pitslide();
	//Val should not be -2 here; if -2 would have been returned, the 'return true' above should have triggered!
	if(val == -1)
	{
		pit_pulldir = -1;
		pit_pullclk = 0;
		return false;
	}
	int32_t dir = val&0xFF;
	int32_t cmbid = (val&0xFFFF00)>>8;
	int32_t sensitivity = combobuf[cmbid].attribytes[2];
	if(combobuf[cmbid].usrflags&cflag5) //No pull at all
	{
		pit_pulldir = -1;
		pit_pullclk = 0;
		return false;
	}
	if(dir > -1 && !(hoverflags & HOV_PITFALL_OUT) && try_hover()) //Engage hovers
	{
		pit_pulldir = -1;
		pit_pullclk = 0;
		return false;
	}
	pit_pulldir = dir;
	int32_t step = 1;
	if(sensitivity == 0)
	{
		step = 2;
		sensitivity = 1;
	}
	if(pit_pullclk++ % sensitivity) //No pull this frame
		return (val&0x100);
	for(; step > 0 && !fallclk; --step)
	{
		switch(dir)
		{
			case l_up: case l_down: case left:
				--x; break;
			case r_up: case r_down: case right:
				++x; break;
		}
		switch(dir)
		{
			case l_up: case r_up: case up:
				--y; break;
			case l_down: case r_down: case down:
				++y; break;
		}
		pitfall();
	}
	return fallclk || (val&0x100);
}

void HeroClass::pitfall()
{
	if(fallclk)
	{
		drop_liftwpn();
		if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x.getInt()));
		//Handle falling
		if(!--fallclk)
		{
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(fallCombo*10000);
			
			throwGenScriptEvent(GENSCR_EVENT_PLAYER_FALL);
			
			fallCombo = ev[0]/10000;
			if(fallCombo < 0 || fallCombo >= MAXCOMBOS)
				fallCombo = 0;
			
			int32_t dmg = game->get_hp_per_heart()/4;
			bool dmg_perc = false;
			bool warp = false;
			
			action=none; FFCore.setHeroAction(none);
			newcombo* cmb = fallCombo ? &combobuf[fallCombo] : NULL;
			if(cmb)
			{
				dmg = cmb->attributes[0]/10000L;
				dmg_perc = cmb->usrflags&cflag3;
				warp = cmb->usrflags&cflag1;
			}
			if(cheat_superman && dmg > 0)
				dmg = 0;
			if(dmg) //Damage
			{
				if(dmg > 0) hclk=48; //IFrames only if damaged, not if healed
				game->set_life(vbound(int32_t(dmg_perc ? game->get_life() - ((vbound(dmg,-100,100)/100.0)*game->get_maxlife()) : (game->get_life()-int64_t(dmg))),0,game->get_maxlife()));
			}
			if(warp) //Warp
			{
				sdir = dir;
				if(cmb->usrflags&cflag2) //Direct Warp
				{
					didpit=true;
					pitx=x;
					pity=y;
				}
				dowarp(0,vbound(cmb->attribytes[1],0,3),0);
			}
			else //Reset to screen entry
			{
				go_respawn_point();
			}
		}
	}
	else if(can_pitfall())
	{
		bool ispitul = ispitfall(x,y+(bigHitbox?0:8));
		bool ispitbl = ispitfall(x,y+15);
		bool ispitur = ispitfall(x+15,y+(bigHitbox?0:8));
		bool ispitbr = ispitfall(x+15,y+15);
		int32_t pitctr = getpitfall(x+8,y+(bigHitbox?8:12));
		if(ispitul && ispitbl && ispitur && ispitbr && pitctr)
		{
			if(!(hoverflags & HOV_PITFALL_OUT) && try_hover()) return;
			if(!bigHitbox && !ispitfall(x,y)) y = (y.getInt() + 8 - (y.getInt() % 8)); //Make the falling sprite fully over the pit
			fallclk = PITFALL_FALL_FRAMES;
			fallCombo = pitctr;
			action=falling; FFCore.setHeroAction(falling);
			spins = 0;
			charging = 0;
			drop_liftwpn();
		}
	}
}

void HeroClass::mod_steps(std::vector<zfix*>& v)
{
	bool can_combo = ((z==0 && fakez==0) || tmpscr->flags2&fAIRCOMBOS);
	bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1,-1) && can_combo) ||
					 (isSideViewHero() && (on_sideview_solid_oldpos(x,y,old_x,old_y)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1,-1));
			 //!DIMITODO: add QR for slow combos under hero
	if(slowcombo) for (int32_t i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x+7,y+8)].type == cBRIDGE && !_walkflag_layer(x+7,y+8,1, &(tmpscr2[i])))
				{
					slowcombo = false;
					break;
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x+7,y+8)].type == cBRIDGE && _effectflag_layer(x+7,y+8,1, &(tmpscr2[i])))
				{
					slowcombo = false;
					break;
				}
			}
		}
	}
	bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & ITEM_FLAG10);
	bool is_swimming = (action == swimming);
	int32_t shieldid = getCurrentActiveShield();
	if(shieldid > -1)
	{
		itemdata const& shield = itemsbuf[shieldid];
		if(shield.flags & ITEM_FLAG10) //Change Speed flag
		{
			zfix perc = shield.misc7;
			perc /= 100;
			if(perc < 0)
				perc = (perc*-1)+1;
			zfix add(shield.misc8);
			add /= 100;
			for(zfix* stp : v)
			{
				zfix& pix = *stp;
				pix = (pix * perc) + add;
			}
		}
	}
	
	if (can_combo)
	{
		rpos_t slow_rpos = COMBOPOS_REGION(x+7, y+8);
		for (int q = 6; q >= 0; --q)
		{
			int cid = get_rpos_handle(slow_rpos, q).data();
			newcombo const& cmb = combobuf[cid];

			if (cmb.speed_mult != 1 || cmb.speed_div || cmb.speed_add)
			{
				for (zfix* stp : v)
				{
					zfix& pix = *stp;
					pix *= cmb.speed_mult;
					if(cmb.speed_div)
						pix /= cmb.speed_div;
					pix += cmb.speed_add;
				}
			}

			if (q > 0 && cmb.type == cBRIDGE)
			{
				if(get_qr(qr_OLD_BRIDGE_COMBOS)
					? !_walkflag_layer(x+7,y+8,q-1)
					: _effectflag_layer(x+7,y+8,q-1))
				{
					break; //Bridge blocks speed change from below it
				}
			}
		}
	}

	zfix mult = 1, div = 1;
	if(is_swimming)
	{
		mult = game->swim_mult;
		div = game->swim_div;
	}
	else if(slowcharging && slowcombo) //1/2 speed
	{
		div = 2;
	}
	else if(slowcharging || slowcombo) //2/3 speed
	{
		mult = 2;
		div = 3;
	}
	if(!div) div = 1;
	if(mult != 1 || div != 1)
	{
		for(zfix* stp : v)
		{
			zfix& pix = *stp;
			pix = ((pix / div) * mult);
		}
	}
}

void HeroClass::moveheroOld()
{
	if(lstunclock || is_conveyor_stunned) return;
	int32_t xoff=x.getInt()&7;
	int32_t yoff=y.getInt()&7;
	if(NO_GRIDLOCK)
	{
		xoff = 0;
		yoff = 0;
	}
	int32_t push=pushing;
	int32_t oldladderx=-1000, oldladdery=-1000; // moved here because linux complains "init crosses goto ~Koopa
	pushing=0;
	zfix temp_step(hero_newstep);
	zfix temp_x(x);
	zfix temp_y(y);
	
	int32_t flippers_id = current_item_id(itype_flippers);
	itemdata const& itm = itemsbuf[flippers_id];
	byte intbtn = byte(itm.misc3&0xFF);
	bool dive_pressed = getIntBtnInput(intbtn, true, true, false, false, true);
	bool eatdive = false;
	if(diveclk>0)
	{
		if (isSideViewHero() && get_qr(qr_SIDESWIM)) diveclk = 0;
		--diveclk;
		if(isDiving() && flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG2 && dive_pressed) //Cancellable Diving -V
		{
			diveclk = itemsbuf[flippers_id].misc2;
			eatdive = true;
		}
	}
	else if(action == swimming && dive_pressed)
	{
		bool global_diving=(flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG1);
		bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
		
		if(global_diving==screen_diving)
		{
			diveclk = (flippers_id < 0 ? 80 : (itemsbuf[flippers_id].misc1 + itemsbuf[flippers_id].misc2));
			eatdive = true;
		}
	}
	if(eatdive)
		getIntBtnInput(intbtn, true, true, false, false, false);
	
	if(action==rafting)
	{
		do_rafting();
		
		if(action==rafting)
		{
			return;
		}
		
		
		set_respawn_point();
		trySideviewLadder();
	}
	
	int32_t olddirectwpn = directWpn; // To be reinstated if startwpn() fails
	int32_t btnwpn = -1;
	
	//&0xFFF removes the "bow & arrows" bitmask
	//The Quick Sword is allowed to interrupt attacks.
	int32_t currentSwordOrWand = (itemsbuf[dowpn].family == itype_wand || itemsbuf[dowpn].family == itype_sword)?dowpn:-1;
	if((!attackclk && action!=attacking && action != sideswimattacking) || ((attack==wSword || attack==wWand) && (itemsbuf[currentSwordOrWand].flags & ITEM_FLAG5)))
	{
		if(DrunkrBbtn())
		{
			btnwpn=getItemFamily(itemsbuf,Bwpn);
			dowpn = NEG_OR_MASK(Bwpn,0xFFF);
			directWpn = directItemB;
		}
		else if(DrunkrAbtn())
		{
			btnwpn=getItemFamily(itemsbuf,Awpn);
			dowpn = NEG_OR_MASK(Awpn,0xFFF);
			directWpn = directItemA;
		}
		else if(get_qr(qr_SET_XBUTTON_ITEMS) && DrunkrEx1btn())
		{
			btnwpn=getItemFamily(itemsbuf,Xwpn);
			dowpn = NEG_OR_MASK(Xwpn,0xFFF);
			directWpn = directItemX;
		}
		else if(get_qr(qr_SET_YBUTTON_ITEMS) && DrunkrEx2btn())
		{
			btnwpn=getItemFamily(itemsbuf,Ywpn);
			dowpn = NEG_OR_MASK(Ywpn,0xFFF);
			directWpn = directItemY;
		}
		
		if(directWpn >= MAXITEMS) directWpn = -1;
		
		// The Quick Sword only allows repeated sword or wand swings.
		if((action==attacking||action==sideswimattacking) && ((attack==wSword && btnwpn!=itype_sword) || (attack==wWand && btnwpn!=itype_wand)))
			btnwpn=-1;
	}
	
	auto swordid = (directWpn>-1 ? directWpn : current_item_id(itype_sword));
	if(can_attack() && (swordid > -1 && itemsbuf[swordid].family==itype_sword) && checkitem_jinx(swordid) && btnwpn==itype_sword && charging==0)
	{
		attackid=directWpn>-1 ? directWpn : current_item_id(itype_sword);
		if(checkbunny(attackid) && (checkmagiccost(attackid) || !(itemsbuf[attackid].flags & ITEM_FLAG6)))
		{
			if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_SWORD_COST))
			{
				paymagiccost(attackid,true);
				misc_internal_hero_flags |= LF_PAID_SWORD_COST;
			}
			SetAttack();
			attack=wSword;
			
			attackclk=0;
			sfx(itemsbuf[directWpn>-1 ? directWpn : current_item_id(itype_sword)].usesound, pan(x.getInt()));
			
			if(dowpn>-1 && itemsbuf[dowpn].script!=0 && !did_scripta && !(FFCore.doscript(ScriptType::Item, dowpn) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(!checkmagiccost(dowpn))
				{
					item_error();
				}
				else
				{
					//clear the item script stack for a new script
					int i = dowpn;
					FFCore.reset_script_engine_data(ScriptType::Item, i);
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
					did_scripta=true;
				}
			}
		}
		else
		{
			item_error();
		}
	}
	else
	{
		did_scripta=false;
	}
	
	if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && !getOnSideviewLadder())
	{
		if(DrunkUp() && canSideviewLadder())
		{
			setOnSideviewLadder(true);
		}
		else if(DrunkDown() && canSideviewLadder(true))
		{
			y+=1;
			setOnSideviewLadder(true);
		}
	}
	
	int32_t wx=x;
	int32_t wy=y;
	if((action==none || action==walking) && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
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
	
	bool no_jinx = true;
	if(can_attack() && btnwpn>itype_sword && charging==0 && btnwpn!=itype_rupee) // This depends on item 0 being a rupee...
	{
		bool paidmagic = false;
		bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
		if(!liftonly && btnwpn==itype_wand && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_wand : false) : current_item(itype_wand)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_wand);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & ITEM_FLAG6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_WAND_COST)){
					paymagiccost(attackid,true);
					misc_internal_hero_flags |= LF_PAID_WAND_COST;
				}
				SetAttack();
				attack=wWand;
				attackclk=0;
			}
			else
			{
				item_error();
			}
		}
		else if(!liftonly && (btnwpn==itype_hammer)&&!((action==attacking||action==sideswimattacking) && attack==wHammer)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_hammer : false) : current_item(itype_hammer)))
		{
			no_jinx = checkitem_jinx(dowpn);
			if(!(no_jinx && checkmagiccost(dowpn) && checkbunny(dowpn)))
			{
				item_error();
			}
			else
			{
				paymagiccost(dowpn);
				paidmagic = true;
				SetAttack();
				attack=wHammer;
				attackid=directWpn>-1 ? directWpn : current_item_id(itype_hammer);
				attackclk=0;
			}
		}
		else if(!liftonly && (btnwpn==itype_candle)&&!((action==attacking||action==sideswimattacking) && attack==wFire)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_candle : false) : current_item(itype_candle)))
		{
			//checkbunny handled where magic cost is paid
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_candle);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx)
			{
				SetAttack();
				attack=wFire;
				attackclk=0;
			}
		}
		else if(!liftonly && (btnwpn==itype_cbyrna)&&!((action==attacking||action==sideswimattacking) && attack==wCByrna)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_cbyrna : false) : current_item(itype_cbyrna)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_cbyrna);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & ITEM_FLAG6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_CBYRNA_COST)){
					paymagiccost(attackid,true);
					misc_internal_hero_flags |= LF_PAID_CBYRNA_COST;
				}
				SetAttack();
				attack=wCByrna;
				attackclk=0;
			}
			else
			{
				item_error();
			}
		}
		else if(!liftonly && (btnwpn==itype_bugnet)&&!((action==attacking||action==sideswimattacking) && attack==wBugNet)
				&& (directWpn>-1 ? (!item_disabled(directWpn) && itemsbuf[directWpn].family==itype_bugnet) : current_item(itype_bugnet)))
		{
			attackid = directWpn>-1 ? directWpn : current_item_id(itype_bugnet);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && checkmagiccost(attackid))
			{
				paymagiccost(attackid);
				SetAttack();
				attack = wBugNet;
				attackclk = 0;
				sfx(itemsbuf[attackid].usesound);
			}
			else
			{
				item_error();
			}
		}
		else
		{
			auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
			no_jinx = checkitem_jinx(itmid);
			if(no_jinx)
			{
				paidmagic = startwpn(itmid);
				
				if(paidmagic)
				{
					if(action==casting || action==drowning || action==lavadrowning || action == sideswimcasting || action==sidedrowning)
					{
						;
					}
					else
					{
						SetAttack();
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
		}
		
		if(dowpn>-1 && no_jinx && itemsbuf[dowpn].script!=0 && !did_scriptb && !(FFCore.doscript(ScriptType::Item, dowpn) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
		{
			if(!((paidmagic || checkmagiccost(dowpn)) && checkbunny(dowpn)))
			{
				item_error();
			}
			else
			{
				// Only charge for magic if item's magic cost wasn't already charged
				// for the item's main use.
				if(!paidmagic && attack!=wWand)
					paymagiccost(dowpn);
				//clear the item script stack for a new script
				int i = dowpn;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
				did_scriptb=true;
			}
		}
		
		if(no_jinx && (action==casting || action==drowning || action==lavadrowning || action == sideswimcasting || action==sidedrowning))
		{
			return;
		}
		if(!no_jinx)
			did_scriptb = false;
	}
	else
	{
		did_scriptb=false;
	}
	
	if(attackclk || action==attacking || action==sideswimattacking)
	{
		
		if((attackclk==0) && action!=sideswimattacking && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
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
		
		bool attacked = doattack();
		
		// This section below interferes with script-setting Hero->Dir, so it comes after doattack
		if(!inlikelike && attackclk>4 && (attackclk&3)==0 && charging==0 && spins==0 && action!=sideswimattacking)
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
		
		if(attacked && (charging==0 && spins<=5) && jumping<1 && action!=sideswimattacking)
		{
			return;
		}
		else if(!attacked)
		{
			// Spin attack - change direction
			if(spins>1 && attack != wHammer)
			{
				spins--;
				
				if(spins%5==0)
				{
					int id = currentscroll > -1 ? currentscroll : (current_item_id(spins>5 ? itype_spinscroll2 : itype_spinscroll));
					sfx(itemsbuf[id].usesound,pan(x.getInt()));
				}
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
			
			if (IsSideSwim()) {action=sideswimming; FFCore.setHeroAction(sideswimming);}
			else {action=none; FFCore.setHeroAction(none);}
			attackclk=0;
			charging=0;
		}
	}
	
	if(pitslide()) //Check pit's 'pull'. If true, then Hero cannot fight the pull.
		return;
	
	if(action==walking) //still walking
	{
		if(!DrunkUp() && !DrunkDown() && !DrunkLeft() && !DrunkRight() && !autostep)
		{
			if(attackclk>0) SetAttack();
			else {action = none; FFCore.setHeroAction(none);}
			hero_count=-1;
			return;
		}
		
		autostep=false;
		
		if(!(diagonalMovement || NO_GRIDLOCK))
		{
			if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
			{
				if(dir==up&&yoff)
				{
					info = walkflag(x,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]),2,up);
					info = info || walkflagMBlock(x+8,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]));
					execute(info);
					
					if(!info.isUnwalkable())
					{
						moveOld2(up);
					}
					else
					{
						action=none; FFCore.setHeroAction(none);
					}
					
					return;
				}
				
				if(dir==down&&yoff)
				{
					info = walkflag(x,y+15+int32_t(lsteps[y.getInt()&7]),2,down);
					info = info || walkflagMBlock(x+8,y+15+int32_t(lsteps[y.getInt()&7]));
					execute(info);
					
					if(!info.isUnwalkable())
					{
						moveOld2(down);
					}
					else
					{
						action=none; FFCore.setHeroAction(none);
					}
					
					return;
				}
				
				if(dir==left&&xoff)
				{
					info = walkflag(x-int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,left) || walkflag(x-int32_t(lsteps[x.getInt()&7]),y+8,1,left);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						moveOld2(left);
					}
					else
					{
						action=none; FFCore.setHeroAction(none);
					}
					
					return;
				}
				
				if(dir==right&&xoff)
				{
					info = walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,right) || walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+8,1,right);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						moveOld2(right);
					}
					else
					{
						action=none; FFCore.setHeroAction(none);
					}
					
					return;
				}
			}
			else
			{
				if(dir==up&&yoff)
				{
					while(true)
					{
						info = walkflag(temp_x,temp_y+(bigHitbox?0:8)-temp_step,2,up);
						info = info || walkflagMBlock(temp_x+8,temp_y+(bigHitbox?0:8)-temp_step);
						execute(info);
						
						if(!info.isUnwalkable())
						{
							hero_newstep = temp_step;
							x = temp_x;
							y = temp_y;
							moveOld2(up);
							return;
						}
						//Could not move, try moving less
						if(temp_y != int32_t(temp_y))
						{
							temp_y = floor((double)temp_y);
							continue;
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
							continue;
						}
						else //Can't move less, stop moving
						{
							action=none; FFCore.setHeroAction(none);
						}
						return;
					}
				}
				
				if(dir==down&&yoff)
				{
					while(true)
					{
						info = walkflag(temp_x,temp_y+15+temp_step,2,down);
						info = info || walkflagMBlock(temp_x+8,temp_y+15+temp_step);
						execute(info);
						
						if(!info.isUnwalkable())
						{
							hero_newstep = temp_step;
							x = temp_x;
							y = temp_y;
							moveOld2(down);
							return;
						}
						//Could not move, try moving less
						if(temp_y != int32_t(temp_y))
						{
							temp_y = floor((double)temp_y);
							continue;
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
							continue;
						}
						else //Can't move less, stop moving
						{
							action=none; FFCore.setHeroAction(none);
						}
						return;
					}
				}
				
				if(dir==left&&xoff)
				{
					while(true)
					{
						info = walkflag(temp_x-temp_step,temp_y+(bigHitbox?0:8),1,left) || walkflag(temp_x-temp_step,temp_y+8,1,left);
						execute(info);
						
						if(!info.isUnwalkable())
						{
							hero_newstep = temp_step;
							x = temp_x;
							y = temp_y;
							moveOld2(left);
							return;
						}
						//Could not move, try moving less
						if(temp_x != int32_t(temp_x))
						{
							temp_x = floor((double)temp_x);
							continue;
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
							continue;
						}
						else //Can't move less, stop moving
						{
							action=none; FFCore.setHeroAction(none);
						}
						return;
					}
				}
				
				if(dir==right&&xoff)
				{
					while(true)
					{
						info = walkflag(temp_x+15+temp_step,temp_y+(bigHitbox?0:8),1,right) || walkflag(temp_x+15+temp_step,temp_y+8,1,right);
						execute(info);
						
						if(!info.isUnwalkable())
						{
							hero_newstep = temp_step;
							x = temp_x;
							y = temp_y;
							moveOld2(right);
							return;
						}
						//Could not move, try moving less
						if(temp_x != int32_t(temp_x))
						{
							temp_x = floor((double)temp_x);
							continue;
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
							continue;
						}
						else //Can't move less, stop moving
						{
							action=none; FFCore.setHeroAction(none);
						}
						return;
					}
				}
			}
		}
	
	} // endif (action==walking)
	
	if((action!=swimming)&&(action!=sideswimming)&&(action !=sideswimhit)&&(action !=sideswimattacking)&&(action!=casting)&&(action!=sideswimcasting)&&(action!=drowning)&&(action!=sidedrowning)&&(action!=lavadrowning) && charging==0 && spins==0 && jumping<1)
	{
		action=none; FFCore.setHeroAction(none);
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
		
		if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim()) //!DIRECTION SET
		{
			walkable = false;
			if(DrunkUp()&&(holddir==-1||holddir==up))
			{
				if(isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
					{
						dir=up;
					}
					
					holddir=up;
					
					if(DrunkRight()&&shiftdir!=left)
					{
						shiftdir=right;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
					}
					else if(DrunkLeft()&&shiftdir!=right)
					{
						shiftdir=left;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = left;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = left;
					}
					else
					{
						shiftdir=-1;
					}
					
					//walkable if Ladder can be placed or is already placed vertically
					if(isSideViewHero() && !toogam && (!get_qr(qr_OLD_LADDER_ITEM_SIDEVIEW) || !(can_deploy_ladder() || (ladderx && laddery && ladderdir==up))) && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						do
						{
							zfix ty = y - hero_newstep;
							info = walkflag(x,(bigHitbox?0:8) + ty,2,up)
								|| walkflag(x+15,(bigHitbox?0:8) + ty,1,up);
							
							if (ty < 0 && !bigHitbox) //sanity check for up scroll
							{
								info = info || walkflag(x, 0_zf, 2, up);
								info = info || walkflag(x+15, 0_zf, 1, up);
							}
							info = info || walkflagMBlock(x+15, (bigHitbox?0:8) + ty);
								
							execute(info);
							
							if(info.isUnwalkable())
							{
								if(y != y.getInt())
								{
									y.doRound();
								}
								else if(hero_newstep > 1)
								{
									if(hero_newstep != int32_t(hero_newstep)) //floor
										hero_newstep = floor((double)hero_newstep);
									else --hero_newstep;
								}
								else
									break;
							}
							else walkable = true;
						}
						while(!walkable);
					}
					
					int32_t s=shiftdir;
					
					if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM))
					{
						shiftdir=-1;
					}
					else
					{
						if(s==left)
						{
							do
							{
								info = (walkflag(x-hero_newstep_diag,y+(bigHitbox?0:8),1,left)||walkflag(x-hero_newstep_diag,y+15,1,left));
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(x != x.getInt())
									{
										x.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x-hero_newstep_diag,(bigHitbox?0:8)+(y-hero_newstep),1,left);
										execute(info);
										if(info.isUnwalkable())
										{
											if(x != x.getInt())
											{
												x.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
						else if(s==right)
						{
							do
							{
								info = (walkflag(x+15+hero_newstep_diag,y+(bigHitbox?0:8),1,right)||walkflag(x+15+hero_newstep_diag,y+15,1,right));
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(x != x.getInt())
									{
										x.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x+15+hero_newstep_diag,(bigHitbox?0:8)+(y-hero_newstep),1,right);
										execute(info);
										if(info.isUnwalkable())
										{
											if(x != x.getInt())
											{
												x.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
					}
					
					moveOld2(up);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							x = x.getInt();
							y = y.getInt();
							if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
									!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
									_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else
							{
								if(_walkflag(x,   y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
										!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
										!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
										sprite::move((zfix)1,(zfix)0);
								}
								else
								{
									pushing=push+1;
								}
							}
						}
						else
						{
							pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
						}
					}
					
					return;
				}
			}
			
			if(DrunkDown()&&(holddir==-1||holddir==down))
			{
				if(isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
					{
						dir=down;
					}
					
					holddir=down;
					
					if(DrunkRight()&&shiftdir!=left)
					{
						shiftdir=right;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
					}
					else if(DrunkLeft()&&shiftdir!=right)
					{
						shiftdir=left;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = left;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = left;
					}
					else
					{
						shiftdir=-1;
					}
					
					//bool walkable;
					if(isSideViewHero() && !toogam && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						do
						{
							info = walkflag(x,15+(y+hero_newstep),2,down);
							
							if(x.getFloor() & 7)
								info = info || walkflag(x+15,15+(y+hero_newstep),1,down);
							else
								info = info || walkflagMBlock(x+15, 15+(y+hero_newstep));
								
							execute(info);
							
							if(info.isUnwalkable())
							{
								if(y != y.getInt())
								{
									y.doRound();
								}
								else if(hero_newstep > 1)
								{
									if(hero_newstep != int32_t(hero_newstep)) //floor
										hero_newstep = floor((double)hero_newstep);
									else --hero_newstep;
								}
								else
									break;
							}
							else walkable = true;
						}
						while(!walkable);
					}
					
					int32_t s=shiftdir;
					
					if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM))
					{
						shiftdir=-1;
					}
					else
					{
						if(s==left)
						{
							do
							{
								info = (walkflag(x-hero_newstep_diag,y+(bigHitbox?0:8),1,left)||walkflag(x-hero_newstep_diag,y+15,1,left));
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(x != x.getInt())
									{
										x.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x-hero_newstep_diag,15+(y+hero_newstep),1,left);
										execute(info);
										if(info.isUnwalkable())
										{
											if(x != x.getInt())
											{
												x.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
						else if(s==right)
						{
							do
							{
								info = (walkflag(x+15+hero_newstep_diag,y+(bigHitbox?0:8),1,right)||walkflag(x+15+hero_newstep_diag,y+15,1,right));
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(x != x.getInt())
									{
										x.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x+15+hero_newstep_diag,15+(y+hero_newstep),1,right);
										execute(info);
										if(info.isUnwalkable())
										{
											if(x != x.getInt())
											{
												x.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
					}
					
					moveOld2(down);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							x = x.getInt();
							y = y.getInt();
							if(!_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+8, y+15+1,1,SWITCHBLOCK_STATE)&&
									_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else if(_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+7, y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
									sprite::move((zfix)1,(zfix)0);
							}
							else
							{
								pushing=push+1;
							}
						}
						else
						{
							pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
						}
					}
					
					return;
				}
			}
			
			if(DrunkLeft()&&(holddir==-1||holddir==left))
			{
				if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking)
					{
						dir=left;
					}
					sideswimdir = left;
					
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
					
					do
					{
						info = walkflag(x-hero_newstep,y+(bigHitbox?0:8),1,left)||walkflag(x-hero_newstep,y+8,1,left);
					
						info = info || walkflag(x-hero_newstep,y+15,1,left);
							
						execute(info);
						
						if(info.isUnwalkable())
						{
							if(x != x.getInt())
							{
								x.doRound();
							}
							else if(hero_newstep > 1)
							{
								if(hero_newstep != int32_t(hero_newstep)) //floor
									hero_newstep = floor((double)hero_newstep);
								else --hero_newstep;
							}
							else
								break;
						}
						else walkable = true;
					}
					while(!walkable);
					
					int32_t s=shiftdir;
					
					if((isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
					{
						shiftdir=-1;
					}
					else
					{
						if(s==up)
						{
							do
							{
								zfix ty = y - hero_newstep_diag;
								info = walkflag(x,(bigHitbox?0:8) + ty,2,up)
									|| walkflag(x+15,(bigHitbox?0:8) + ty,1,up);
								
								if (ty < 0 && !bigHitbox) //sanity check for up scroll
								{
									info = info || walkflag(x, 0_zf, 2, up);
									info = info || walkflag(x+15, 0_zf, 1, up);
								}
								info = info || walkflagMBlock(x+15, (bigHitbox?0:8) + ty);
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(y != y.getInt())
									{
										y.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										zfix tx = x-hero_newstep, ty = y-hero_newstep_diag;
										info = walkflag(tx,(bigHitbox?0:8)+ty,1,up);
								
										if (ty < 0 && !bigHitbox) //sanity check for up scroll
										{
											info = info || walkflag(tx, 0_zf, 1, up);
											info = info || walkflag(tx+15, 0_zf, 1, up);
										}
										info = info || walkflagMBlock(tx+15, (bigHitbox?0:8) + ty);
										
										execute(info);
										if(info.isUnwalkable())
										{
											if(y != y.getInt())
											{
												y.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
						else if(s==down)
						{
							do
							{
								info = walkflag(x,y+15+hero_newstep_diag,2,down)||walkflag(x+15,y+15+hero_newstep_diag,1,down);
								
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(y != y.getInt())
									{
										y.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x-hero_newstep,y+15+hero_newstep_diag,1,down);
										execute(info);
										if(info.isUnwalkable())
										{
											if(y != y.getInt())
											{
												y.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
					}
					
					moveOld2(left);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							x = x.getInt();
							y = y.getInt();
							int32_t v1=bigHitbox?0:8;
							int32_t v2=bigHitbox?8:12;
							
							if(!_walkflag(x-1,y+v1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+v2,1,SWITCHBLOCK_STATE)&&
									_walkflag(x-1,y+15,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x-1,y+v1,  1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+v2-1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+15,  1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
									sprite::move((zfix)0,(zfix)1);
							}
							else
							{
								pushing=push+1;
							}
						}
						else
						{
							pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
							
							if(action!=swimming)
							{
							}
						}
					}
					
					return;
				}
			}
			
			if(DrunkRight()&&(holddir==-1||holddir==right))
			{
				if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking)
					{
						dir=right;
					}
					sideswimdir = right;
					
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
					
					do
					{
						info = walkflag(x+15+hero_newstep,y+(bigHitbox?0:8),1,right)||walkflag(x+15+hero_newstep,y+8,1,right);;
						
						info = info || walkflag(x+15+hero_newstep,y+15,1,right);
							
						execute(info);
						
						if(info.isUnwalkable())
						{
							if(x != x.getInt())
							{
								x.doRound();
							}
							else if(hero_newstep > 1)
							{
								if(hero_newstep != int32_t(hero_newstep)) //floor
									hero_newstep = floor((double)hero_newstep);
								else --hero_newstep;
							}
							else
								break;
						}
						else walkable = true;
					}
					while(!walkable);
					
					int32_t s=shiftdir;
					
					if((isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
					{
						shiftdir=-1;
					}
					else
					{
						if(s==up)
						{
							do
							{
								zfix ty = y - hero_newstep_diag;
								info = walkflag(x,(bigHitbox?0:8) + ty,2,up)
									|| walkflag(x+15,(bigHitbox?0:8) + ty,1,up);
								
								if (ty < 0 && !bigHitbox) //sanity check for up scroll
								{
									info = info || walkflag(x, 0_zf, 2, up);
									info = info || walkflag(x+15, 0_zf, 1, up);
								}
								info = info || walkflagMBlock(x+15, (bigHitbox?0:8) + ty);
								
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(y != y.getInt())
									{
										y.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										zfix tx = x+hero_newstep, ty = y-hero_newstep_diag;
										info = walkflag(tx+15,(bigHitbox?0:8)+ty,1,up);
								
										if (ty < 0 && !bigHitbox) //sanity check for up scroll
										{
											info = info || walkflag(tx, 0_zf, 1, up);
											info = info || walkflag(tx+15, 0_zf, 1, up);
										}
										info = info || walkflagMBlock(tx+15, (bigHitbox?0:8) + ty);
										execute(info);
										if(info.isUnwalkable())
										{
											if(y != y.getInt())
											{
												y.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
						else if(s==down)
						{
							do
							{
								info = walkflag(x,y+15+hero_newstep_diag,2,down)||walkflag(x+15,y+15+hero_newstep_diag,1,down);
									
								execute(info);
								
								if(info.isUnwalkable())
								{
									if(y != y.getInt())
									{
										y.doRound();
									}
									else if(hero_newstep_diag > 1)
									{
										if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
											hero_newstep_diag.doFloor();
										else --hero_newstep_diag;
									}
									else
										shiftdir = -1;
								}
								else if(walkable)
								{
									do
									{
										info = walkflag(x+15+hero_newstep,y+15+hero_newstep_diag,1,down);
										execute(info);
										if(info.isUnwalkable())
										{
											if(y != y.getInt())
											{
												y.doRound();
											}
											else if(hero_newstep_diag > 1)
											{
												if(hero_newstep_diag != hero_newstep_diag.getInt()) //floor
													hero_newstep_diag.doFloor();
												else --hero_newstep_diag;
											}
											else
												shiftdir = -1;
										}
										else break;
									}
									while(shiftdir != -1);
									break;
								}
								else break;
							}
							while(shiftdir != -1);
						}
					}
					
					moveOld2(right);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							x = x.getInt();
							y = y.getInt();
							int32_t v1=bigHitbox?0:8;
							int32_t v2=bigHitbox?8:12;
								   
							if(!_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
								   !_walkflag(x+16,y+v2,1,SWITCHBLOCK_STATE)&&
								   _walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
									   !_walkflag(x+16,y+v2-1,1,SWITCHBLOCK_STATE)&&
									   !_walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
									sprite::move((zfix)0,(zfix)1);
							}
							else
							{
								pushing=push+1;
								z3step=2;
							}
						}
						else
						{
							pushing=push+1; // L: This makes solid damage combos and diagonal-triggered Armoses work.
							
							if(action!=swimming)
							{
							}
						}
					}
					
					return;
				}
			}
		}
		else
		{
			if(DrunkUp()&&(holddir==-1||holddir==up))
			{
				if(isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam)
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
					if(isSideViewHero() && !toogam && !(can_deploy_ladder() || (ladderx && laddery && ladderdir==up)) && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						info = walkflag(x,y+(bigHitbox?0:8)-z3step,2,up);
						
						if(x.getInt() & 7)
							info = info || walkflag(x+16,y+(bigHitbox?0:8)-z3step,1,up);
						else
							info = info || walkflagMBlock(x+16, y+(bigHitbox?0:8)-z3step);
							
						execute(info);
						
						if(info.isUnwalkable())
						{
							if(z3step==2)
							{
								z3step=1;
								info = walkflag(x,y+(bigHitbox?0:8)-z3step,2,up);
								
								if(x.getInt()&7)
									info = info || walkflag(x+16,y+(bigHitbox?0:8)-z3step,1,up);
								else
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
					
					int32_t s=shiftdir;
					
					if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM))
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
					
					moveOld2(up);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							if(!_walkflag(x,   y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
									!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
									_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else
							{
								if(_walkflag(x,   y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
										!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
										!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
										sprite::move((zfix)1,(zfix)0);
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
				if(isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam)
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
					if(isSideViewHero() && !toogam && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						info = walkflag(x,y+15+z3step,2,down);
						
						if(x.getInt()&7)
							info = info || walkflag(x+16,y+15+z3step,1,down);
						else
							info = info || walkflagMBlock(x+16, y+15+z3step);
						
						execute(info);
						
						if(info.isUnwalkable())
						{
							if(z3step==2)
							{
								z3step=1;
								info = walkflag(x,y+15+z3step,2,down);
								
								if(x.getInt()&7)
									info = info || walkflag(x+16,y+15+z3step,1,down);
								else
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
					
					int32_t s=shiftdir;
					
					if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM))
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
					
					moveOld2(down);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							if(!_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+8, y+15+1,1,SWITCHBLOCK_STATE)&&
									_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else if(_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+7, y+15+1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
									sprite::move((zfix)1,(zfix)0);
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
				if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam)
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
					
					if(y.getInt()&7)
						info = info || walkflag(x-z3step,y+16,1,left);
						
					execute(info);
					
					if(info.isUnwalkable())
					{
						if(z3step==2)
						{
							z3step=1;
							info = walkflag(x-z3step,y+(bigHitbox?0:8),1,left)||walkflag(x-z3step,y+8,1,left);
							
							if(y.getInt()&7)
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
					
					int32_t s=shiftdir;
					
					if((isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
					
					moveOld2(left);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							int32_t v1=bigHitbox?0:8;
							int32_t v2=bigHitbox?8:12;
							
							if(!_walkflag(x-1,y+v1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+v2,1,SWITCHBLOCK_STATE)&&
									_walkflag(x-1,y+15,1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x-1,y+v1,  1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+v2-1,1,SWITCHBLOCK_STATE)&&
									!_walkflag(x-1,y+15,  1,SWITCHBLOCK_STATE))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
									sprite::move((zfix)0,(zfix)1);
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
				if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam)
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
					
					if(y.getInt()&7)
						info = info || walkflag(x+15+z3step,y+16,1,right);
						
					execute(info);
					
					if(info.isUnwalkable())
					{
						if(z3step==2)
						{
							z3step=1;
							info = walkflag(x+15+z3step,y+(bigHitbox?0:8),1,right)||walkflag(x+15+z3step,y+8,1,right);
							
							if(y.getInt()&7)
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
					
					int32_t s=shiftdir;
					
					if((isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
					
					moveOld2(right);
					shiftdir=s;
					
					if(!walkable)
					{
						if(shiftdir==-1) //Corner-shove; prevent being stuck on corners -V
						{
							int32_t v1=bigHitbox?0:8;
							int32_t v2=bigHitbox?8:12;
							
							info = !walkflag(x+16,y+v1,1,right)&&
								   !walkflag(x+16,y+v2,1,right)&&
								   walkflag(x+16,y+15,1,right);
								   
							//do NOT execute these
							if(info.isUnwalkable())
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else
							{
								info = walkflag(x+16,y+v1,  1,right)&&
									   !walkflag(x+16,y+v2-1,1,right)&&
									   !walkflag(x+16,y+15,  1,right);
									   
								if(info.isUnwalkable())
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
										sprite::move((zfix)0,(zfix)1);
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
		}
		if(shield_forcedir > -1 && action != rafting)
			dir = shield_forcedir;
		int32_t wtry  = iswaterex_z3(MAPCOMBO(x,y+15), -1, x,y+15, true, false);
		int32_t wtry8 = iswaterex_z3(MAPCOMBO(x+15,y+15), -1, x+15,y+15, true, false);
		int32_t wtrx = iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)), -1, x,y+(bigHitbox?0:8), true, false);
		int32_t wtrx8 = iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)), -1, x+15,y+(bigHitbox?0:8), true, false);
		int32_t wtrc = iswaterex_z3(MAPCOMBO(x+8,y+(bigHitbox?8:12)), -1, x+8,y+(bigHitbox?8:12), true, false);
		
		if(can_use_item(itype_flippers,i_flippers)&&current_item(itype_flippers) >= combobuf[wtrc].attribytes[0]&&(!(combobuf[wtrc].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))&&!(ladderx+laddery)&&z==0&&fakez==0)
		{
			if(wtrx&&wtrx8&&wtry&&wtry8 && !DRIEDLAKE)
			{
				//action=swimming;
				if(action !=none && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && !isSideViewHero())
				{
					hopclk = 0xFF;
				}
			}
		}
		
		return;
	} //endif (LTTPWALK)
	temp_step = hero_newstep;
	temp_x = x;
	temp_y = y;
	
	if(isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam)
	{
		if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
			goto LEFTRIGHT_NEWMOVE;
		else goto LEFTRIGHT_OLDMOVE;
	}
	
	// make it easier to get in left & right doors
	
	//ignore ladder for this part. sigh sigh sigh -DD
	oldladderx = ladderx;
	oldladdery = laddery;
	if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
	{
		if(isdungeon() && DrunkLeft() && (temp_x==32 && temp_y==80))
		{
			do
			{
				info = walkflag(temp_x,temp_y+(bigHitbox?0:8),1,left) ||
					   walkflag(temp_x-temp_step,temp_y+(bigHitbox?0:8),1,left);
				
				if(info.isUnwalkable())
				{
					if(temp_x != int32_t(temp_x))
					{
						temp_x = floor((double)temp_x);
					}
					else if(temp_step > 1)
					{
						if(temp_step != int32_t(temp_step)) //floor
							temp_step = floor((double)temp_step);
						else --temp_step;
					}
					else
						break;
				}
			}
			while(info.isUnwalkable());
			
			if(!info.isUnwalkable())
			{
				x = temp_x;
				y = temp_y;
				hero_newstep = temp_step;
				//ONLY process the side-effects of the above walkflag if Hero will actually move
				//sigh sigh sigh... walkflag is a horrible mess :-/ -DD
				execute(info);
				moveOld2(left);
				return;
			}
			temp_x = x;
			temp_y = y;
			temp_step = hero_newstep;
		}
		
		if(isdungeon() && DrunkRight() && temp_x==208 && temp_y==80)
		{
			do
			{
				info = walkflag(temp_x+15+temp_step,temp_y+(bigHitbox?0:8),1,right) ||
				   walkflag(temp_x+15+temp_step,temp_y+8,1,right);
				
				if(info.isUnwalkable())
				{
					if(temp_x != int32_t(temp_x))
					{
						temp_x = floor((double)temp_x);
					}
					else if(temp_step > 1)
					{
						if(temp_step != int32_t(temp_step)) //floor
							temp_step = floor((double)temp_step);
						else --temp_step;
					}
					else
						break;
				}
			}
			while(info.isUnwalkable());
			
			if(!info.isUnwalkable())
			{
				x = temp_x;
				y = temp_y;
				hero_newstep = temp_step;
				execute(info);
				moveOld2(right);
				return;
			}
			temp_x = x;
			temp_y = y;
			temp_step = hero_newstep;
		}
		
		ladderx = oldladderx;
		laddery = oldladdery;
		
		if(DrunkUp())
		{
			if(xoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=up && dir!=down)
				{
					if(xoff>2&&xoff<6)
					{
						moveOld2(dir);
					}
					else if(xoff>=6)
					{
						moveOld2(right);
					}
					else if(xoff>=1)
					{
						moveOld2(left);
					}
				}
				else
				{
					if(xoff>=4)
					{
						moveOld2(right);
					}
					else if(xoff<4)
					{
						moveOld2(left);
					}
				}
			}
			else
			{
				do
				{
					if(action==swimming || IsSideSwim() || action == swimhit)
					{
						info = walkflag(temp_x,temp_y+(bigHitbox?0:8)-temp_step,2,up);
						
						if(_walkflag(temp_x+15, temp_y+(bigHitbox?0:8)-temp_step, 1,SWITCHBLOCK_STATE) &&
								!(iswaterex_z3(MAPCOMBO(temp_x, temp_y+(bigHitbox?0:8)-temp_step), -1, temp_x, temp_y+(bigHitbox?0:8)-temp_step, true, false) &&
								  iswaterex_z3(MAPCOMBO(temp_x+15, temp_y+(bigHitbox?0:8)-temp_step), -1, temp_x+15, temp_y+(bigHitbox?0:8)-temp_step, true, false)))
							info.setUnwalkable(true);
					}
					else
					{
						info = walkflag(temp_x,temp_y+(bigHitbox?0:8)-temp_step,2,up);
						if(x.getInt() & 7)
							info = info || walkflag(temp_x+16,temp_y+(bigHitbox?0:8)-temp_step,1,up);
						else
							info = info || walkflagMBlock(temp_x+8,temp_y+(bigHitbox?0:8)-temp_step);
					}
					
					if(info.isUnwalkable())
					{
						if(temp_y != int32_t(temp_y))
						{
							temp_y = floor((double)temp_y);
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
						}
						else
							break;
					}
				}
				while(info.isUnwalkable());
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					x = temp_x;
					y = temp_y;
					hero_newstep = temp_step;
					moveOld2(up);
					return;
				}
				
				if(!DrunkLeft() && !DrunkRight())
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)1,(zfix)0);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=up;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
				else
				{
					goto LEFTRIGHT_NEWMOVE;
				}
			}
			
			return;
		}
		
		if(DrunkDown())
		{
			if(xoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=up && dir!=down)
				{
					if(xoff>2&&xoff<6)
					{
						moveOld2(dir);
					}
					else if(xoff>=6)
					{
						moveOld2(right);
					}
					else if(xoff>=1)
					{
						moveOld2(left);
					}
				}
				else
				{
					if(xoff>=4)
					{
						moveOld2(right);
					}
					else if(xoff<4)
					{
						moveOld2(left);
					}
				}
			}
			else
			{
				do
				{
					if(action==swimming || IsSideSwim() || action == swimhit)
					{
						info=walkflag(temp_x,temp_y+15+temp_step,2,down);
						
						if(_walkflag(temp_x+15, temp_y+15+temp_step, 1,SWITCHBLOCK_STATE) &&
								!(iswaterex_z3(MAPCOMBO(temp_x, temp_y+15+temp_step), -1, temp_x, temp_y+15+temp_step, true, false) &&
								  iswaterex_z3(MAPCOMBO(temp_x+15, temp_y+15+temp_step), -1, temp_x+15, temp_y+15+temp_step, true, false)))
							info.setUnwalkable(true);
					}
					else
					{
						info=walkflag(temp_x,temp_y+15+temp_step,2,down);
						if(x.getInt() & 7)
							info = info || walkflag(temp_x+16,temp_y+15+temp_step,1,down);
						else
							 info = info || walkflagMBlock(temp_x+8,temp_y+15+temp_step);
					}
					
					if(info.isUnwalkable())
					{
						if(temp_y != int32_t(temp_y))
						{
							temp_y = floor((double)temp_y);
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
						}
						else
							break;
					}
				}
				while(info.isUnwalkable());
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					x = temp_x;
					y = temp_y;
					hero_newstep = temp_step;
					moveOld2(down);
					return;
				}
				
				if(!DrunkLeft() && !DrunkRight())
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						if(!_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+8, y+15+1,1,SWITCHBLOCK_STATE)&&
								_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+7, y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
								sprite::move((zfix)1,(zfix)0);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=down;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
				else goto LEFTRIGHT_NEWMOVE;
			}
			
			return;
		}
		
LEFTRIGHT_NEWMOVE:
		temp_x = x;
		temp_y = y;
		temp_step = hero_newstep;
		if(isdungeon() && (temp_y<=26 || temp_y>=134) && !get_qr(qr_FREEFORM) && !toogam)
		{
			return;
		}
		
		if(DrunkLeft())
		{
			if(yoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=left && dir!=right)
				{
					if(yoff>2&&yoff<6)
					{
						moveOld2(dir);
					}
					else if(yoff>=6)
					{
						moveOld2(down);
					}
					else if(yoff>=1)
					{
						moveOld2(up);
					}
				}
				else
				{
					if(yoff>=4)
					{
						moveOld2(down);
					}
					else if(yoff<4)
					{
						moveOld2(up);
					}
				}
			}
			else
			{
				do
				{
					info = walkflag(temp_x-temp_step,temp_y+(bigHitbox?0:8),1,left) ||
						   walkflag(temp_x-temp_step,temp_y+(isSideViewHero() ?0:8), 1,left);
					   
					if(y.getInt() & 7)
						info = info || walkflag(temp_x-temp_step,temp_y+16,1,left);
					
					if(info.isUnwalkable())
					{
						if(temp_x != int32_t(temp_x))
						{
							temp_x = floor((double)temp_x);
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
						}
						else
							break;
					}
				}
				while(info.isUnwalkable());
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					x = temp_x;
					y = temp_y;
					hero_newstep = temp_step;
					moveOld2(left);
					return;
				}
				
				if(!DrunkUp() && !DrunkDown())
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
						
						if(!_walkflag(x-1,y+v1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+v2,1,SWITCHBLOCK_STATE)&&
								_walkflag(x-1,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x-1,y+v1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+v2-1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+15,  1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
								sprite::move((zfix)0,(zfix)1);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=left;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
			}
			
			return;
		}
		
		if(DrunkRight())
		{
			if(yoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=left && dir!=right)
				{
					if(yoff>2&&yoff<6)
					{
						moveOld2(dir);
					}
					else if(yoff>=6)
					{
						moveOld2(down);
					}
					else if(yoff>=1)
					{
						moveOld2(up);
					}
				}
				else
				{
					if(yoff>=4)
					{
						moveOld2(down);
					}
					else if(yoff<4)
					{
						moveOld2(up);
					}
				}
			}
			else
			{
				do
				{
					info = walkflag(temp_x+15+temp_step,temp_y+(bigHitbox?0:8),1,right) || 
							walkflag(temp_x+15+temp_step,temp_y+(isSideViewHero() ?0:8),1,right);
					
					if(y.getInt() & 7)
						info = info || walkflag(temp_x+15+temp_step,y+16,1,right);
					
					if(info.isUnwalkable())
					{
						if(temp_x != int32_t(temp_x))
						{
							temp_x = floor((double)temp_x);
						}
						else if(temp_step > 1)
						{
							if(temp_step != int32_t(temp_step)) //floor
								temp_step = floor((double)temp_step);
							else --temp_step;
						}
						else
							break;
					}
				}
				while(info.isUnwalkable());
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					x = temp_x;
					y = temp_y;
					hero_newstep = temp_step;
					moveOld2(right);
					return;
				}
				
				if(!DrunkUp() && !DrunkDown())
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
							   
						if(!_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
							   !_walkflag(x+16,y+v2,1,SWITCHBLOCK_STATE)&&
							   _walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
								   !_walkflag(x+16,y+v2-1,1,SWITCHBLOCK_STATE)&&
								   !_walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
								sprite::move((zfix)0,(zfix)1);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=right;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
			}
		}
	}
	else
	{
		info = walkflag(x-int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,left) ||
			   walkflag(x-int32_t(lsteps[x.getInt()&7]),y+8,1,left);
		
		if(isdungeon() && DrunkLeft() && !info.isUnwalkable() && (x==32 && y==80))
		{
			//ONLY process the side-effects of the above walkflag if Hero will actually move
			//sigh sigh sigh... walkflag is a horrible mess :-/ -DD
			execute(info);
			moveOld2(left);
			return;
		}
		
		info = walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,right) ||
			   walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+8,1,right);
		
		if(isdungeon() && DrunkRight() && !info.isUnwalkable() && x==208 && y==80)
		{
			execute(info);
			moveOld2(right);
			return;
		}
		
		ladderx = oldladderx;
		laddery = oldladdery;
		
		if(DrunkUp())
		{
			if(xoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=up && dir!=down)
				{
					if(xoff>2&&xoff<6)
					{
						moveOld2(dir);
					}
					else if(xoff>=6)
					{
						moveOld2(right);
					}
					else if(xoff>=1)
					{
						moveOld2(left);
					}
				}
				else
				{
					if(xoff>=4)
					{
						moveOld2(right);
					}
					else if(xoff<4)
					{
						moveOld2(left);
					}
				}
			}
			else
			{
				if(action==swimming || IsSideSwim() || action == swimhit)
				{
					info = walkflag(x,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]),2,up);
					
					if(_walkflag(x+15, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]), 1,SWITCHBLOCK_STATE) &&
							!(iswaterex_z3(MAPCOMBO(x, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7])), -1, x, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7])) &&
							  iswaterex_z3(MAPCOMBO(x+15, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7])), -1, x+15, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]))))
						info.setUnwalkable(true);
				}
				else
				{
					info = walkflag(x,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]),2,up);
					if(x.getInt() & 7)
						info = info || walkflag(x+16,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]),1,up);
					else
						info = info || walkflagMBlock(x+8,y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]));
				}
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					moveOld2(up);
					return;
				}
				
				if(!DrunkLeft() && !DrunkRight())
				{
					if(NO_GRIDLOCK)
					{
						if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
								!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)1,(zfix)0);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=up;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
				else
				{
					goto LEFTRIGHT_OLDMOVE;
				}
			}
			
			return;
		}
		
		if(DrunkDown())
		{
			if(xoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=up && dir!=down)
				{
					if(xoff>2&&xoff<6)
					{
						moveOld2(dir);
					}
					else if(xoff>=6)
					{
						moveOld2(right);
					}
					else if(xoff>=1)
					{
						moveOld2(left);
					}
				}
				else
				{
					if(xoff>=4)
					{
						moveOld2(right);
					}
					else if(xoff<4)
					{
						moveOld2(left);
					}
				}
			}
			else
			{
				if(action==swimming || IsSideSwim() || action == swimhit)
				{
					info=walkflag(x,y+15+int32_t(lsteps[y.getInt()&7]),2,down);
					
					if(_walkflag(x+15, y+15+int32_t(lsteps[y.getInt()&7]), 1,SWITCHBLOCK_STATE) &&
							!(iswaterex_z3(MAPCOMBO(x, y+15+int32_t(lsteps[y.getInt()&7])), -1, x, y+15+int32_t(lsteps[y.getInt()&7])) &&
							  iswaterex_z3(MAPCOMBO(x+15, y+15+int32_t(lsteps[y.getInt()&7])), -1, x+15, y+15+int32_t(lsteps[y.getInt()&7]))))
						info.setUnwalkable(true);
				}
				else
				{
					info=walkflag(x,y+15+int32_t(lsteps[y.getInt()&7]),2,down);
					if(x.getInt() & 7)
						info = (info || walkflag(x+16,y+15+int32_t(lsteps[y.getInt()&7]),1,down));
					else
						 info = (info || walkflagMBlock(x+8,y+15+int32_t(lsteps[y.getInt()&7])));
				}
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					moveOld2(down);
					return;
				}
				
				if(!DrunkLeft() && !DrunkRight())
				{
					if(NO_GRIDLOCK)
					{
						if(!_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+8, y+15+1,1,SWITCHBLOCK_STATE)&&
								_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,   y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+7, y+15+1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x+15,y+15+1,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
								sprite::move((zfix)1,(zfix)0);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=down;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
				else goto LEFTRIGHT_OLDMOVE;
			}
			
			return;
		}
		
LEFTRIGHT_OLDMOVE:

		if(isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam)
		{
			return;
		}
		
		if(DrunkLeft())
		{
			if(yoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=left && dir!=right)
				{
					if(yoff>2&&yoff<6)
					{
						moveOld2(dir);
					}
					else if(yoff>=6)
					{
						moveOld2(down);
					}
					else if(yoff>=1)
					{
						moveOld2(up);
					}
				}
				else
				{
					if(yoff>=4)
					{
						moveOld2(down);
					}
					else if(yoff<4)
					{
						moveOld2(up);
					}
				}
			}
			else
			{
				info = walkflag(x-int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,left) ||
					   walkflag(x-int32_t(lsteps[x.getInt()&7]),y+(isSideViewHero() ?0:8), 1,left);
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					moveOld2(left);
					return;
				}
				
				if(!DrunkUp() && !DrunkDown())
				{
					if(NO_GRIDLOCK)
					{
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
						
						if(!_walkflag(x-1,y+v1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+v2,1,SWITCHBLOCK_STATE)&&
								_walkflag(x-1,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x-1,y+v1,  1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+v2-1,1,SWITCHBLOCK_STATE)&&
								!_walkflag(x-1,y+15,  1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
								sprite::move((zfix)0,(zfix)1);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=left;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
			}
			
			return;
		}
		
		if(DrunkRight())
		{
			if(yoff && !is_on_conveyor && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && jumping<1)
			{
				if(dir!=left && dir!=right)
				{
					if(yoff>2&&yoff<6)
					{
						moveOld2(dir);
					}
					else if(yoff>=6)
					{
						moveOld2(down);
					}
					else if(yoff>=1)
					{
						moveOld2(up);
					}
				}
				else
				{
					if(yoff>=4)
					{
						moveOld2(down);
					}
					else if(yoff<4)
					{
						moveOld2(up);
					}
				}
			}
			else
			{
				info = walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,right)
					|| walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+(isSideViewHero()?0:8),1,right);
				
				execute(info);
				
				if(!info.isUnwalkable())
				{
					moveOld2(right);
					return;
				}
				
				if(!DrunkUp() && !DrunkDown())
				{
					if(NO_GRIDLOCK)
					{
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
							   
						if(!_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
							   !_walkflag(x+16,y+v2,1,SWITCHBLOCK_STATE)&&
							   _walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x+16,y+v1,1,SWITCHBLOCK_STATE)&&
								   !_walkflag(x+16,y+v2-1,1,SWITCHBLOCK_STATE)&&
								   !_walkflag(x+16,y+15,1,SWITCHBLOCK_STATE))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
								sprite::move((zfix)0,(zfix)1);
						}
						else
						{
							pushing=push+1;
						}
					}
					else pushing=push+1;
					
					if(charging==0 && spins==0)
					{
						dir=right;
					}
					
					if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						herostep();
					}
					
					return;
				}
			}
		}
	}
}

bool HeroClass::scr_walkflag(zfix_round zdx,zfix_round zdy,int d2,bool kb)
{
	if(toogam) return false;
	int dx = zdx.getRound(), dy = zdy.getRound();
	
	if(blockpath && dy<80) //Blocked top parts of rooms
		return true;
	
	if(mblock2.active() && mblock2.hit(dx,dy,0,1,1,1))
		return true;
	//collide_object handled in scr_canmove
	
	if(isdungeon() && currscr<128 && dy<40
		&& ((x<=112||x>=128) || _walkflag(120,24,2,SWITCHBLOCK_STATE))
		&& !get_qr(qr_FREEFORM))
		return true; //Old NES dungeon stuff
	
	bool solid = _walkflag(zdx,zdy,1,SWITCHBLOCK_STATE);
	
	if(isdungeon() && currscr<128 && !get_qr(qr_FREEFORM))
	{
		if(dx>=112&&dx<120&&dy<40&&dy>=32)
			solid=true;
		
		if(dx>=136&&dx<144&&dy<40&&dy>=32)
			solid=true;
	}
	
	if(action==swimming || IsSideSwim())
	{
		if(!solid)
		{
			bool isthissolid = false;
			if (_walkflag(x+7,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
				|| _walkflag(x+7,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE)
				|| _walkflag(x+8,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
				|| _walkflag(x+8,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE))
				isthissolid = true;
			//This checks if Hero is currently swimming in solid water (cause even if the QR "No Hopping" is enabled, he should still hop out of solid water) - Dimi
			
			int ls = 22;
			if((get_qr(qr_DROWN) && isSwimming()) || (!diagonalMovement) || get_qr(qr_NO_HOPPING))
				ls = 1;
			if(landswim < ls)
			{
				if(dx<0||dy<0);
				else if(dx>=256);
				else if(dy>=176);
				else if(get_qr(qr_DROWN) && !ilswim);
				else if(iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx,dy)) //!DIMI: weird duplicate function here before. Was water bugged this whole time, or was it just an unneccessary duplicate?
					solid = false;
				else
					solid = true;
			}
		}
		else
		{
			int32_t wtrx  = iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx,dy);
			
			if(wtrx)
				solid = false;
		}
	}
	else if(ladderx+laddery)                                  // ladder is being used
	{
		int32_t lx = zfix(dx);
		int32_t ly = zfix(dy);
		
		if(ladderdir<=down) //vertical ladder
		{
			if(abs(ly-(laddery+8))<=8) // ly is between laddery (laddery+8-8) and laddery+16 (laddery+8+8)
			{
				bool temp = false;
				
				if(!(abs(lx-(ladderx+8))<=8))
					temp = true;
						
				if(!temp)
				{
					solid = false;
				}
				else if(current_item_power(itype_ladder)<2 && (d2==left || d2==right) && !isSideViewHero())
				{
					solid = true;
				}
			}
		}
		else //horizontal ladder
		{
			if(abs(lx-(ladderx+8))<=8)
			{
				if(abs(ly-(laddery+(bigHitbox?8:12)))<=(bigHitbox?8:4))
				{
					solid = false;
				}
				else if(current_item_power(itype_ladder)<2 && (d2==up || d2==down))
				{
					solid = true;
				}
				else if((abs(ly-laddery+8)<=8) && d2<=down)
				{
					solid = false;
				}
			}
		}
	}
	else if(solid || isSideViewHero() || get_qr(qr_DROWN))
	{
		// see if it's a good spot for the ladder or for swimming
		bool unwalkablex  = _walkflag(zdx,zdy,1,SWITCHBLOCK_STATE); //will be used later for the ladder -DD
		
		if(get_qr(qr_DROWN))
		{
			// Drowning changes the following attributes:
			// * Dangerous water is also walkable, so ignore the previous
			// definitions of unwalkablex.
			// * Instead, prevent the ladder from being used in the
			// one frame where Hero has landed on water before drowning.
			unwalkablex = !iswaterex(MAPCOMBO(x+4,y+11), currmap, currscr, -1, x+4,y+11);
		}
		
		// check if he can swim
		if(current_item(itype_flippers) && z==0 && fakez==0)
		{
			int32_t wtrx  = iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx,dy);
			if (current_item(itype_flippers) >= combobuf[wtrx].attribytes[0] && (!(combobuf[wtrx].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))) //Don't swim if the water's required level is too high! -Dimi
			{
				//ladder ignores water combos that are now walkable thanks to flippers -DD
				unwalkablex = unwalkablex && (!wtrx);
				
				if(landswim >= 22)
				{
					solid = false;
				}
				else if(wtrx)
				{
					if(dir==d2)
					{
						ladderx = 0;
						laddery = 0;
					}
				}
			}
		}
		
		// check if he can use the ladder
		// "Allow Ladder Anywhere" is toggled by fLADDER
		if(can_deploy_ladder())
			// laddersetup
		{
			// Check if there's water to use the ladder over
			bool wtrx = (iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx,dy) != 0);
			int32_t ldrid = current_item_id(itype_ladder);
			bool ladderpits = ldrid > -1 && (itemsbuf[ldrid].flags&ITEM_FLAG1);
			
			if(wtrx)
			{
				if(isSideViewHero())
				{
					wtrx  = !_walkflag(zdx, zdy+8, 1,SWITCHBLOCK_STATE) && !_walkflag(dx, dy, 1,SWITCHBLOCK_STATE) && dir!=down;
				}
				// * walk on half-water using the ladder instead of using flippers.
				// * otherwise, walk on ladder(+hookshot) combos.
				else if((isstepable(MAPCOMBO(dx, dy)) || wtrx==true))
				{
					if(!get_qr(qr_OLD_210_WATER))
					{
						//if Hero could swim on a tile instead of using the ladder,
						//refuse to use the ladder to step over that tile. -DD
						wtrx  = isstepable(MAPCOMBO(dx, dy)) && unwalkablex;
					}
				}
			}
			else
			{
				// No water; check other things
				
				//Check pits
				if(ladderpits)
				{
					int32_t pit_cmb = getpitfall(dx,dy);
					wtrx = pit_cmb && (combobuf[pit_cmb].usrflags&cflag4);
				}
				if(!ladderpits || (!wtrx || isSideViewHero())) //If no pit, check ladder combos
				{
					int32_t combo=combobuf[MAPCOMBO(dx, dy)].type;
					wtrx=(combo==cLADDERONLY || combo==cLADDERHOOKSHOT);
				}
			}
			
			for (int32_t i = 0; i <= 1; ++i)
			{
				if(tmpscr2[i].valid!=0)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(i,dx,dy)].type == cBRIDGE && !_walkflag_layer(dx,dy,1, &(tmpscr2[i]))) wtrx = false;
					}
					else
					{
						if (combobuf[MAPCOMBO2(i,dx,dy)].type == cBRIDGE && _effectflag_layer(dx,dy,1, &(tmpscr2[i]))) wtrx = false;
					}
				}
			}
			bool walkwater = (get_qr(qr_DROWN) && !iswaterex(MAPCOMBO(dx,dy), currmap, currscr, -1, dx,dy));
			
			if(d2==dir)
			{
				int32_t c = walkwater ? 0:8;
				int32_t b = walkwater ? 8:0;
				
				if(d2>=left)
				{
					// If the difference between dy and y is small enough
					if(abs((dy)-(int32_t(y+c)))<=(b) && wtrx)
					{
						// Don't activate the ladder if it would be entirely
						// over water and Hero has the flippers. This isn't
						// a good way to do this, but it's too risky
						// to make big changes to this stuff.
						bool deployLadder=true;
						int32_t lx=dx&0xF0;
						if(current_item(itype_flippers) && current_item(itype_flippers) >= combobuf[iswaterex(MAPCOMBO(lx+8, y+8), currmap, currscr, -1, lx+8, y+8)].attribytes[0] && z==0 && fakez==0)
						{
							if(iswaterex(MAPCOMBO(lx, y), currmap, currscr, -1, lx, y) && 
								iswaterex(MAPCOMBO(lx+15, y), currmap, currscr, -1, lx+15, y) &&
								iswaterex(MAPCOMBO(lx, y+15), currmap, currscr, -1, lx, y+15) && 
								iswaterex(MAPCOMBO(lx+15, y+15), currmap, currscr, -1, lx+15, y+15))
								deployLadder=false;
						}
						if(deployLadder)
						{
							ladderx = dx&0xF0;
							laddery = y;
							ladderdir = left;
							ladderstart = d2;
							solid = laddery!=y.getInt();
						}
					}
				}
				else if(d2<=down)
				{
					// If the difference between dx and x is small enough
					if(abs((dx)-(int32_t(x+c)))<=(b) && wtrx)
					{
						ladderx = x;
						laddery = dy&0xF0;
						ladderdir = up;
						ladderstart = d2;
						solid = ladderx!=x.getInt();
					}
				}
			}
		}
	}
	
	return solid;
}

bool HeroClass::scr_canmove(zfix dx, zfix dy, bool kb, bool ign_sv)
{
	if(toogam) return true;
	if(!(dx || dy)) return true;
	zfix bx = x, by = y+(bigHitbox?0:8); //left/top
	zfix rx = x+15.9999_zf, ry = y+15.9999_zf; //right/bottom
	zfix wid = 16, hei = bigHitbox ? 16 : 8;
	if(!ign_sv && dy < 0 && sideview_mode() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
		return false;
	
	bool nosolid = true;
	
	if(dx && !dy)
	{
		if(dx < 0)
		{
			zfix mx = bx+dx;
			for(zfix ty = 0; by+ty < ry; ty += 8)
			{
				if(scr_walkflag(mx, by+ty, left, kb))
					return false;
			}
			if(scr_walkflag(mx.rnd(ROUND_CEIL), ry.rnd(ROUND_CEIL), left, kb))
				return false;
			if(nosolid && collide_object(bx+dx,by,-dx,hei,this))
				return false;
		}
		else
		{
			zfix mx = rx+dx;
			int lx = mx-hit_width+1;
			for(zfix ty = 0; by+ty < ry; ty += 8)
			{
				if(scr_walkflag(mx, by+ty, right, kb))
					return false;
			}
			if(scr_walkflag(mx.rnd(ROUND_CEIL), ry.rnd(ROUND_CEIL), right, kb))
				return false;
			if(nosolid && collide_object(bx+wid,by,dx,hei,this))
				return false;
		}
	}
	else if(dy && !dx)
	{
		if(dy < 0)
		{
			zfix my = by+dy;
			for(zfix tx = 0; bx+tx < rx; tx += 8)
			{
				if(scr_walkflag(bx+tx, my, up, kb))
					return false;
			}
			if(scr_walkflag(rx.rnd(ROUND_CEIL), my.rnd(ROUND_CEIL), up, kb))
				return false;
			if(nosolid && collide_object(bx,by+dy,wid,-dy,this))
				return false;
		}
		else
		{
			zfix my = ry+dy;
			int ly = my-hit_height+1;
			for(zfix tx = 0; bx+tx < rx; tx += 8)
			{
				if(scr_walkflag(bx+tx, my, down, kb))
					return false;
			}
			if(scr_walkflag(rx.rnd(ROUND_CEIL), my.rnd(ROUND_CEIL), down, kb))
				return false;
			if(nosolid && collide_object(bx,by+hei,wid,dy,this))
				return false;
		}
	}
	else //! Untested, and currently unused.
	{
		return scr_canmove(dx, 0, kb, ign_sv) && scr_canmove(dy, 0, kb, ign_sv);
	}
	return true;
}
bool handle_movestate(std::function<bool()> proc)
{
	zfix ox = Hero.x, oy = Hero.y;
	auto oladderx = Hero.ladderx;
	auto oladdery = Hero.laddery;
	auto oladderdir = Hero.ladderdir;
	auto oladderstart = Hero.ladderstart;
	
	bool ret = proc();
	
	Hero.x = ox;
	Hero.y = oy;
	Hero.ladderx = oladderx;
	Hero.laddery = oladdery;
	Hero.ladderdir = oladderdir;
	Hero.ladderstart = oladderstart;
	
	return ret;
}

zfix handle_movestate_zfix(std::function<zfix()> proc)
{
	zfix ox = Hero.x, oy = Hero.y;
	auto oladderx = Hero.ladderx;
	auto oladdery = Hero.laddery;
	auto oladderdir = Hero.ladderdir;
	auto oladderstart = Hero.ladderstart;
	
	zfix ret = proc();
	
	Hero.x = ox;
	Hero.y = oy;
	Hero.ladderx = oladderx;
	Hero.laddery = oladdery;
	Hero.ladderdir = oladderdir;
	Hero.ladderstart = oladderstart;
	
	return ret;
}

optional<zfix> HeroClass::get_solid_coord(zfix tx, zfix ty, byte dir, byte mdir, bool kb, zfix earlyterm)
{
	zfix tmp;
	switch(dir)
	{
		default:
		case up:
			tmp = binary_search_zfix(ty, earlyterm, [&](zfix ty, zfix& retval)
				{
					if(!scr_walkflag(tx,ty,mdir,kb))
					{
						retval = ty;
						return BSEARCH_CONTINUE_UP;
					}
					else return BSEARCH_CONTINUE_DOWN;
				}, -999);
			if(tmp > -999)
				tmp -= 15.9999_zf;
			break;
		case down:
			tmp = binary_search_zfix(ty, earlyterm, [&](zfix ty, zfix& retval)
				{
					if(!scr_walkflag(tx,ty,mdir,kb))
					{
						retval = ty;
						return BSEARCH_CONTINUE_DOWN;
					}
					else return BSEARCH_CONTINUE_UP;
				}, -999);
			if(tmp > -999)
				tmp -= (bigHitbox?0:8);
			break;
		case left:
			tmp = binary_search_zfix(tx, earlyterm, [&](zfix tx, zfix& retval)
				{
					if(!scr_walkflag(tx,ty,mdir,kb))
					{
						retval = tx;
						return BSEARCH_CONTINUE_UP;
					}
					else return BSEARCH_CONTINUE_DOWN;
				}, -999);
			if(tmp > -999)
				tmp -= 15.9999_zf;
			break;
		case right:
			tmp = binary_search_zfix(tx, earlyterm, [&](zfix tx, zfix& retval)
				{
					if(!scr_walkflag(tx,ty,mdir,kb))
					{
						retval = tx;
						return BSEARCH_CONTINUE_DOWN;
					}
					else return BSEARCH_CONTINUE_UP;
				}, -999);
			break;
	}
	if(tmp > -999)
	{
		zfix tx2(x), ty2(y);
		zfix dx, dy;
		switch(dir)
		{
			case left:
				tx2 = tmp;
				break;
			case right:
				tx2 = tmp;
				break;
			default:
			case up:
				ty2 = tmp;
				break;
			case down:
				ty2 = tmp;
				break;
		}
		switch(mdir)
		{
			case left:
				dx = -0.0001_zf;
				break;
			case right:
				dx = 0.0001_zf;
				break;
			default:
			case up:
				dy = -0.0001_zf;
				break;
			case down:
				dy = 0.0001_zf;
				break;
		}
		zfix oldx(x), oldy(y);
		x = tx2;
		y = ty2;
		bool valid = scr_canmove(dx, dy, kb, true);
		x = oldx;
		y = oldy;
		if(valid)
			return tmp;
	}
	return std::nullopt;
}

bool HeroClass::movexy(zfix dx, zfix dy, bool kb, bool ign_sv, bool shove, bool earlyret)
{
	bool ret = true;
	bool sv = !ign_sv && sideview_mode() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking;
	if(sv)
		dy = 0;
	if(dx && dy)
		shove = false;
	bool checkladder = dy < 0;
	
	const int scl = 2;
	while(abs(dx) > scl || abs(dy) > scl)
	{
		if(abs(dx) > abs(dy))
		{
			int32_t tdx = dx.sign() * scl;
			if(movexy(tdx, 0, kb, ign_sv, shove, earlyret))
				dx -= tdx;
			else
			{
				if(earlyret) return false;
				dx = tdx;
				ret = false;
			}
		}
		else
		{
			int32_t tdy = dy.sign() * scl;
			if(movexy(0, tdy, kb, ign_sv, shove, earlyret))
				dy -= tdy;
			else
			{
				if(earlyret) return false;
				dy = tdy;
				ret = false;
			}
		}
	}
	
	bool skipdmg = earlyret || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || hclk || ((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS));
	if(dx)
	{
		if(scr_canmove(dx, 0, kb, ign_sv))
			x += dx;
		else
		{
			bool stopped = true;
			if(shove)
			{
				zfix tx = (dx < 0 ? (x-1) : (x+16));
				int v=bigHitbox?0:8;
				zfix ly = y+v;
				zfix ry = y+15.9999_zf;
				auto mdir = GET_XDIR(dx);
				bool hit_top = scr_walkflag(tx.rnd(RoundDir(dx)),ly,mdir,false);
				bool hit_bottom = scr_walkflag(tx.rnd(RoundDir(dx)),ry.rnd(ROUND_CEIL),mdir,false);
				if(hit_top!=hit_bottom)
				{
					bool shoved = false;
					if(hit_bottom) //shove up
					{
						if(skipdmg || !checkdamagecombos(tx,get_qr(qr_SENSITIVE_SOLID_DAMAGE)?int32_t(y+15):(v+bigHitbox?11:4)))
						{
							if(optional<zfix> ty = get_solid_coord(tx,ry,up,mdir,false,ry-shove_offset))
							{
								zfix dy = zc_max(-1_zf,*ty-y);
								if((shoved = dy))
									movexy(0, dy, kb, true, false, false);
							}
						}
					}
					else //shove down
					{
						if(skipdmg || !checkdamagecombos(tx,v+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
						{
							if(optional<zfix> ty = get_solid_coord(tx,ly,down,mdir,false,ly+shove_offset))
							{
								zfix dy = zc_min(1_zf,*ty-y);
								if((shoved = dy))
									movexy(0, dy, kb, true, false, false);
							}
						}
					}
					
					if(shoved && scr_canmove(dx, 0, kb, ign_sv))
					{
						x += dx;
						stopped = false;
					}
				}
			}
			if(stopped)
			{
				ret = false;
				int xsign = dx.sign();
				while(scr_canmove(xsign, 0, kb, ign_sv))
				{
					x += xsign;
					dx -= xsign;
				}
				if(dx)
				{
					dx.doDecBound(0,-9999, 0,9999);
					dx = binary_search_zfix(dx.decsign(), dx, [&](zfix val, zfix& retval){
						if(scr_canmove(val, 0, kb, ign_sv))
						{
							retval = val;
							return BSEARCH_CONTINUE_AWAY0;
						}
						else return BSEARCH_CONTINUE_TOWARD0;
					});
					x += dx;
				}
			}
		}
	}
	if(dy)
	{
		if(scr_canmove(0, dy, kb, ign_sv))
			y += dy;
		else
		{
			bool stopped = true;
			if(shove)
			{
				zfix ty = (dy < 0 ? (y+(bigHitbox?0:8)-1) : (y+16));
				zfix lx = x;
				zfix rx = x+15.9999_zf;
				auto mdir = GET_YDIR(dy);
				bool hit_left = scr_walkflag(lx,ty.rnd(RoundDir(dy)),mdir,false);
				bool hit_right = scr_walkflag(rx.rnd(ROUND_CEIL),ty.rnd(RoundDir(dy)),mdir,false);
				if(hit_left!=hit_right)
				{
					bool shoved = false;
					if(hit_right) //shove left
					{
						if(skipdmg || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),ty))
						{
							if(optional<zfix> tx = get_solid_coord(rx,ty,left,mdir,false,rx-shove_offset))
							{
								zfix dx = zc_max(-1_zf,*tx-x);
								if((shoved = dx))
									movexy(dx, 0, kb, true, false, false);
							}
						}
					}
					else //shove right
					{
						if(skipdmg || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),ty))
						{
							if(optional<zfix> tx = get_solid_coord(lx,ty,right,mdir,false,lx+shove_offset))
							{
								zfix dx = zc_min(1_zf,*tx-x);
								if((shoved = dx))
									movexy(dx, 0, kb, true, false, false);
							}
						}
					}
					
					if(shoved && scr_canmove(0, dy, kb, ign_sv))
					{
						y += dy;
						stopped = false;
					}
				}
			}
			if(stopped)
			{
				if(earlyret) return false;
				ret = false;
				int ysign = dy.sign();
				while(scr_canmove(0, ysign, kb, ign_sv))
				{
					y += ysign;
					dy -= ysign;
				}
				if(dy)
				{
					dy.doDecBound(0,-9999, 0,9999);
					dy = binary_search_zfix(dy.decsign(), dy, [&](zfix val, zfix& retval){
						if(scr_canmove(0, val, kb, ign_sv))
						{
							retval = val;
							return BSEARCH_CONTINUE_AWAY0;
						}
						else return BSEARCH_CONTINUE_TOWARD0;
					});
					y += dy;
				}
			}
		}
	}
	
	if(earlyret)
		return ret;
	WalkflagInfo info;
	info = walkflag(x,y+8-(bigHitbox*8)-4,2,up);
	execute(info);
	if(!ign_sv && sideview_mode() && IsSideSwim() && checkladder)
	{
		if(!iswaterex(MAPCOMBO(x, y+(bigHitbox?0:8)), currmap, currscr, -1, x, y+(bigHitbox?0:8) - 2, true, false)
			&& !canSideviewLadderRemote(x, y-4) && !info.isUnwalkable() && (y+(bigHitbox?0:8) - 4) > 0)
		{
			if (game->get_sideswim_jump() != 0)
			{
				setFall(zfix(0-(FEATHERJUMP*(game->get_sideswim_jump()/10000.0))));
				sfx(WAV_ZN1SPLASH,(int32_t)x);
				hopclk = 0;
				if (charging || spins) action = attacking;
				else action = none;
			}
			else
			{
				movexy(0,-1*dy,false,false,false);
			}
		}
	}
	return ret;
}
bool HeroClass::can_movexy(zfix dx, zfix dy, bool kb, bool ign_sv, bool shove)
{
	return handle_movestate([&]()
	{
		return movexy(dx,dy,kb,ign_sv,shove,true);
	});
}
bool HeroClass::moveAtAngle(zfix degrees, zfix px, bool kb, bool ign_sv, bool shove, bool earlyret)
{
	double v = degrees.getFloat() * PI / 180.0;
	zfix dx = zc::math::Cos(v)*px, dy = zc::math::Sin(v)*px;
	return movexy(dx, dy, kb, ign_sv, shove, earlyret);
}
bool HeroClass::can_moveAtAngle(zfix degrees, zfix px, bool kb, bool ign_sv, bool shove)
{
	return handle_movestate([&]()
	{
		return moveAtAngle(degrees,px,kb,ign_sv,shove,true);
	});
}
bool HeroClass::moveDir(int dir, zfix px, bool kb, bool ign_sv, bool shove, bool earlyret)
{
	static const zfix diagrate = zslongToFix(7071);
	switch(NORMAL_DIR(dir))
	{
		case up:
			return movexy(0, -px, kb, ign_sv, shove, earlyret);
		case down:
			return movexy(0, px, kb, ign_sv, shove, earlyret);
		case left:
			return movexy(-px, 0, kb, ign_sv, shove, earlyret);
		case right:
			return movexy(px, 0, kb, ign_sv, shove, earlyret);
		case r_up:
			return movexy(px*diagrate, -px*diagrate, kb, ign_sv, shove, earlyret);
		case r_down:
			return movexy(px*diagrate, px*diagrate, kb, ign_sv, shove, earlyret);
		case l_up:
			return movexy(-px*diagrate, -px*diagrate, kb, ign_sv, shove, earlyret);
		case l_down:
			return movexy(-px*diagrate, px*diagrate, kb, ign_sv, shove, earlyret);
	}
	return false;
}
bool HeroClass::can_moveDir(int dir, zfix px, bool kb, bool ign_sv, bool shove)
{
	return handle_movestate([&]()
	{
		return moveDir(dir,px,kb,ign_sv,shove,true);
	});
}


bool HeroClass::premove()
{
	if(lstunclock) return false;
	if(is_conveyor_stunned) return (convey_forcex || convey_forcey);
	int32_t xoff=x.getInt()&7;
	int32_t yoff=y.getInt()&7;
	if(NO_GRIDLOCK)
	{
		xoff = 0;
		yoff = 0;
	}
	int32_t push=pushing;
	int32_t oldladderx=-1000, oldladdery=-1000; // moved here because linux complains "init crosses goto ~Koopa
	int32_t flippers_id = current_item_id(itype_flippers);
	itemdata const& itm = itemsbuf[flippers_id];
	byte intbtn = byte(itm.misc3&0xFF);
	bool dive_pressed = getIntBtnInput(intbtn, true, true, false, false, true);
	bool eatdive = false;
	if(diveclk>0)
	{
		if (isSideViewHero() && get_qr(qr_SIDESWIM)) diveclk = 0;
		--diveclk;
		if(isDiving() && flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG2 && dive_pressed) //Cancellable Diving -V
		{
			diveclk = itemsbuf[flippers_id].misc2;
			eatdive = true;
		}
	}
	else if(action == swimming && dive_pressed)
	{
		bool global_diving=(flippers_id > -1 && itemsbuf[flippers_id].flags & ITEM_FLAG1);
		bool screen_diving=(tmpscr->flags5&fTOGGLEDIVING) != 0;
		
		if(global_diving==screen_diving)
		{
			diveclk = (flippers_id < 0 ? 80 : (itemsbuf[flippers_id].misc1 + itemsbuf[flippers_id].misc2));
			eatdive = true;
		}
	}
	if(eatdive)
		getIntBtnInput(intbtn, true, true, false, false, false);
	
	if(action==rafting)
	{
		do_rafting();
		
		if(action==rafting)
		{
			return false;
		}
		
		
		set_respawn_point();
		trySideviewLadder();
	}
	
	int32_t olddirectwpn = directWpn; // To be reinstated if startwpn() fails
	int32_t btnwpn = -1;
	
	//&0xFFF removes the "bow & arrows" bitmask
	//The Quick Sword is allowed to interrupt attacks.
	int32_t currentSwordOrWand = (itemsbuf[dowpn].family == itype_wand || itemsbuf[dowpn].family == itype_sword)?dowpn:-1;
	if((!attackclk && action!=attacking && action != sideswimattacking) || ((attack==wSword || attack==wWand) && (itemsbuf[currentSwordOrWand].flags & ITEM_FLAG5)))
	{
		if(DrunkrBbtn())
		{
			btnwpn=getItemFamily(itemsbuf,Bwpn);
			dowpn = NEG_OR_MASK(Bwpn,0xFFF);
			directWpn = directItemB;
		}
		else if(DrunkrAbtn())
		{
			btnwpn=getItemFamily(itemsbuf,Awpn);
			dowpn = NEG_OR_MASK(Awpn,0xFFF);
			directWpn = directItemA;
		}
		else if(get_qr(qr_SET_XBUTTON_ITEMS) && DrunkrEx1btn())
		{
			btnwpn=getItemFamily(itemsbuf,Xwpn);
			dowpn = NEG_OR_MASK(Xwpn,0xFFF);
			directWpn = directItemX;
		}
		else if(get_qr(qr_SET_YBUTTON_ITEMS) && DrunkrEx2btn())
		{
			btnwpn=getItemFamily(itemsbuf,Ywpn);
			dowpn = NEG_OR_MASK(Ywpn,0xFFF);
			directWpn = directItemY;
		}
		
		if(directWpn >= MAXITEMS) directWpn = -1;
		
		// The Quick Sword only allows repeated sword or wand swings.
		if((action==attacking||action==sideswimattacking) && ((attack==wSword && btnwpn!=itype_sword) || (attack==wWand && btnwpn!=itype_wand)))
			btnwpn=-1;
	}
	
	auto swordid = (directWpn>-1 ? directWpn : current_item_id(itype_sword));
	if(can_attack() && (swordid > -1 && itemsbuf[swordid].family==itype_sword) && checkitem_jinx(swordid) && btnwpn==itype_sword && charging==0)
	{
		attackid=directWpn>-1 ? directWpn : current_item_id(itype_sword);
		if(checkbunny(attackid) && (checkmagiccost(attackid) || !(itemsbuf[attackid].flags & ITEM_FLAG6)))
		{
			if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_SWORD_COST))
			{
				paymagiccost(attackid,true);
				misc_internal_hero_flags |= LF_PAID_SWORD_COST;
			}
			SetAttack();
			attack=wSword;
			
			attackclk=0;
			sfx(itemsbuf[directWpn>-1 ? directWpn : current_item_id(itype_sword)].usesound, pan(x.getInt()));
			
			if(dowpn>-1 && itemsbuf[dowpn].script!=0 && !did_scripta && !(FFCore.doscript(ScriptType::Item, dowpn) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
			{
				if(!checkmagiccost(dowpn))
				{
					item_error();
				}
				else
				{
					//clear the item script stack for a new script
					int i = dowpn;
					FFCore.reset_script_engine_data(ScriptType::Item, i);
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
					did_scripta=true;
				}
			}
		}
		else
		{
			item_error();
		}
	}
	else
	{
		did_scripta=false;
	}
	
	if(action!=swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && !getOnSideviewLadder())
	{
		if(DrunkUp() && canSideviewLadder())
		{
			setOnSideviewLadder(true);
		}
		else if(DrunkDown() && canSideviewLadder(true))
		{
			y+=1;
			setOnSideviewLadder(true);
		}
	}
	
	int32_t wx=x;
	int32_t wy=y;
	if(conv_forcedir > -1) dir = conv_forcedir;
	else if((action==none || action==walking) && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
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
	
	bool no_jinx = true;
	if(can_attack() && btnwpn>itype_sword && charging==0 && btnwpn!=itype_rupee) // This depends on item 0 being a rupee...
	{
		bool paidmagic = false;
		bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
		if(!liftonly && btnwpn==itype_wand && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_wand : false) : current_item(itype_wand)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_wand);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & ITEM_FLAG6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_WAND_COST)){
					paymagiccost(attackid,true);
					misc_internal_hero_flags |= LF_PAID_WAND_COST;
				}
				SetAttack();
				attack=wWand;
				attackclk=0;
			}
			else
			{
				item_error();
			}
		}
		else if(!liftonly && (btnwpn==itype_hammer)&&!((action==attacking||action==sideswimattacking) && attack==wHammer)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_hammer : false) : current_item(itype_hammer)))
		{
			no_jinx = checkitem_jinx(dowpn);
			if(!(no_jinx && checkmagiccost(dowpn) && checkbunny(dowpn)))
			{
				item_error();
			}
			else
			{
				paymagiccost(dowpn);
				paidmagic = true;
				SetAttack();
				attack=wHammer;
				attackid=directWpn>-1 ? directWpn : current_item_id(itype_hammer);
				attackclk=0;
			}
		}
		else if(!liftonly && (btnwpn==itype_candle)&&!((action==attacking||action==sideswimattacking) && attack==wFire)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_candle : false) : current_item(itype_candle)))
		{
			//checkbunny handled where magic cost is paid
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_candle);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx)
			{
				SetAttack();
				attack=wFire;
				attackclk=0;
			}
		}
		else if(!liftonly && (btnwpn==itype_cbyrna)&&!((action==attacking||action==sideswimattacking) && attack==wCByrna)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].family==itype_cbyrna : false) : current_item(itype_cbyrna)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_cbyrna);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & ITEM_FLAG6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & ITEM_FLAG6) && !(misc_internal_hero_flags & LF_PAID_CBYRNA_COST)){
					paymagiccost(attackid,true);
					misc_internal_hero_flags |= LF_PAID_CBYRNA_COST;
				}
				SetAttack();
				attack=wCByrna;
				attackclk=0;
			}
			else
			{
				item_error();
			}
		}
		else if(!liftonly && (btnwpn==itype_bugnet)&&!((action==attacking||action==sideswimattacking) && attack==wBugNet)
				&& (directWpn>-1 ? (!item_disabled(directWpn) && itemsbuf[directWpn].family==itype_bugnet) : current_item(itype_bugnet)))
		{
			attackid = directWpn>-1 ? directWpn : current_item_id(itype_bugnet);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && checkmagiccost(attackid))
			{
				paymagiccost(attackid);
				SetAttack();
				attack = wBugNet;
				attackclk = 0;
				sfx(itemsbuf[attackid].usesound);
			}
			else
			{
				item_error();
			}
		}
		else
		{
			auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
			no_jinx = checkitem_jinx(itmid);
			if(no_jinx)
			{
				paidmagic = startwpn(itmid);
				
				if(paidmagic)
				{
					if(action==casting || action==drowning || action==lavadrowning || action == sideswimcasting || action==sidedrowning)
					{
						;
					}
					else
					{
						SetAttack();
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
		}
		
		if(dowpn>-1 && no_jinx && itemsbuf[dowpn].script!=0 && !did_scriptb && !(FFCore.doscript(ScriptType::Item, dowpn) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
		{
			if(!((paidmagic || checkmagiccost(dowpn)) && checkbunny(dowpn)))
			{
				item_error();
			}
			else
			{
				// Only charge for magic if item's magic cost wasn't already charged
				// for the item's main use.
				if(!paidmagic && attack!=wWand)
					paymagiccost(dowpn);
				//clear the item script stack for a new script
				int i = dowpn;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
				did_scriptb=true;
			}
		}
		
		if(no_jinx && (action==casting || action==drowning || action==lavadrowning || action == sideswimcasting || action==sidedrowning))
		{
			return false;
		}
		if(!no_jinx)
			did_scriptb = false;
	}
	else
	{
		did_scriptb=false;
	}
	
	if(attackclk || action==attacking || action==sideswimattacking)
	{
		if(conv_forcedir > -1) dir = conv_forcedir;
		else if((attackclk==0) && action!=sideswimattacking && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
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
		
		bool attacked = doattack();
		
		// This section below interferes with script-setting Hero->Dir, so it comes after doattack
		if(conv_forcedir > -1) dir = conv_forcedir;
		else if(!inlikelike && attackclk>4 && (attackclk&3)==0 && charging==0 && spins==0 && action!=sideswimattacking)
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
		
		if(attacked && (charging==0 && spins<=5) && jumping<1 && action!=sideswimattacking)
		{
			return false;
		}
		else if(!attacked)
		{
			// Spin attack - change direction
			if(spins>1 && attack != wHammer)
			{
				spins--;
				
				if(spins%5==0)
				{
					int id = currentscroll > -1 ? currentscroll : (current_item_id(spins>5 ? itype_spinscroll2 : itype_spinscroll));
					sfx(itemsbuf[id].usesound,pan(x.getInt()));
				}
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
				
				return false;
			}
			else
			{
				spins=0;
			}
			
			if (IsSideSwim()) {action=sideswimming; FFCore.setHeroAction(sideswimming);}
			else {action=none; FFCore.setHeroAction(none);}
			attackclk=0;
			charging=0;
		}
	}
	return true;
}
void HeroClass::movehero()
{
	auto push=pushing;
	pushing=0;
	
	if(!is_conveyor_stunned) //these do not apply to conveyor auto-walk
	{
		if(pitslide()) //Check pit's 'pull'. If true, then Hero cannot fight the pull.
			return;
		
		if(action==walking) //still walking
		{
			if(!DrunkUp() && !DrunkDown() && !DrunkLeft() && !DrunkRight() && !autostep)
			{
				if(attackclk>0) SetAttack();
				else {action = none; FFCore.setHeroAction(none);}
				hero_count=-1;
				return;
			}
			
			autostep=false;
		} // endif (action==walking)
		
		if((action!=swimming)&&(action!=sideswimming)&&(action !=sideswimhit)&&(action !=sideswimattacking)&&(action!=casting)&&(action!=sideswimcasting)&&(action!=drowning)&&(action!=sidedrowning)&&(action!=lavadrowning) && charging==0 && spins==0 && jumping<1)
		{
			action=none; FFCore.setHeroAction(none);
		}
	}
	
	bool nohorz = (isdungeon() && (y<=26 || y>=134) && !get_qr(qr_FREEFORM) && !toogam);
	bool novert = (isdungeon() && (x<=26 || x>=214) && !get_qr(qr_FREEFORM) && !toogam);
	
	zfix dx, dy;
	if(is_conveyor_stunned)
	{
		dx = convey_forcex;
		dy = convey_forcey;
		convey_forcex = 0;
		convey_forcey = 0;
		if(action != walking)
		{
			action = walking; FFCore.setHeroAction(walking);
		}
	}
	else if(diagonalMovement)
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
		
		if(DrunkUp()&&(holddir==-1||holddir==up)&&!novert)
		{
			if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
			{
				dir=up;
			}
			holddir=up;
			
			if(DrunkRight()&&shiftdir!=left&&!nohorz)
			{
				shiftdir=right;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
			}
			else if(DrunkLeft()&&shiftdir!=right&&!nohorz)
			{
				shiftdir=left;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = left;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = left;
			}
			else
			{
				shiftdir=-1;
			}
		}
		else if(DrunkDown()&&(holddir==-1||holddir==down)&&!novert)
		{
			if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
			{
				dir=down;
			}
			holddir=down;
				
			if(DrunkRight()&&shiftdir!=left&&!nohorz)
			{
				shiftdir=right;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
			}
			else if(DrunkLeft()&&shiftdir!=right&&!nohorz)
			{
				shiftdir=left;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = left;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = left;
			}
			else
			{
				shiftdir=-1;
			}
		}
		else if(DrunkLeft()&&(holddir==-1||holddir==left)&&!nohorz)
		{
			if(charging==0 && spins==0 && action != sideswimattacking)
			{
				dir=left;
			}
			sideswimdir = left;
			holddir=left;
			
			if(DrunkUp()&&shiftdir!=down&&!novert)
			{
				shiftdir=up;
			}
			else if(DrunkDown()&&shiftdir!=up&&!novert)
			{
				shiftdir=down;
			}
			else
			{
				shiftdir=-1;
			}
		}
		else if(DrunkRight()&&(holddir==-1||holddir==right)&&!nohorz)
		{
			if(charging==0 && spins==0 && action != sideswimattacking)
			{
				dir=right;
			}
			sideswimdir = right;
			holddir=right;
				
			if(DrunkUp()&&shiftdir!=down&&!novert)
			{
				shiftdir=up;
			}
			else if(DrunkDown()&&shiftdir!=up&&!novert)
			{
				shiftdir=down;
			}
			else
			{
				shiftdir=-1;
			}
		}
		else
		{
			int32_t wtry  = iswaterex(MAPCOMBO(x,y+15), currmap, currscr, -1, x,y+15, true, false);
			int32_t wtry8 = iswaterex(MAPCOMBO(x+15,y+15), currmap, currscr, -1, x+15,y+15, true, false);
			int32_t wtrx = iswaterex(MAPCOMBO(x,y+(bigHitbox?0:8)), currmap, currscr, -1, x,y+(bigHitbox?0:8), true, false);
			int32_t wtrx8 = iswaterex(MAPCOMBO(x+15,y+(bigHitbox?0:8)), currmap, currscr, -1, x+15,y+(bigHitbox?0:8), true, false);
			int32_t wtrc = iswaterex(MAPCOMBO(x+8,y+(bigHitbox?8:12)), currmap, currscr, -1, x+8,y+(bigHitbox?8:12), true, false);
			
			if(can_use_item(itype_flippers,i_flippers)&&current_item(itype_flippers) >= combobuf[wtrc].attribytes[0]&&(!(combobuf[wtrc].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))&&!(ladderx+laddery)&&z==0&&fakez==0)
			{
				if(wtrx&&wtrx8&&wtry&&wtry8 && !DRIEDLAKE)
				{
					//action=swimming;
					if(action !=none && action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking && !isSideViewHero())
					{
						hopclk = 0xFF;
					}
				}
			}
			return;
		}
		get_move(holddir,dx,dy,dir);
	}
	else //4-way
	{
		shiftdir = -1;
		holddir = -1;
		if(!novert && DrunkUp())
		{
			holddir = dir = up;
		}
		else if(!novert && DrunkDown())
		{
			holddir = dir = down;
		}
		else if(!nohorz && DrunkLeft())
		{
			holddir = dir = left;
		}
		else if(!nohorz && DrunkRight())
		{
			holddir = dir = right;
		}
		get_move(holddir,dx,dy,dir);
	}
	if(conv_forcedir > -1)
		dir = conv_forcedir;
	
	if(!new_engine_move(dx,dy))
		pushing = push+1;
}

void HeroClass::get_move(int movedir, zfix& dx, zfix& dy, int32_t& facedir)
{
	dx = 0; dy = 0;
    if(inlikelike || lstunclock > 0 || is_conveyor_stunned || movedir < 0)
        return;
	
	zfix base_movepix(zfix(steprate) / 100);
	zfix movepix(base_movepix);
	zfix up_step(zfix(game->get_sideswim_up()) / 100);
	zfix left_step(zfix(game->get_sideswim_side()) / 100);
	zfix right_step(zfix(game->get_sideswim_side()) / 100);
	zfix down_step(zfix(game->get_sideswim_down()) / 100);
	std::vector<zfix*> steps;
	steps.push_back(&movepix);
	steps.push_back(&up_step);
	steps.push_back(&left_step);
	steps.push_back(&right_step);
	steps.push_back(&down_step);
	
	mod_steps(steps);
	
	up_step = -up_step;
	left_step = -left_step;
	
	
	zfix step(movepix);
	zfix step_diag(movepix);


	if (diagonalMovement)
	{
		//zprint2("Player's X is %d, Y is %d\n", x, y);
		if (((movedir == up || movedir == down) && (shiftdir == left || shiftdir == right)) ||
			(movedir == left || movedir == right) && (shiftdir == up || shiftdir == down))
		{
			step = STEP_DIAGONAL(step);
			up_step = STEP_DIAGONAL(up_step);
			left_step = STEP_DIAGONAL(left_step);
			right_step = STEP_DIAGONAL(right_step);
			down_step = STEP_DIAGONAL(down_step);
		}
		switch (movedir)
		{
		case up:
			switch (shiftdir)
			{
				case left:
					dx = IsSideSwim() ? left_step : -step;
					break;
				case right:
					dx = IsSideSwim() ? right_step : step;
					break;
			}
			if (IsSideSwim())
			{
				dy = up_step;
			}
			else dy = -step;
			break;
		case down:
			switch (shiftdir)
			{
				case left:
					dx = -step;
					if (IsSideSwim()) dx = left_step;
					break;
				case right:
					dx = step;
					if (IsSideSwim()) dx = right_step;
					break;
			}
			dy = IsSideSwim() ? down_step : step;
			break;
		case left:
			switch (shiftdir)
			{
				case up:
					if (IsSideSwim())
					{
						dy = up_step;
					}
					else dy = -step;
					break;
				case down:
					dy = step;
					if (IsSideSwim()) dy = down_step;
					break;
			}
			dx = IsSideSwim() ? left_step : -step;
			break;
		case right:
			switch (shiftdir)
			{
				case up:
					if (!IsSideSwim()) dy = -step;
					if (IsSideSwim())
					{
						dy = up_step;
					}
					break;
				case down:
					dy = step;
					if (IsSideSwim()) dy = down_step;
					break;
			}
			dx = IsSideSwim() ? right_step : step;
			break;
		};
	}
	else
	{
		switch (movedir)
		{
			case up:
				dy = IsSideSwim() ? up_step : -step;
				break;
			case down:
				dy = IsSideSwim() ? down_step : step;
				break;
			case left:
				dx = IsSideSwim() ? left_step : -step;
				break;
			case right:
				dx = IsSideSwim() ? right_step : step;
				break;
		};
	}
	
	if((charging==0 || attack==wHammer) && spins==0 && attackclk!=HAMMERCHARGEFRAME && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (movedir == up || movedir == down))) //!DIRECTION SET
	{
		facedir = movedir;
	}
	else if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (movedir == up || movedir == down) && (shiftdir == left || shiftdir == right) && (charging==0 && spins==0))
	{
		facedir = shiftdir;
	}
}

bool HeroClass::new_engine_move(zfix dx, zfix dy) //no collision check
{
	if(!dx && !dy) return true;
	if(action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
	{
		herostep();
		
		//ack... don't walk if in midair! -DD
		if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(x,y,old_x,old_y) && !getOnSideviewLadder()))
		{
			action=walking; FFCore.setHeroAction(walking);
		}
			
		if(++hero_count > (16*hero_animation_speed))
			hero_count=0;
	}
	else if(!(frame & 1))
	{
		herostep();
	}
	
	bool ret = true;
	if(charging==0 || attack!=wHammer)
	{
		ret = movexy(dx,dy,false,false,true);
	}
	return ret;
}

void HeroClass::moveOld(int32_t d2)
{
	//al_trace("%s\n",d2==up?"up":d2==down?"down":d2==left?"left":d2==right?"right":"?");
    static bool totalskip = false;
    
    if( inlikelike || lstunclock > 0 || is_conveyor_stunned)
        return;
	
    int32_t dx=0,dy=0;
    int32_t xstep=lsteps[x.getInt()&7];
    int32_t ystep=lsteps[y.getInt()&7];
    int32_t z3skip=0;
    int32_t z3diagskip=0;
    bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && ((z==0 && fakez == 0) || tmpscr->flags2&fAIRCOMBOS)) ||
                     (isSideViewHero() && (on_sideview_solid_oldpos(x,y,old_x,old_y)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement);
    bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & ITEM_FLAG10);
    bool is_swimming = (action == swimming);
    
    //slow walk combo, or charging, moves at 2/3 speed
    if(
        (!is_swimming && (slowcharging ^ slowcombo))||
        (is_swimming && (zinit.hero_swim_speed>60))
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
        (is_swimming && (zinit.hero_swim_speed<60))||
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
                if(!isSideViewHero() || (ladderx && laddery && ladderdir==up) || getOnSideviewLadder() || action == sideswimming || action == sideswimhit || action == sideswimattacking) dy-=ystep;
                
                break;
                
            case down:
                if(!isSideViewHero() || (ladderx && laddery && ladderdir==up) || getOnSideviewLadder() || action == sideswimming || action == sideswimhit || action == sideswimattacking) dy+=ystep;
                
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
    
    if((charging==0 || attack==wHammer) && spins==0 && attackclk!=HAMMERCHARGEFRAME && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (d2 == up || d2 == down))) //!DIRECTION SET
    {
        dir=d2;
    }
    else if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (d2 == up || d2 == down) && (shiftdir == left || shiftdir == right) && (charging==0 && spins==0))
    {
	dir = shiftdir; 
    }
    
    if(action != swimming && !IsSideSwim())
    {
        herostep();
        
        //ack... don't walk if in midair! -DD
        if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(x,y,old_x,old_y) && !getOnSideviewLadder()))
	{
            action=walking; FFCore.setHeroAction(walking);
	}
            
        if(++hero_count > (16*hero_animation_speed))
            hero_count=0;
    }
    else if(!(frame & 1))
    {
        herostep();
    }
    
    if(charging==0 || attack!=wHammer)
    {
        sprite::move((zfix)dx,(zfix)dy);
    }
}
void HeroClass::moveOld2(int32_t d2, int32_t forceRate)
{
    if( inlikelike || lstunclock > 0 || is_conveyor_stunned)
        return;
	
	if(!get_qr(qr_NEW_HERO_MOVEMENT) && !IsSideSwim())
	{
		moveOld(d2);
		return;
	}
	
    bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1, -1) && ((z==0 && fakez==0) || tmpscr->flags2&fAIRCOMBOS)) ||
                     (isSideViewHero() && (on_sideview_solid_oldpos(x,y,old_x,old_y)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1, -1));
		     //!DIMITODO: add QR for slow combos under hero
	if(slowcombo) for (int32_t i = 0; i <= 1; ++i)
	{
		if(tmpscr2[i].valid!=0)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,x+7,y+8)].type == cBRIDGE && !_walkflag_layer(x+7,y+8,1, &(tmpscr2[i])))
				{
					slowcombo = false;
					break;
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,x+7,y+8)].type == cBRIDGE && _effectflag_layer(x+7,y+8,1, &(tmpscr2[i])))
				{
					slowcombo = false;
					break;
				}
			}
		}
	}
    bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & ITEM_FLAG10);
    bool is_swimming = (action == swimming);
	bool fastSwim = (zinit.hero_swim_speed>60);
	zfix rate(steprate);
	int32_t shieldid = getCurrentActiveShield();
	if(shieldid > -1)
	{
		itemdata const& shield = itemsbuf[shieldid];
		if(shield.flags & ITEM_FLAG10) //Change Speed flag
		{
			zfix perc = shield.misc7;
			perc /= 100;
			if(perc < 0)
				perc = (perc*-1)+1;
			rate = (rate * perc) + shield.misc8;
		}
	}
	
	zfix dx, dy;
	zfix movepix(rate / 100);
	zfix step(movepix);
	zfix step_diag(movepix);
	zfix up_step(game->get_sideswim_up() / -100.0);
	zfix left_step(game->get_sideswim_side() / -100.0);
	zfix right_step(game->get_sideswim_side() / 100.0);
	zfix down_step(game->get_sideswim_down() / 100.0);
	bool checkladder  = false;
	
	if(hero_newstep > movepix) hero_newstep = movepix;
	if(hero_newstep_diag > movepix) hero_newstep_diag = movepix;
	//2/3 speed
	if((is_swimming && fastSwim) || (!is_swimming && (slowcharging ^ slowcombo)))
	{
		step = ((step / 3.0) * 2);
		step_diag = ((step_diag / 3.0) * 2);
		up_step = ((up_step / 3.0) * 2);
		left_step = ((left_step / 3.0) * 2);
		right_step = ((right_step / 3.0) * 2);
		down_step = ((down_step / 3.0) * 2);
	}
	//1/2 speed
	else if((is_swimming && !fastSwim) || (slowcharging && slowcombo))
	{
		step /= 2;
		step_diag /= 2;
		up_step /= 2;
		left_step /= 2;
		right_step /= 2;
		down_step /= 2;
	}
	//normal speed
	else
	{
		//no modification
	}
	
	if(diagonalMovement)
	{
		//zprint2("Player's X is %d, Y is %d\n", x, y);
		if(((d2 == up || d2 == down) && (shiftdir == left || shiftdir == right)) ||
			(d2 == left || d2 == right) && (shiftdir == up || shiftdir == down))
		{
			if(hero_newstep > 0 && hero_newstep_diag > 0)
			{
				step = STEP_DIAGONAL(step);
				step_diag = STEP_DIAGONAL(step_diag);
				up_step = STEP_DIAGONAL(up_step);
				left_step = STEP_DIAGONAL(left_step);
				right_step = STEP_DIAGONAL(right_step);
				down_step = STEP_DIAGONAL(down_step);
			}
		}
		if(hero_newstep < step) step = hero_newstep; //handle collision
		if(hero_newstep_diag < step_diag) step_diag = hero_newstep_diag; //handle collision
		switch(d2)
		{
			case up:
				switch(shiftdir)
				{
					case left:
						dx = -step_diag;
						if (IsSideSwim()) dx = left_step;
						break;
					case right:
						dx = step_diag;
						if (IsSideSwim()) dx = right_step;
						break;
				}
				if(walkable)
				{
					if (!IsSideSwim()) dy = -step;
					if (IsSideSwim()) 
					{
						dy = up_step;
						if (!iswaterex(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), currmap, currscr, -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
					}
				}
				break;
			case down:
				switch(shiftdir)
				{
					case left:
						dx = -step_diag;
						if (IsSideSwim()) dx = left_step;
						break;
					case right:
						dx = step_diag;
						if (IsSideSwim()) dx = right_step;
						break;
				}
				if(walkable)
				{
					dy = step;
					if (IsSideSwim()) dy = down_step;
				}
				break;
			case left:
				switch(shiftdir)
				{
					case up:
						if (!IsSideSwim()) dy = -step_diag;
						if (IsSideSwim()) 
						{
							dy = up_step;
							if (!iswaterex(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), currmap, currscr, -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
						}
						break;
					case down:
						dy = step_diag;
						if (IsSideSwim()) dy = down_step;
						break;
				}
				if(walkable)
				{
					dx = -step;
					if (IsSideSwim()) dx = left_step;
				}
				break;
			case right:
				switch(shiftdir)
				{
					case up:
						if (!IsSideSwim()) dy = -step_diag;
						if (IsSideSwim()) 
						{
							dy = up_step;
							if (!iswaterex(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), currmap, currscr, -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
						}
						break;
					case down:
						dy = step_diag;
						if (IsSideSwim()) dy = down_step;
						break;
				}
				if(walkable)
				{
					dx = step;
					if (IsSideSwim()) dx = right_step;
				}
				break;
		};
	}
	else
	{
		if(hero_newstep < step) step = hero_newstep; //handle collision
		switch(d2)
		{
			case up:
				dy -= step;
				if (IsSideSwim()) dy = up_step;
				break;
			case down:
				dy += step;
				if (IsSideSwim()) dy = down_step;
				break;
			case left:
				dx -= step;
				if (IsSideSwim()) dx = left_step;
				break;
			case right:
				dx += step;
				if (IsSideSwim()) dx = right_step;
				break;
		};
	}
	hero_newstep = movepix;
	hero_newstep_diag = movepix;

	if((charging==0 || attack==wHammer) && spins==0 && attackclk!=HAMMERCHARGEFRAME && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (d2 == up || d2 == down))) //!DIRECTION SET
	{
		dir=d2;
	}
	else if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (d2 == up || d2 == down) && (shiftdir == left || shiftdir == right) && (charging==0 && spins==0))
	{
		dir = shiftdir; 
	}
	if(forceRate > -1)
	{
		checkladder = false;
		switch(dir)
		{
			case right:
			case r_up:
			case r_down:
				dx = zfix(forceRate) / 100;
				break;
			case left:
			case l_up:
			case l_down:
				dx = zfix(-forceRate) / 100;
				break;
			default:
				dx = 0;
		}
		switch(dir)
		{
			case down:
			case r_down:
			case l_down:
				dy = zfix(forceRate) / 100;
				break;
			case up:
			case r_up:
			case l_up:
				dy = zfix(-forceRate) / 100;
				break;
			default:
				dy = 0;
		}
	}
	if(dx == 0 && dy == 0) return;
	if(action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
	{
		herostep();
		
		//ack... don't walk if in midair! -DD
		if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(x,y,old_x,old_y) && !getOnSideviewLadder()))
		{
			action=walking; FFCore.setHeroAction(walking);
		}
			
		if(++hero_count > (16*hero_animation_speed))
			hero_count=0;
	}
	else if(!(frame & 1))
	{
		herostep();
	}
	
	if(charging==0 || attack!=wHammer)
	{
		sprite::move(dx, dy);
		WalkflagInfo info;
		info = walkflag(x,y+8-(bigHitbox*8)-4,2,up);
		execute(info);
		if (checkladder && !canSideviewLadderRemote(x, y-4) && !info.isUnwalkable() && (y + 8 - (bigHitbox * 8) - 4) > 0)
		{
			if (game->get_sideswim_jump() != 0)
			{
				setFall(zfix(0-(FEATHERJUMP*(game->get_sideswim_jump()/10000.0))));
				sfx(WAV_ZN1SPLASH,(int32_t)x);
				hopclk = 0;
				if (charging || spins) action = attacking;
				else action = none;
			}
			else
			{
				sprite::move(0_zf, zfix(-1*dy));
			}
		}
	}
}
	
HeroClass::WalkflagInfo HeroClass::walkflag(zfix fx,zfix fy,int32_t cnt,byte d2)
{
	return walkflag(fx.getInt(), fy.getInt(), cnt, d2);
}
HeroClass::WalkflagInfo HeroClass::walkflag(int32_t wx,int32_t wy,int32_t cnt,byte d2)
{
    WalkflagInfo ret;
    
	wx = vbound(wx, -1, world_w);
	wy = vbound(wy, -1, world_h);

	if (wx < 0 || wx >= world_w || wy < 0 || wy >= world_h)
	{
		ret.setUnwalkable(false);
		return ret;
	}
    
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
    
    if(mblock2.active() && mblock2.hit(wx,wy,0,1,1,1))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    if (collide_object(wx, wy,1, 1))
    {
	ret.setUnwalkable(true);
	return ret;
    }
    
    if(isdungeon() && currscr<128 && wy<(bigHitbox?32:40) && (((diagonalMovement||NO_GRIDLOCK)?(x<=112||x>=128):x!=120) || _walkflag(120,24,2,SWITCHBLOCK_STATE))
            && !get_qr(qr_FREEFORM))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    bool wf = _walkflag(wx,wy,cnt,SWITCHBLOCK_STATE);
    
    if(isdungeon() && currscr<128 && !get_qr(qr_FREEFORM))
    {
        if((diagonalMovement||NO_GRIDLOCK))
        {
            if(wx>=112&&wx<120&&wy<40&&wy>=32) wf=true;
            
            if(wx>=136&&wx<144&&wy<40&&wy>=32) wf=true;
        }
    }
    //All problems related to exiting water are probably here. -Z
    if(action==swimming || IsSideSwim())
    {
        if(!wf)
        {
	    bool isthissolid = false;
		if (_walkflag(x+7,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
                || _walkflag(x+7,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE)
		|| _walkflag(x+8,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE)
                || _walkflag(x+8,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE)) isthissolid = true;
		//This checks if Hero is currently swimming in solid water (cause even if the QR "No Hopping" is enabled, he should still hop out of solid water) - Dimi
		
		
            if(landswim>= (get_qr(qr_DROWN) && isSwimming() ? 1
                           : (!diagonalMovement) ? 1 : (get_qr(qr_NO_HOPPING)?1:22)))
            {
                //Check for out of bounds for swimming
                bool changehop = true;
                
                if((diagonalMovement||NO_GRIDLOCK))
                {
                    if(wx<0||wy<0)
                        changehop = false;
                    else if(wx>world_w - 8)
                        changehop = false;
                    else if(wx>world_w - 16&&cnt==2)
                        changehop = false;
                    else if(wy>world_h - 8)
                        changehop = false;
                }
		if ((get_qr(qr_NO_HOPPING) || CanSideSwim()) && !isthissolid) changehop = false;
                //This may be where the hang-up for exiting water exists. -Z
                // hop out of the water
                if(changehop)
                    ret.setHopClk(1);
            }
            else
            {
                if((!(get_qr(qr_NO_HOPPING) || CanSideSwim()) || isthissolid) && (dir==d2 || shiftdir==d2))
                {
                    //int32_t vx=((int32_t)x+4)&0xFFF8;
                    //int32_t vy=((int32_t)y+4)&0xFFF8;
                    if(d2==left)
                    {
                        if(!iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?6:11)), -1, x-1,y+(bigHitbox?6:11)) &&
                           !iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?9:12)), -1, x-1,y+(bigHitbox?9:12)) &&
                           !_walkflag(x-1,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE) &&
                           !_walkflag(x-1,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==right)
                    {
                        if(!iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?6:11)), -1, x+16,y+(bigHitbox?6:11)) &&
                           !iswaterex_z3(MAPCOMBO(x+16,y+(bigHitbox?9:12)), -1, x+16,y+(bigHitbox?9:12)) &&
                           !_walkflag(x+16,y+(bigHitbox?6:11),1,SWITCHBLOCK_STATE) &&
                           !_walkflag(x+16,y+(bigHitbox?9:12),1,SWITCHBLOCK_STATE))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==up)
                    {
                        if(!iswaterex_z3(MAPCOMBO(x+7,y+(bigHitbox?0:8)-1), -1, x+7,y+(bigHitbox?0:8)-1) &&
                           !iswaterex_z3(MAPCOMBO(x+8,y+(bigHitbox?0:8)-1), -1, x+8,y+(bigHitbox?0:8)-1) &&
                           !_walkflag(x+7,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE) &&
                           !_walkflag(x+8,y+(bigHitbox?0:8)-1,1,SWITCHBLOCK_STATE))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                    else if(d2==down)
                    {
                        if(!iswaterex_z3(MAPCOMBO(x+7,y+16), -1, x+7,y+16) &&
                           !iswaterex_z3(MAPCOMBO(x+8,y+16), -1, x+8,y+16) &&
                           !_walkflag(x+7,y+16,1,SWITCHBLOCK_STATE) &&
                           !_walkflag(x+8,y+16,1,SWITCHBLOCK_STATE))
                        {
                            ret.setHopDir(d2);
                            ret.setIlswim(true);
                        }
                        else ret.setIlswim(false);
                    }
                }
                
                if(wx<0||wy<0);
                else if(wx > world_w - 8);
                else if(wx > world_w - 16 &&cnt==2);
                else if(wy > world_h - 8);
                else if(get_qr(qr_DROWN) && !ilswim);
                else if(iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy)) //!DIMI: weird duplicate function here before. Was water bugged this whole time, or was it just an unneccessary duplicate?
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
            int32_t wtrx  = iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy);
            int32_t wtrx8 = iswaterex_z3(MAPCOMBO(x+8,wy), -1, x+8,wy); //!DIMI: Is x + 8 intentional???
            
            if((d2>=left && wtrx) || (d2<=down && wtrx && wtrx8))
            {
                ret.setUnwalkable(false);
                return ret;
            }
        }
    }
    else if(ladderx+laddery)                                  // ladder is being used
    {
        int32_t lx = !(get_qr(qr_DROWN)&&iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11)&&!_walkflag(x+4,y+11,1,SWITCHBLOCK_STATE)) ? zfix(wx) : x;
        int32_t ly = !(get_qr(qr_DROWN)&&iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11)&&!_walkflag(x+4,y+11,1,SWITCHBLOCK_STATE)) ? zfix(wy) : y;
        
        if((diagonalMovement||NO_GRIDLOCK))
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
                    
                    if(current_item_power(itype_ladder)<2 && (d2==left || d2==right) && !isSideViewHero())
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
                    if(y.getInt()<=laddery)
                    {
                        ret.setUnwalkable(_walkflag(ladderx,laddery-8,1,SWITCHBLOCK_STATE) ||
                                          _walkflag(ladderx+8,laddery-8,1,SWITCHBLOCK_STATE));
                        return ret;
                        
                    }

					[[fallthrough]];
                case down:
                    if(TRUNCATE_TILE(wy)==laddery)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                    
                    break;
                    
                default:
                    if(TRUNCATE_TILE(wx)==ladderx)
                    {
                        ret.setUnwalkable(false);
                        return ret;
                    }
                }
                
                if(d2<=down)
                {
                    ret.setUnwalkable(_walkflag(ladderx,wy,1,SWITCHBLOCK_STATE) || _walkflag(ladderx+8,wy,1,SWITCHBLOCK_STATE));
                    return ret;
                }
                
                ret.setUnwalkable(_walkflag(TRUNCATE_TILE(wx),wy,1,SWITCHBLOCK_STATE) || _walkflag(TRUNCATE_TILE(wx)+8,wy,1,SWITCHBLOCK_STATE));
                return ret;
            }
            
            // different dir
            if(current_item_power(itype_ladder)<2 && !(isSideViewHero() && (d2==left || d2==right)))
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
    else if(wf || isSideViewHero() || get_qr(qr_DROWN))
    {
        // see if it's a good spot for the ladder or for swimming
        bool unwalkablex  = _walkflag(wx,wy,1,SWITCHBLOCK_STATE); //will be used later for the ladder -DD
        bool unwalkablex8 = _walkflag(x+8,wy,1,SWITCHBLOCK_STATE);
        
        if(get_qr(qr_DROWN))
        {
            // Drowning changes the following attributes:
            // * Dangerous water is also walkable, so ignore the previous
            // definitions of unwalkablex and unwalkablex8.
            // * Instead, prevent the ladder from being used in the
            // one frame where Hero has landed on water before drowning.
            unwalkablex = unwalkablex8 = !iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11);
        }
        
        // check if he can swim
        if(current_item(itype_flippers) && z==0 && fakez==0)
        {
		int32_t wtrx  = iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy);
		int32_t wtrx8 = iswaterex_z3(MAPCOMBO(x+8,wy), -1, x+8,wy); //!DIMI: Still not sure if this should be x + 8...
		if (current_item(itype_flippers) >= combobuf[wtrx8].attribytes[0] && (!(combobuf[wtrx8].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))) //Don't swim if the water's required level is too high! -Dimi
		{
		//ladder ignores water combos that are now walkable thanks to flippers -DD
		    unwalkablex = unwalkablex && (!wtrx);
		    unwalkablex8 = unwalkablex8 && (!wtrx8);
		    
		    if(landswim >= 22)
		    {
			ret.setHopClk(2);
			ret.setUnwalkable(false);
			return ret;
		    }
		    else if((d2>=left && wtrx) || (d2<=down && wtrx && wtrx8))
		    {
			if(!(diagonalMovement||NO_GRIDLOCK))
			{
			    ret.setHopClk(2);
			    
			    if(charging || spins>5)
			    {
				//if Hero is charging, he might be facing the wrong direction (we want him to
				//hop into the water, not in the facing direction)
				ret.setDir(d2);
				//moreover Hero can't charge in the water -DD
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
        }
        
        // check if he can use the ladder
        // "Allow Ladder Anywhere" is toggled by fLADDER
        if(can_deploy_ladder())
            // laddersetup
        {
            // Check if there's water to use the ladder over
            bool wtrx = (iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy) != 0);
            bool wtrx8 = (iswaterex_z3(MAPCOMBO(x+8,wy), -1, x+8,wy) != 0);
			int32_t ldrid = current_item_id(itype_ladder);
			bool ladderpits = ldrid > -1 && (itemsbuf[ldrid].flags&ITEM_FLAG1);
            
            if(wtrx || wtrx8)
            {
                if(isSideViewHero())
                {
                    wtrx  = !_walkflag(wx, wy+8, 1,SWITCHBLOCK_STATE) && !_walkflag(wx, wy, 1,SWITCHBLOCK_STATE) && dir!=down;
                    wtrx8 = !_walkflag(wx+8, wy+8, 1,SWITCHBLOCK_STATE) && !_walkflag(wx+8, wy, 1,SWITCHBLOCK_STATE) && dir!=down;
                }
                // * walk on half-water using the ladder instead of using flippers.
                // * otherwise, walk on ladder(+hookshot) combos.
                else if(wtrx==wtrx8 && (isstepable(MAPCOMBO(wx, wy)) || isstepable(MAPCOMBO(wx+8,wy)) || wtrx==true))
                {
		    if(!get_qr(qr_OLD_210_WATER))
		    {
			//if Hero could swim on a tile instead of using the ladder,
			//refuse to use the ladder to step over that tile. -DD
			wtrx  = isstepable(MAPCOMBO(wx, wy)) && unwalkablex;
			wtrx8 = isstepable(MAPCOMBO(wx+8,wy)) && unwalkablex8;
		    }
                }
            }
            else
            {
				// No water; check other things
				
				//Check pits
				if(ladderpits)
				{
					int32_t pit_cmb = getpitfall(wx,wy);
					wtrx = pit_cmb && (combobuf[pit_cmb].usrflags&cflag4);
					pit_cmb = getpitfall(x+8,wy);
					wtrx8 = pit_cmb && (combobuf[pit_cmb].usrflags&cflag4);
				}
				if(!ladderpits || (!(wtrx || wtrx8) || isSideViewHero())) //If no pit, check ladder combos
				{
					int32_t combo=combobuf[MAPCOMBO(wx, wy)].type;
					wtrx=(combo==cLADDERONLY || combo==cLADDERHOOKSHOT);
					combo=combobuf[MAPCOMBO(wx+8, wy)].type;
					wtrx8=(combo==cLADDERONLY || combo==cLADDERHOOKSHOT);
				}
			}
            
	     for (int32_t i = 0; i <= 1; ++i)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,wx,wy)].type == cBRIDGE && !_walkflag_layer(wx,wy,i)) wtrx = false;
				if (combobuf[MAPCOMBO2(i,wx+8,wy)].type == cBRIDGE && !_walkflag_layer(wx+8,wy,i)) wtrx8 = false;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,wx,wy)].type == cBRIDGE && _effectflag_layer(wx,wy,i)) wtrx = false;
				if (combobuf[MAPCOMBO2(i,wx+8,wy)].type == cBRIDGE && _effectflag_layer(wx+8,wy,i)) wtrx8 = false;
			}
		}
            bool walkwater = (get_qr(qr_DROWN) && !iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy));
            
            if((diagonalMovement||NO_GRIDLOCK))
            {
                if(d2==dir)
                {
                    int32_t c = walkwater ? 0:8;
                    int32_t b = walkwater ? 8:0;
                    
                    if(d2>=left)
                    {
                        // If the difference between wy and y is small enough
                        if(abs((wy)-(int32_t(y+c)))<=(b) && wtrx)
                        {
                            // Don't activate the ladder if it would be entirely
                            // over water and Hero has the flippers. This isn't
                            // a good way to do this, but it's too risky
                            // to make big changes to this stuff.
                            bool deployLadder=true;
                            int32_t lx= TRUNCATE_TILE(wx);
                            if(current_item(itype_flippers) && current_item(itype_flippers) >= combobuf[iswaterex_z3(MAPCOMBO(lx+8, y+8), -1, lx+8, y+8)].attribytes[0] && z==0 && fakez==0)
                            {
                                if(iswaterex_z3(MAPCOMBO(lx, y), -1, lx, y) && 
				iswaterex_z3(MAPCOMBO(lx+15, y), -1, lx+15, y) &&
                                iswaterex_z3(MAPCOMBO(lx, y+15), -1, lx, y+15) && 
				iswaterex_z3(MAPCOMBO(lx+15, y+15), -1, lx+15, y+15))
                                    deployLadder=false;
                            }
                            if(deployLadder)
                            {
                                ladderx = TRUNCATE_TILE(wx);
                                laddery = y;
                                ladderdir = left;
                                ladderstart = d2;
                                ret.setUnwalkable(laddery!=y.getInt());
                                return ret;
                            }
                        }
                    }
                    else if(d2<=down)
                    {
                        // If the difference between wx and x is small enough
                        if(abs((wx)-(int32_t(x+c)))<=(b) && wtrx)
                        {
                            ladderx = x;
                            laddery = TRUNCATE_TILE(wy);
                            ladderdir = up;
                            ladderstart = d2;
                            ret.setUnwalkable(ladderx!=x.getInt());
                            return ret;
                        }
                        
                        if(cnt==2)
                        {
                            if(abs((wx+8)-(int32_t(x+c)))<=(b) && wtrx8)
                            {
                                ladderx = x;
                                laddery = TRUNCATE_TILE(wy);
                                ladderdir = up;
                                ladderstart = d2;
                                ret.setUnwalkable(ladderx!=x.getInt());
                                return ret;
                            }
                        }
                    }
                }
            }
            else
            {
                bool flgx  = _walkflag(wx,wy,1,SWITCHBLOCK_STATE) && !wtrx; // Solid, and not steppable
                bool flgx8 = _walkflag(x+8,wy,1,SWITCHBLOCK_STATE) && !wtrx8; // Solid, and not steppable
                
                if((d2>=left && wtrx)
                        // Deploy the ladder vertically even if Hero is only half on water.
                        || (d2<=down && ((wtrx && !flgx8) || (wtrx8 && !flgx))))
                {
                    if(((y.getInt()+15) < wy) || ((y.getInt()+8) > wy))
                        ladderdir = up;
                    else
                        ladderdir = left;
                        
                    if(ladderdir==up)
                    {
                        ladderx = TRUNCATE_HALF_TILE(x.getInt());
                        laddery = TRUNCATE_TILE(wy);
                    }
                    else
                    {
                        ladderx = TRUNCATE_TILE(wx);
                        laddery = TRUNCATE_HALF_TILE(y.getInt());
						
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
HeroClass::WalkflagInfo HeroClass::walkflagMBlock(int32_t wx,int32_t wy)
{
	HeroClass::WalkflagInfo ret;
	if (!mblock2.active()) //Without this, weird swimming behaviors happen.
	{
		ret.setFlags(~1);
		ret.setHopDir(-1);
	}
	if(toogam) return ret;
	if (mblock2.active())
		ret.setUnwalkable(mblock2.hit(wx,wy,0,1,1,1));
	if (collide_object(wx, wy,1, 1))
		ret.setUnwalkable(true);
	return ret;
}

bool HeroClass::checksoliddamage()
{
	if(toogam) return false;

	if (get_qr(qr_NO_SIDEVIEW_SOLID_DAMAGE) && get_qr(qr_NOSOLIDDAMAGECOMBOS)) return false; //no point checking if both of these rules are on
    
	if(z!=0||fakez!=0) return false;
	int32_t bx = x.getInt();
	int32_t by = y.getInt();
	int32_t initk = 0;
	switch(dir)
	{
	case up:
		
		by-=get_qr(qr_NEW_HERO_MOVEMENT2)?(bigHitbox ? 4 : -4):(bigHitbox ? 1 : -7);
		if (get_qr(qr_LENIENT_SOLID_DAMAGE)) bx += 8;
		else if (!get_qr(qr_SENSITIVE_SOLID_DAMAGE)) bx += 4;
		if(by<0)
		{
			return false;
		}
		break;
		
	case down:
		
		by+=20;
		if (by >= world_h)
		{
			return false;
		}
		
		break;
		
	case left:
		bx-=get_qr(qr_NEW_HERO_MOVEMENT2)?1:4;
		if (get_qr(qr_LENIENT_SOLID_DAMAGE)) by += bigHitbox?7:11;
		else if (!get_qr(qr_SENSITIVE_SOLID_DAMAGE)) by += 4;
		if (!bigHitbox && !get_qr(qr_LENIENT_SOLID_DAMAGE)) 
		{
			by+=8;
			initk = 1;
		}
		if(bx<0)
		{
			return false;
		}
		
		break;
		
	case right:
		bx+=get_qr(qr_NEW_HERO_MOVEMENT2)?16:20;
		if (get_qr(qr_LENIENT_SOLID_DAMAGE)) by += bigHitbox?7:11;
		else if (!get_qr(qr_SENSITIVE_SOLID_DAMAGE)) by += 4;
		if (!bigHitbox && !get_qr(qr_LENIENT_SOLID_DAMAGE)) 
		{
			by+=8;
			initk = 1;
		}
		if (bx >= world_w)
		{
			return false;
		}
		
		break;
	}

	newcombo const& cmb = combobuf[MAPCOMBO(bx, by)];
	int32_t t = cmb.type;
	if(cmb.triggerflags[0] & combotriggerONLYGENTRIG)
		t = cNONE;
	int32_t initbx = bx;
	int32_t initby = by;
	
	// Unlike push blocks, damage combos should be tested on layers 2 and under
	for(int32_t i=(get_qr(qr_DMGCOMBOLAYERFIX) ? 2 : 0); i>=0; i--)
	{
		bx = initbx;
		by = initby;
		if (!get_qr(qr_NOSOLIDDAMAGECOMBOS))
		{
			for (int32_t k = initk; k <= 2; k++)
			{
				auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, i);
				newcombo const& cmb = combobuf[rpos_handle.data()];
				t = cmb.type;
				if(cmb.triggerflags[0] & combotriggerONLYGENTRIG)
					t = cNONE;
				// Solid damage combos use pushing>0, hence the code is here.
				if (!get_qr(qr_LESS_AWFUL_SIDESPIKES) || !isSideViewHero() || (dir != down && (dir != up || getOnSideviewLadder())))
				{
					if(combo_class_buf[t].modify_hp_amount && _walkflag(bx,by,1,SWITCHBLOCK_STATE) && pushing>0 && hclk<1 && action!=casting && action != sideswimcasting)
					{
						// Bite Hero
						if (checkdamagecombos(bx, bx, by, by, i-1, true)) return true;
					}
				}
				if (dir < left) bx += (k % 2) ? 7 : 8;
				else by += (k % 2) ? 7 : 8;
				if (get_qr(qr_LENIENT_SOLID_DAMAGE)) break;
			}
		}
		if(isSideViewHero() && !get_qr(qr_NO_SIDEVIEW_SOLID_DAMAGE) && // Check for sideview damage combos
			hclk<1 && action!=casting && action!=sideswimcasting) // ... but only if Hero could be hurt
		{
			if (get_qr(qr_LESS_AWFUL_SIDESPIKES))
			{
				if (on_sideview_solid_oldpos(x,y,old_x,old_y) && (!getOnSideviewLadder() || DrunkDown()))
				{
					if(checkdamagecombos(x+4, x+4, y+16, y+18, i-1, false, false) && checkdamagecombos(x+12, x+12, y+16, y+18, i-1, false, false))
					{
						if (checkdamagecombos(x+4, x+12, y+16, y+18, i-1, false, true)) return true;
					}
				}
				if (checkdamagecombos(x+4, x+12, y+8, y+15, i-1, false, true)) return true;
			}
			else
			{
				//old 2.50.2-ish code for 2.50.0 sideview quests for er_OLDSIDEVIEWSPIKES
				if ( get_qr(qr_OLDSIDEVIEWSPIKES ) )
				{
					// TODO z3 !!!
					if (checkdamagecombos(x+8-(zfix)(tmpscr->csensitive),
						x+8+(zc_max(tmpscr->csensitive-1,0)),
						y+17-(get_qr(qr_LTTPCOLLISION)?tmpscr->csensitive:(tmpscr->csensitive+1)/2),
						y+17+zc_max((get_qr(qr_LTTPCOLLISION)?tmpscr->csensitive:(tmpscr->csensitive+1)/2)-1,0), i-1, true))
							return true;
				}
				else //2.50.1 and later
				{
					if(checkdamagecombos(x+4, x+12, y+16, y+24))
						return true;
				}
			}
			
		}
	}

	return false;
}
void HeroClass::checkpushblock()
{
	if(toogam) return;
	
	if(z!=0||fakez!=0) return;
	
	// Return early in some cases..
	bool earlyReturn=false;
	
	if(!(diagonalMovement||NO_GRIDLOCK) || dir==left)
		if(x.getInt()&15) earlyReturn=true;
		
	// if(y<16) return;
	if(isSideViewHero() && !on_sideview_solid_oldpos(x,y,old_x,old_y)) return;
	
	int32_t bx = TRUNCATE_TILE(x.getInt());
	int32_t by = TRUNCATE_TILE(y.getInt());
	
	switch(dir)
	{
	case up:
		if(y<16)
		{
			earlyReturn=true;
			break;
		}
		
		if(!((int32_t)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
		
		if((int32_t)x&8) bx+=16;
		
		break;
		
	case down:
		if(y>world_h-48)
		{
			earlyReturn=true;
			break;
		}
		else
		{
			by+=16;
			
			if((int32_t)x&8) bx+=16;
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
			
			if(y.getInt()&8)
			{
				by+=16;
			}
		}
		
		break;
		
	case right:
		if(x>world_w-48)
		{
			earlyReturn=true;
			break;
		}
		else
		{
			bx+=16;
			
			if(y.getInt()&8)
			{
				by+=16;
			}
		}
		
		break;
	}
	
	if (checksoliddamage()) return;
	
	if(earlyReturn)
		return;
	
	int itemid=current_item_id(itype_bracelet);
	rpos_t rpos = COMBOPOS_REGION(bx, by);
	size_t combopos = RPOS_TO_POS(rpos);

	bool limitedpush = (itemid>=0 && itemsbuf[itemid].flags & ITEM_FLAG1);
	itemdata const* glove = itemid < 0 ? NULL : &itemsbuf[itemid];
	for(int lyr = 2; lyr >= 0; --lyr) //Top-down, in case of stacked push blocks
	{
		if(get_qr(qr_HESITANTPUSHBLOCKS)&&(pushing<4)) break;
		if(lyr && !get_qr(qr_PUSHBLOCK_LAYER_1_2))
			continue;
		auto rpos_handle = get_rpos_handle(rpos, lyr);
		cpos_info& cpinfo = get_combo_posinfo(rpos_handle);
		mapscr* m = rpos_handle.screen;
		int cid = lyr == 0 ? MAPCOMBO(bx,by) : MAPCOMBOL(lyr,bx,by);
		newcombo const& cmb = combobuf[cid];
		int f = MAPFLAG2(lyr-1,bx,by);
		int f2 = cmb.flag;
		int t = cmb.type;
		
		bool waitblock = (t==cPUSH_WAIT || t==cPUSH_HW || t==cPUSH_HW2) ||
			(t == cPUSHBLOCK && (cmb.usrflags&cflag6));
		int heavy = 0;
		if(t==cPUSH_HW || t==cPUSH_HEAVY)
			heavy = 1;
		else if(t==cPUSH_HEAVY2 || t==cPUSH_HW2)
			heavy = 2;
		else if(t == cPUSHBLOCK)
			heavy = cmb.attribytes[0];
		
		if(waitblock && (pushing<16 || hasMainGuy())) continue;
		
		if(heavy && (itemid<0 || glove->power < heavy ||
			(limitedpush && usecounts[itemid] > zc_max(1, glove->misc3)))) continue;
		
		bool doit=false;
		bool changecombo=false;
		
		int blockdir = dir;
		if(blockdir > 3) blockdir = Y_DIR(dir);
		if(t == cPUSHBLOCK)
		{
			switch(blockdir)
			{
				case up:
					doit = cmb.usrflags & cflag1;
					break;
				case down:
					doit = cmb.usrflags & cflag2;
					break;
				case left:
					doit = cmb.usrflags & cflag3;
					break;
				case right:
					doit = cmb.usrflags & cflag4;
					break;
			}
			if(cmb.usrflags & cflag5) //Separate directions
			{
				if(int limit = cmb.attribytes[4+blockdir])
				{
					if(cpinfo.pushes[blockdir] >= limit)
						doit = false;
				}
				else if(cmb.usrflags & cflag9)
					doit = false;
			}
			else
			{
				if(int limit = cmb.attribytes[4])
				{
					if(cpinfo.sumpush() >= limit)
						doit = false;
				}
				else if(cmb.usrflags & cflag9)
					doit = false;
			}
		}
		else
		{
			if(is_push_flag_dir(f,dir))
			{
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
		}
		
		if(get_qr(qr_SOLIDBLK))
		{
			switch(blockdir)
			{
			case up:
				if(_walkflag(bx,by-8,2,SWITCHBLOCK_STATE)&&!(MAPFLAG2(lyr-1,bx,by-8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx,by-8)==mfBLOCKHOLE))    doit=false;
				
				break;
				
			case down:
				if(_walkflag(bx,by+24,2,SWITCHBLOCK_STATE)&&!(MAPFLAG2(lyr-1,bx,by+24)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx,by+24)==mfBLOCKHOLE))   doit=false;
				
				break;
				
			case left:
				if(_walkflag(bx-16,by+8,2,SWITCHBLOCK_STATE)&&!(MAPFLAG2(lyr-1,bx-16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx-16,by+8)==mfBLOCKHOLE)) doit=false;
				
				break;
				
			case right:
				if(_walkflag(bx+16,by+8,2,SWITCHBLOCK_STATE)&&!(MAPFLAG2(lyr-1,bx+16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx+16,by+8)==mfBLOCKHOLE)) doit=false;
				
				break;
			}
		}
		
		switch(blockdir)
		{
		case up:
			if((MAPFLAG2(lyr-1,bx,by-8)==mfNOBLOCKS||MAPCOMBOFLAG2(lyr-1,bx,by-8)==mfNOBLOCKS))       doit=false;
			
			break;
			
		case down:
			if((MAPFLAG2(lyr-1,bx,by+24)==mfNOBLOCKS||MAPCOMBOFLAG2(lyr-1,bx,by+24)==mfNOBLOCKS))     doit=false;
			
			break;
			
		case left:
			if((MAPFLAG2(lyr-1,bx-16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG2(lyr-1,bx-16,by+8)==mfNOBLOCKS)) doit=false;
			
			break;
			
		case right:
			if((MAPFLAG2(lyr-1,bx+16,by+8)==mfNOBLOCKS||MAPCOMBOFLAG2(lyr-1,bx+16,by+8)==mfNOBLOCKS)) doit=false;
			
			break;
		}
		
		if(doit)
		{
			if(limitedpush)
				++usecounts[itemid];
			
			//   for(int32_t i=0; i<1; i++)
			if(!mblock2.active())
			{
				if(is_push_flag_dir(f,dir))
				{
					m->sflag[combopos]=0;
				}
				
				mblock2.blockLayer = lyr;
				
				if(t == cPUSHBLOCK)
				{
					zfix blockstep = 0.5;
					if(cmb.attrishorts[0] > 0)
						blockstep = zslongToFix(cmb.attrishorts[0]*100);
					mblock2.push_new(zfix(bx),zfix(by),blockdir,f,blockstep);
					mblock2.blockinfo = cpinfo;
					mblock2.blockinfo.push(blockdir, cmb.usrflags&cflag8);
					cpinfo.clear();
					if(cmb.attribytes[1])
						sfx(cmb.attribytes[1],(int32_t)x);
				}
				else
				{
					mblock2.push((zfix)bx,(zfix)by,blockdir,f);
					
					if(get_qr(qr_MORESOUNDS))
						sfx(WAV_ZN1PUSHBLOCK,(int32_t)x);
				}
			}
			break;
		}
	}
}

bool usekey()
{
	int32_t itemid = current_item_id(itype_magickey);
	
	if(itemid<0 ||
			(itemsbuf[itemid].flags & ITEM_FLAG1 ? itemsbuf[itemid].power<dlevel
			 : itemsbuf[itemid].power!=dlevel))
	{
		if(game->lvlkeys[dlevel]!=0)
		{
			game->lvlkeys[dlevel]--;
			//run script for level key item
			int32_t key_item = 0; //current_item_id(itype_lkey); //not possible
			for ( int32_t q = 0; q < MAXITEMS; ++q )
			{
				if ( itemsbuf[q].family == itype_lkey )
				{
					key_item = q; break;
				}
			}
			
			if ( key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
			{
				int i = key_item;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
				FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
			}
			return true;
		}
		else
		{
			if(game->get_keys()==0)
			{
				return false;
			}
			else 
			{
				//run script for key item
				int32_t key_item = 0; //current_item_id(itype_key); //not possible
				for ( int32_t q = 0; q < MAXITEMS; ++q )
				{
					if ( itemsbuf[q].family == itype_key )
					{
						key_item = q; break;
					}
				}
				//zprint2("key_item is: %d\n",key_item);
				//zprint2("key_item script is: %d\n",itemsbuf[key_item].script);
				if ( key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) 
				{
					int i = key_item;
					FFCore.reset_script_engine_data(ScriptType::Item, i);
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
					FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
				}
				game->change_keys(-1);
			}
		}
	}
	
	return true;
}

bool canUseKey(int32_t num)
{
    int32_t itemid = current_item_id(itype_magickey);
    
    if(itemid<0 ||
            (itemsbuf[itemid].flags & ITEM_FLAG1 ? itemsbuf[itemid].power<dlevel
             : itemsbuf[itemid].power!=dlevel))
    {
        return game->lvlkeys[dlevel] + game->get_keys() >= num;
    }
    
    return true;
}

bool usekey(int32_t num)
{
	if(!canUseKey(num)) return false;
	for(auto q = 0; q < num; ++q)
	{
		if(!usekey()) return false; //should never return false here, but, just to be safe....
	}
	return true;
}


bool islockeddoor(int32_t x, int32_t y, int32_t lock)
{
    int32_t mc = (y&0xF0)+(x>>4);
    bool ret = (((mc==7||mc==8||mc==23||mc==24) && tmpscr->door[up]==lock)
                || ((mc==151||mc==152||mc==167||mc==168) && tmpscr->door[down]==lock)
                || ((mc==64||mc==65||mc==80||mc==81) && tmpscr->door[left]==lock)
                || ((mc==78||mc==79||mc==94||mc==95) && tmpscr->door[right]==lock));
    return ret;
}

void HeroClass::oldchecklockblock()
{
	if(toogam) return;
	
	int32_t bx = TRUNCATE_TILE(x.getInt());
	int32_t bx2 = TRUNCATE_TILE(x.getInt() + 8);
	int32_t by = TRUNCATE_TILE(y.getInt());
	// int32_t bx = x.getInt()&0xF0;
	// int32_t bx2 = int32_t(x+8)&0xF0;
	// int32_t by = y.getInt()&0xF0;
	
	switch(dir)
	{
		case up:
			if(!((int32_t)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
		
			break;
		
		case down:
			by+=16;
			break;
		
		case left:
			if((((int32_t)x)&0x0F)<8)
				bx-=16;
		
			if(y.getInt()&8)
			{
				by+=16;
			}
		
			bx2=bx;
			break;
		
		case right:
			bx+=16;
		
			if(y.getInt()&8)
			{
				by+=16;
			}
		
			bx2=bx;
			break;
	}
	
	bool found1=false;
	bool found2=false;
	int32_t foundlayer = -1;
	int32_t cid1 = MAPCOMBO(bx, by), cid2 = MAPCOMBO(bx2, by);
	newcombo const& cmb = combobuf[cid1];
	newcombo const& cmb2 = combobuf[cid2];
	// Layer 0 is overridden by Locked Doors
	if((cmb.type==cLOCKBLOCK && !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx,by,1, -1) && !islockeddoor(bx,by,dLOCKED)))
	{
		found1=true;
		foundlayer = 0;
	}
	else if (cmb2.type==cLOCKBLOCK && !(cmb2.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2,by,1, -1) && !islockeddoor(bx2,by,dLOCKED))
	{
		found2=true;
		foundlayer = 0;
	}
	
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,i)) found1 = false;
			if (combobuf[MAPCOMBO2(i,bx2,by)].type == cBRIDGE && !_walkflag_layer(bx2,by,i)) found2 = false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,i)) found1 = false;
			if (combobuf[MAPCOMBO2(i,bx2,by)].type == cBRIDGE && _effectflag_layer(bx2,by,i)) found2 = false;
		}
	}
	
   
	// Layers
	if(!(found1 || found2))
	{
		foundlayer = -1;
		for(int32_t i=0; i<2; i++)
		{
			cid1 = MAPCOMBO2(i, bx, by);
			cid2 = MAPCOMBO2(i, bx2, by);
			newcombo const& cmb = combobuf[cid1];
			newcombo const& cmb2 = combobuf[cid2];
			if (i == 0)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS))
				{
					if (combobuf[cid1].type == cBRIDGE && !_walkflag_layer(bx,by,1)) continue; //Continue, because It didn't find any on layer 0, and if you're checking
					if (combobuf[cid2].type == cBRIDGE && !_walkflag_layer(bx2,by,1)) continue; //layer 1 and there's a bridge on layer 2, stop checking layer 1.
				}
				else
				{
					if (combobuf[cid1].type == cBRIDGE && _effectflag_layer(bx,by,1)) continue;
					if (combobuf[cid2].type == cBRIDGE && _effectflag_layer(bx2,by,1)) continue;
				}
			}
			if(cmb.type==cLOCKBLOCK && !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx,by,1, i))
			{
				found1=true;
				foundlayer = i+1;
				//zprint("Found layer: %d \n", i);
				break;
			}
			else if(cmb2.type==cLOCKBLOCK && !(cmb2.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2,by,1, i))
			{
				found2=true;
				foundlayer = i+1;
				//zprint("Found layer: %d \n", i);
				break;
			}
		}
	}
	
	if(!(found1 || found2) || pushing<8)
	{
		return;
	}
	newcombo const& cmb3 = combobuf[found1 ? cid1 : cid2];
	if(!try_locked_combo(cmb3))
		return;
	
	auto rpos_handle = found1 ? get_rpos_handle_for_world_xy(bx, by, 0) : get_rpos_handle_for_world_xy(bx2, by, 0);
	if(cmb.usrflags&cflag16)
	{
		setxmapflag(rpos_handle.screen_index, 1<<cmb.attribytes[5]);
		remove_xstatecombos(rpos_handle.screen, rpos_handle.screen_index, 1<<cmb.attribytes[5], false);
	}
	else
	{
		setmapflag(rpos_handle.screen, rpos_handle.screen_index, mLOCKBLOCK);
		remove_lockblocks(rpos_handle.screen, rpos_handle.screen_index);
	}
	if ( cmb3.usrflags&cflag3 )
	{
		if ( (cmb3.attribytes[3]) )
			sfx(cmb3.attribytes[3]);
	}
	else sfx(WAV_DOOR);
}

void HeroClass::oldcheckbosslockblock()
{
	if(toogam) return;
	
	int32_t bx = TRUNCATE_TILE(x.getInt());
	int32_t bx2 = TRUNCATE_TILE(x.getInt()+8);
	int32_t by = TRUNCATE_TILE(y.getInt());
	
	switch(dir)
	{
		case up:
			if(!((int32_t)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
		
			break;
		
		case down:
			by+=16;
			break;
		
		case left:
			if((((int32_t)x)&0x0F)<8)
				bx-=16;
		
			if(y.getInt()&8)
			{
				by+=16;
			}
		
			bx2=bx;
			break;
		
		case right:
			bx+=16;
		
			if(y.getInt()&8)
			{
				by+=16;
			}
		
			bx2=bx;
			break;
	}
	

	bool found1 = false;
	bool found2 = false;
	int32_t foundlayer = -1;
	int32_t cid1 = MAPCOMBO(bx, by), cid2 = MAPCOMBO(bx2, by);
	newcombo const& cmb = combobuf[cid1];
	newcombo const& cmb2 = combobuf[cid2];
	int cmb_screen_index = get_screen_index_for_world_xy(bx, by);
	int cmb2_screen_index = get_screen_index_for_world_xy(bx2, by);
	// Layer 0 is overridden by Locked Doors
	if ((cmb.type == cBOSSLOCKBLOCK && !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx, by, 1, -1) && !islockeddoor(bx, by, dLOCKED)))
	{
		found1 = true;
		foundlayer = 0;
	}
	else if (cmb2.type == cBOSSLOCKBLOCK && !(cmb2.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2, by, 1, -1) && !islockeddoor(bx2, by, dLOCKED))
	{
		found2 = true;
		foundlayer = 0;
	}

	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i, bx, by)].type == cBRIDGE && !_walkflag_layer(bx, by, i, 1)) found1 = false;
			if (combobuf[MAPCOMBO2(i, bx2, by)].type == cBRIDGE && !_walkflag_layer(bx2, by, i, 1)) found2 = false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i, bx, by)].type == cBRIDGE && _effectflag_layer(bx, by, i, 1)) found1 = false;
			if (combobuf[MAPCOMBO2(i, bx2, by)].type == cBRIDGE && _effectflag_layer(bx2, by, i, 1)) found2 = false;
		}
	}


	// Layers
	if (!(found1 || found2))
	{
		foundlayer = -1;
		for (int32_t i = 0; i < 2; i++)
		{
			cid1 = MAPCOMBO2(i, bx, by);
			cid2 = MAPCOMBO2(i, bx2, by);
			newcombo const& cmb = combobuf[cid1];
			newcombo const& cmb2 = combobuf[cid2];
			if (i == 0)
			{
				if (tmpscr2[1].valid != 0)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[cid1].type == cBRIDGE && !_walkflag_layer(bx, by, 1, &(tmpscr2[1]))) continue;
						if (combobuf[cid2].type == cBRIDGE && !_walkflag_layer(bx2, by, 1, &(tmpscr2[1]))) continue;
					}
					else
					{
						if (combobuf[cid1].type == cBRIDGE && _effectflag_layer(bx, by, 1, &(tmpscr2[1]))) continue;
						if (combobuf[cid2].type == cBRIDGE && _effectflag_layer(bx2, by, 1, &(tmpscr2[1]))) continue;
					}
				}
			}
			if (cmb.type == cBOSSLOCKBLOCK && !(cmb.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx, by, 1, i))
			{
				found1 = true;
				foundlayer = i;
				break;
			}
			else if (cmb2.type == cBOSSLOCKBLOCK && !(cmb2.triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2, by, 1, i))
			{
				found2 = true;
				foundlayer = i;
				break;
			}
		}
	}

	if (!(found1 || found2) || pushing < 8)
	{
		return;
	}
	int32_t cid = found1 ? cid1 : cid2;
	
	if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
	
	
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
		int i = key_item;
		FFCore.reset_script_engine_data(ScriptType::Item, i);
		ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
		FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
	}
	
	if(cmb.usrflags&cflag16)
	{
		setxmapflag(cmb_screen_index, 1<<cmb.attribytes[5]);
		remove_xstatecombos(get_layer_scr(currmap, cmb_screen_index, 0), cmb_screen_index, 1<<cmb.attribytes[5]);
	}
	else
	{
		// TODO z3
		setmapflag(mBOSSLOCKBLOCK);
		remove_bosslockblocks(tmpscr, currscr);
	}
	if ( (combobuf[cid].attribytes[3]) )
		sfx(combobuf[cid].attribytes[3]);
}

void HeroClass::oldcheckchest(int32_t type)
{
	// chests aren't affected by tmpscr->flags2&fAIRCOMBOS
	if(toogam || z>0 || fakez > 0) return;
	if(pushing<8) return;

	int32_t bx = TRUNCATE_TILE(x.getInt());
	int32_t bx2 = TRUNCATE_TILE(x.getInt()+8);
	int32_t by = TRUNCATE_TILE(y.getInt());
	
	switch(dir)
	{
		case up:
			if(isSideViewHero()) return;
			
			if(!((int32_t)y&15)&&y!=0) by-=bigHitbox ? 16 : 0;
			
			break;
			
		case left:
		case right:
			if(isSideViewHero()) break;
			[[fallthrough]];
		case down:
			return;
	}
	
	bool found=false;
	int found_screen_index=0;
	bool itemflag=false;
	
	if (combobuf[MAPCOMBO(bx,by)].type==type && _effectflag(bx,by,1, -1))
	{
		found=true;
		found_screen_index=get_screen_index_for_world_xy(bx, by);
	}
	else if (combobuf[MAPCOMBO(bx2,by)].type==type && _effectflag(bx2,by,1, -1))
	{
		found=true;
		found_screen_index=get_screen_index_for_world_xy(bx2, by);
	}
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,i)) found = false;
			if (combobuf[MAPCOMBO2(i,bx2,by)].type == cBRIDGE && !_walkflag_layer(bx2,by,i)) found = false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,i)) found = false;
			if (combobuf[MAPCOMBO2(i,bx2,by)].type == cBRIDGE && _effectflag_layer(bx2,by,i)) found = false;
		}
	}
	
	if(!found)
	{
		for(int32_t i=0; i<2; i++)
		{
			if (i == 0)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS))
				{
					if (combobuf[MAPCOMBO2(1,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,1)) continue;
					if (combobuf[MAPCOMBO2(1,bx2,by)].type == cBRIDGE && !_walkflag_layer(bx2,by,1)) continue;
				}
				else
				{
					if (combobuf[MAPCOMBO2(1,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,1)) continue;
					if (combobuf[MAPCOMBO2(1,bx2,by)].type == cBRIDGE && _effectflag_layer(bx2,by,1)) continue;
				}
			}
			if (combobuf[MAPCOMBO2(i,bx,by)].type==type && _effectflag(bx,by,1, i))
			{
				found=true;
				found_screen_index=get_screen_index_for_world_xy(bx, by);
				break;
			}
			else if (combobuf[MAPCOMBO2(i,bx2,by)].type==type && _effectflag(bx2,by,1, i))
			{
				found=true;
				found_screen_index=get_screen_index_for_world_xy(bx2, by);
				break;
			}
		}
	}
	
	if(!found)
	{
		return;
	}

	mapscr* screen = get_scr(currmap, found_screen_index);
	
	switch(type)
	{
		case cLOCKEDCHEST:
			if(!usekey()) return;
			
			setmapflag(screen, found_screen_index, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			setmapflag(screen, found_screen_index, mCHEST);
			break;
			
		case cBOSSCHEST:
			if(!(game->lvlitems[dlevel]&liBOSSKEY)) return;
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
				int i = key_item;
				FFCore.reset_script_engine_data(ScriptType::Item, i);
				ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
				FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
			}
			setmapflag(screen, found_screen_index, mBOSSCHEST);
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
		for(int32_t i=0; i<2; i++)
		{
			itemflag |= MAPFLAG2(i,bx,by)==mfARMOS_ITEM;
			itemflag |= MAPFLAG2(i,bx2,by)==mfARMOS_ITEM;
			itemflag |= MAPCOMBOFLAG2(i,bx,by)==mfARMOS_ITEM;
			itemflag |= MAPCOMBOFLAG2(i,bx2,by)==mfARMOS_ITEM;
		}
	}
	
	if(itemflag && !getmapflag(found_screen_index, (found_screen_index < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM))
	{
		add_item_for_screen(found_screen_index, new item(x, y,(zfix)0, screen->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((screen->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
	}
}

void HeroClass::checkchest(int32_t type)
{
	bool ischest = type == cCHEST || type == cLOCKEDCHEST || type == cBOSSCHEST;
	bool islockblock = type == cLOCKBLOCK || type == cBOSSLOCKBLOCK;
	bool islocked = type == cLOCKBLOCK || type == cLOCKEDCHEST;
	bool isbosslocked = type == cBOSSLOCKBLOCK || type == cBOSSCHEST;
	if(ischest)
	{
		if(get_qr(qr_OLD_CHEST_COLLISION))
		{
			oldcheckchest(type);
			return;
		}
	}
	if(islockblock && get_qr(qr_OLD_LOCKBLOCK_COLLISION))
	{
		if(type == cLOCKBLOCK)
			oldchecklockblock();
		else if(type == cBOSSLOCKBLOCK)
			oldcheckbosslockblock();
		return;
	}
	if(toogam || z>0 || fakez > 0) return;
	zfix bx, by;
	zfix bx2, by2;
	zfix fx(-1), fy(-1);
	switch(dir)
	{
		case up:
			by = y + (bigHitbox ? -2 : 6);
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case down:
			by = y + 17;
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case left:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x - 2;
			bx2 = x - 2;
			break;
		case right:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x + 17;
			bx2 = x + 17;
			break;
	}
	
	int32_t found = -1;
	int32_t foundlayer = 0;
	
	newcombo const* cmb = &combobuf[MAPCOMBO(bx,by)];
	
	if(cmb->type==type && !(cmb->triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx,by,1, -1))
	{
		found = MAPCOMBO(bx,by);
		fx = bx; fy = by;
		for (int32_t i = 0; i <= 1; ++i)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,1)) found = -1;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,1)) found = -1;
			}
		}
	}
	if(found<0)
	{
		cmb = &combobuf[MAPCOMBO(bx2,by2)];
		if(cmb->type==type && !(cmb->triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2,by2,1, -1))
		{
			found = MAPCOMBO(bx2,by2);
			for (int32_t i = 0; i < 6; ++i)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS))
				{
					if (combobuf[MAPCOMBO2(i,bx2,by2)].type == cBRIDGE && !_walkflag_layer(bx2,by2,i))
					{
						found = -1;
						break;
					}
				}
				else
				{
					if (combobuf[MAPCOMBO2(i,bx2,by2)].type == cBRIDGE && _effectflag_layer(bx2,by2,i))
					{
						found = -1;
						break;
					}
				}
			}
			if(found != -1)
			{
				fx = bx2; fy = by2;
			}
		}
	}
	
	if(found<0)
	{
		for(int32_t i=0; i<6; i++)
		{
			cmb = &combobuf[MAPCOMBO2(i,bx,by)];
			if(combobuf[MAPCOMBO2(i,bx,by)].type==type && !(cmb->triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx,by,1, i))
			{
				found = MAPCOMBO2(i,bx,by);
				for(int32_t j = i+1; j < 6; ++j)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(j,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,j))
						{
							found = -1;
							break;
						}
					}
					else
					{
						if (combobuf[MAPCOMBO2(j,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,j))
						{
							found = -1;
							break;
						}
					}
				}
				if(found>-1)
				{
					foundlayer = i+1;
					fx = bx; fy = by;
					break;
				}
			}
			cmb = &combobuf[MAPCOMBO2(i,bx2,by2)];
			if(combobuf[MAPCOMBO2(i,bx2,by2)].type==type && !(cmb->triggerflags[0] & combotriggerONLYGENTRIG) && _effectflag(bx2,by2,1, i))
			{
				found = MAPCOMBO2(i,bx2,by2);
				for(int32_t j = i+1; j < 6; ++j)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(j,bx2,by2)].type == cBRIDGE && !_walkflag_layer(bx2,by2,j))
						{
							found = -1;
							break;
						}
					}
					else
					{
						if (combobuf[MAPCOMBO2(j,bx2,by2)].type == cBRIDGE && _effectflag_layer(bx2,by2,j))
						{
							found = -1;
							break;
						}
					}
				}
				if(found>-1)
				{
					foundlayer = i+1;
					fx = bx2; fy = by2;
					break;
				}
			}
		}
	}
	
	if(found<0) return;
	cmb = &combobuf[found];
	switch(dir)
	{
		case up:
			if(cmb->usrflags&cflag10)
				return;
			break;
		case down:
			if(cmb->usrflags&cflag9)
				return;
			break;
		case left:
			if(cmb->usrflags&cflag12)
				return;
			break;
		case right:
			if(cmb->usrflags&cflag11)
				return;
			break;
	}
	int32_t intbtn = cmb->attribytes[2];
	
	if(intbtn) //
	{
		if(cmb->usrflags & cflag13) //display prompt
		{
			int altcmb = cmb->attributes[2]/10000;
			prompt_combo = cmb->attributes[1]/10000;
			if(altcmb && ((islocked && !can_locked_combo(*cmb))
				|| (isbosslocked && !(game->lvlitems[dlevel]&liBOSSKEY))))
				prompt_combo = altcmb;
			prompt_cset = cmb->attribytes[4];
			prompt_x = cmb->attrishorts[0];
			prompt_y = cmb->attrishorts[1];
		}
		if(!getIntBtnInput(intbtn, true, true, false, false))
		{
			return; //Button not pressed
		}
	}
	else if(pushing < 8 || pushing % 8) return; //Not pushing against chest enough
	
	if(ischest)
	{
		if (!trigger_chest(get_rpos_handle_for_world_xy(fx, fy, foundlayer))) return;
	}
	else if(islockblock)
	{
		if (!trigger_lockblock(get_rpos_handle_for_world_xy(fx, fy, foundlayer))) return;
	}
	if(intbtn && (cmb->usrflags & cflag13))
		prompt_combo = 0;
}

void HeroClass::checkgenpush(rpos_t rpos)
{
	for (int layer = 0; layer < 7; ++layer)
	{
		auto rpos_handle = get_rpos_handle(rpos, layer);
		newcombo const& cmb = combobuf[rpos_handle.data()];
		if (cmb.triggerflags[1] & combotriggerPUSH)
			do_trigger_combo(rpos_handle);
	}
}

void HeroClass::checkgenpush()
{
	if(pushing < 8 || pushing % 8) return;
	zfix bx, by;
	zfix bx2, by2;
	switch(dir)
	{
		case up:
			by = y + (bigHitbox ? -2 : 6);
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case down:
			by = y + 17;
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case left:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x - 2;
			bx2 = x - 2;
			break;
		case right:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x + 17;
			bx2 = x + 17;
			break;
	}

	rpos_t rpos_1 = COMBOPOS_REGION(bx, by);
	rpos_t rpos_2 = COMBOPOS_REGION(bx2, by2);
	checkgenpush(rpos_1);
	if (rpos_1 != rpos_2) checkgenpush(rpos_2);

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_some_ffcs_in_region([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by) || ffcIsAt(ffc_handle, bx2, by2))
			{
				newcombo const& cmb3 = combobuf[ffc_handle.data()];
				if(cmb3.triggerflags[1] & combotriggerPUSH)
				{
					do_trigger_combo_ffc(ffc_handle);
					return false;
				}
			}
			return true;
		});
	}
}

void HeroClass::checksigns() //Also checks for generic trigger buttons
{
	if(toogam || z>0 || fakez>0) return;
	if(msg_active || (msg_onscreen && get_qr(qr_MSGDISAPPEAR)))
		return; //Don't overwrite a message waiting to be dismissed
	zfix bx, by;
	zfix bx2, by2;
	zfix fx(-1), fy(-1);
	switch(dir)
	{
		case up:
			by = y + (bigHitbox ? -2 : 6);
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case down:
			by = y + 17;
			by2 = by;
			bx = x + 4;
			bx2 = bx + 8;
			break;
		case left:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x - 2;
			bx2 = x - 2;
			break;
		case right:
			by = y + (bigHitbox ? 0 : 8);
			by2 = y + 8;
			bx = x + 17;
			bx2 = x + 17;
			break;
	}
	
	int32_t found = -1;
	int32_t found_scr = -1;
	std::optional<ffc_handle_t> foundffc;
	int32_t found_lyr = 0;
	bool found_sign = false;
	int32_t tmp_cid = MAPCOMBO(bx, by);
	int32_t scr = get_screen_index_for_world_xy(bx, by);
	newcombo const* tmp_cmb = &combobuf[tmp_cid];
	if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->triggerflags[0] & combotriggerONLYGENTRIG))
		|| tmp_cmb->triggerbtn) && _effectflag(bx,by,1, -1))
	{
		found = tmp_cid;
		found_scr = scr;
		fx = bx; fy = by;
		for (int32_t i = 0; i <= 1; ++i)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,i)) found = -1;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,i)) found = -1;
			}
		}
	}
	tmp_cid = MAPCOMBO(bx2,by2);
	scr = get_screen_index_for_world_xy(bx2, by2);
	tmp_cmb = &combobuf[tmp_cid];
	if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->triggerflags[0] & combotriggerONLYGENTRIG))
		|| tmp_cmb->triggerbtn) && _effectflag(bx2,by2,1, -1))
	{
		found = tmp_cid;
		found_scr = scr;
		fx = bx2; fy = by2;
		for (int32_t i = 0; i <= 1; ++i)
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i,bx2,by2)].type == cBRIDGE && !_walkflag_layer(bx2,by2,i)) found = -1;
			}
			else
			{
				if (combobuf[MAPCOMBO2(i,bx2,by2)].type == cBRIDGE && _effectflag_layer(bx2,by2,i)) found = -1;
			}
		}
	}
	
	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		foundffc = find_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by) || ffcIsAt(ffc_handle, bx2, by2))
			{
				tmp_cmb = &combobuf[ffc_handle.data()];
				if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->triggerflags[0] & combotriggerONLYGENTRIG))
				|| tmp_cmb->triggerbtn) && true) //!TODO: FFC effect flag?
				{
					return true;
				}
			}

			return false;
		});
	}
	
	if(found<0 && !foundffc)
	{
		scr = get_screen_index_for_world_xy(bx, by);
		for(int32_t i=0; i<6; i++)
		{
			tmp_cid = MAPCOMBO2(i,bx,by);
			tmp_cmb = &combobuf[tmp_cid];
			if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->triggerflags[0] & combotriggerONLYGENTRIG))
				|| tmp_cmb->triggerbtn) && _effectflag(bx,by,1, i))
			{
				found = tmp_cid;
				found_scr = scr;
				found_lyr = i+1;
				fx = bx; fy = by;
				if (i == 0)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(1,bx,by)].type == cBRIDGE && !_walkflag_layer(bx,by,1)) found = -1;
					}
					else
					{
						if (combobuf[MAPCOMBO2(1,bx,by)].type == cBRIDGE && _effectflag_layer(bx,by,1)) found = -1;
					}
				}
			}
			scr = get_screen_index_for_world_xy(bx2, by2);
			tmp_cid = MAPCOMBO2(i,bx2,by2);
			tmp_cmb = &combobuf[tmp_cid];
			if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->triggerflags[0] & combotriggerONLYGENTRIG))
				|| tmp_cmb->triggerbtn) && _effectflag(bx2,by2,1, i))
			{
				found = tmp_cid;
				found_scr = scr;
				found_lyr = i+1;
				fx = bx2; fy = by2;
				if (i == 0)
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(1,bx2,by2)].type == cBRIDGE && !_walkflag_layer(bx2,by2,1)) found = -1;
					}
					else
					{
						if (combobuf[MAPCOMBO2(1,bx2,by2)].type == cBRIDGE && _effectflag_layer(bx2,by2,1)) found = -1;
					}
				}
			}
			if(found>-1) break;
		}
	}
	
	if(found<0&&!foundffc) return;

	newcombo const& cmb = combobuf[foundffc ? foundffc->data() : found];
	int offx = 0;
	int offy = 0;
	if (!foundffc)
		std::tie(offx, offy) = translate_screen_coordinates_to_world(scr);
	
	byte signInput = 0;
	bool didsign = false, didprompt = false;
	if(cmb.type == cSIGNPOST && !(cmb.triggerflags[0] & combotriggerONLYGENTRIG))
	{
		switch(dir)
		{
			case up:
				if(cmb.usrflags&cflag10)
					goto endsigns;
				break;
			case down:
				if(cmb.usrflags&cflag9)
					goto endsigns;
				break;
			case left:
				if(cmb.usrflags&cflag12)
					goto endsigns;
				break;
			case right:
				if(cmb.usrflags&cflag11)
					goto endsigns;
				break;
		}
		int32_t intbtn = cmb.attribytes[2];
		
		if(intbtn) //
		{
			signInput = getIntBtnInput(intbtn, true, true, false, false);
			if(!signInput)
			{
				if(cmb.usrflags & cflag13) //display prompt
				{
					prompt_combo = cmb.attributes[1]/10000;
					prompt_cset = cmb.attribytes[4];
					prompt_x = cmb.attrishorts[0] + offx;
					prompt_y = cmb.attrishorts[1] + offy;
					didprompt = true;
				}
				goto endsigns; //Button not pressed
			}
		}
		else if(pushing < 8 || pushing%8) goto endsigns; //Not pushing against sign enough
		
		trigger_sign(cmb, found_scr);
		didsign = true;
	}
endsigns:
	if (!foundffc && on_cooldown(get_rpos_handle_for_world_xy(fx, fy, found_lyr))) return;

	switch(dir)
	{
		case down:
			if(!(cmb.triggerflags[0] & combotriggerBTN_TOP))
				return;
			break;
		case up:
			if(!(cmb.triggerflags[0] & combotriggerBTN_BOTTOM))
				return;
			break;
		case right:
			if(!(cmb.triggerflags[0] & combotriggerBTN_LEFT))
				return;
			break;
		case left:
			if(!(cmb.triggerflags[0] & combotriggerBTN_RIGHT))
				return;
			break;
	}
	if(cmb.triggerbtn && (getIntBtnInput(cmb.triggerbtn, true, true, false, false) || checkIntBtnVal(cmb.triggerbtn, signInput)))
	{
		if (foundffc)
			do_trigger_combo_ffc(foundffc.value(), didsign ? ctrigIGNORE_SIGN : 0);
		else 
			do_trigger_combo(get_rpos_handle_for_world_xy(fx, fy, found_lyr), didsign ? ctrigIGNORE_SIGN : 0);
	}
	else if(didprompt)
		return;
	else if(cmb.type == cBUTTONPROMPT)
	{
		prompt_combo = cmb.attributes[0]/10000;
		prompt_cset = cmb.attribytes[0];
		prompt_x = cmb.attrishorts[0] + offx;
		prompt_y = cmb.attrishorts[1] + offy;
	}
	else if(cmb.prompt_cid)
	{
		prompt_combo = cmb.prompt_cid;
		prompt_cset = cmb.prompt_cs;
		prompt_x = cmb.prompt_x + offx;
		prompt_y = cmb.prompt_y + offy;
	}
}

void HeroClass::checklocked()
{
	if(toogam) return; //Walk through walls. 
	
	if(!isdungeon()) return;
	
	if( !diagonalMovement && pushing!=8) return;
	/*This is required to allow the player to open a door, while sliding along a wall (pressing in the direction of the door, and sliding left or right)
	*/
	if ( diagonalMovement && pushing < 8 ) return; //Allow wall walking Should I add a quest rule for this? -Z
	
	
	bool found = false;
	for ( int32_t q = 0; q < 4; q++ ) {
		if ( tmpscr->door[q] == dLOCKED || tmpscr->door[q] == dBOSS ) { found = true; }
	}
	
	if ( !found ) return;
	
	int32_t si = (currmap<<7) + currscr;
	int32_t di = 0;
	
	
	
	if ( diagonalMovement || get_qr(qr_DISABLE_4WAY_GRIDLOCK)) 
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
					setmapflag_mi(si, mDOOR_UP);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_DOWN);
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
					setmapflag_mi(si, mDOOR_UP);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_DOWN);
					sfx(WAV_DOOR);
					markBmap(-1);
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
						int i = key_item;
						FFCore.reset_script_engine_data(ScriptType::Item, i);
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
						FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
					}
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
					setmapflag_mi(si, mDOOR_DOWN);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_UP);
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
					setmapflag_mi(si, mDOOR_DOWN);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_UP);
					sfx(WAV_DOOR);
					markBmap(-1);
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
						int i = key_item;
						FFCore.reset_script_engine_data(ScriptType::Item, i);
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
						FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
					}
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
					setmapflag_mi(si, mDOOR_LEFT);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_RIGHT);
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
					setmapflag_mi(si, mDOOR_LEFT);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_RIGHT);
					sfx(WAV_DOOR);
					markBmap(-1);
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
						int i = key_item;
						FFCore.reset_script_engine_data(ScriptType::Item, i);
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
						FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
					}
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
					setmapflag_mi(si, mDOOR_RIGHT);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_LEFT);
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
					setmapflag_mi(si, mDOOR_RIGHT);
					
					if(di != 0xFFFF)
						setmapflag_mi(di, mDOOR_LEFT);
					sfx(WAV_DOOR);
					markBmap(-1);
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
						int i = key_item;
						FFCore.reset_script_engine_data(ScriptType::Item, i);
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
						FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
					}
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
						setmapflag_mi(si, mDOOR_UP);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_DOWN);
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
						setmapflag_mi(si, mDOOR_UP);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_DOWN);
						sfx(WAV_DOOR);
						markBmap(-1);
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
							int i = key_item;
							FFCore.reset_script_engine_data(ScriptType::Item, i);
							ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
							FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
						}
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
						setmapflag_mi(si, mDOOR_DOWN);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_UP);
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
						setmapflag_mi(si, mDOOR_DOWN);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_UP);
						sfx(WAV_DOOR);
						markBmap(-1);
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
							int i = key_item;
							FFCore.reset_script_engine_data(ScriptType::Item, i);
							ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
							FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
						}
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
						setmapflag_mi(si, mDOOR_LEFT);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_RIGHT);
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
						setmapflag_mi(si, mDOOR_LEFT);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_RIGHT);
						sfx(WAV_DOOR);
						markBmap(-1);
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
							int i = key_item;
							FFCore.reset_script_engine_data(ScriptType::Item, i);
							ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
							FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
						}
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
						setmapflag_mi(si, mDOOR_RIGHT);
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_LEFT);
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
						setmapflag_mi(si, mDOOR_RIGHT);
						
						
						if(di != 0xFFFF)
							setmapflag_mi(di, mDOOR_LEFT);
						sfx(WAV_DOOR);
						markBmap(-1);
						// Run Boss Key Script
						int32_t key_item = 0; //current_item_id(itype_bosskey); //not possible
						for ( int32_t q = 0; q < MAXITEMS; ++q )
						{
							if ( itemsbuf[q].family == itype_bosskey )
							{
								key_item = q; break;
							}
						}
						if ( key_item > 0 && itemsbuf[key_item].script && !(FFCore.doscript(ScriptType::Item, key_item) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ) //
						{
							int i = key_item;
							FFCore.reset_script_engine_data(ScriptType::Item, i);
							ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[i].script, i);
							FFCore.deallocateAllScriptOwned(ScriptType::Item,(key_item));
						}

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

void HeroClass::checkswordtap()
{
	if(attack!=wSword || charging<=0 || pushing<8) return;
	
	int32_t bx=x;
	int32_t by=y+8;
	
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
	
	if(!_walkflag(bx,by,0,SWITCHBLOCK_STATE)) return;
	
	attackclk=SWORDTAPFRAME;
	pushing=-8; //16 frames between taps
	tapping=true;
	
	int32_t type = COMBOTYPE(bx,by);
	
	if(!isCuttableType(type))
	{
		bool hollow = (MAPFLAG(bx,by) == mfBOMB || MAPCOMBOFLAG(bx,by) == mfBOMB ||
					   MAPFLAG(bx,by) == mfSBOMB || MAPCOMBOFLAG(bx,by) == mfSBOMB);
					   
		// Layers
		for(int32_t i=0; i < 6; i++)
			hollow = (hollow || MAPFLAG2(i,bx,by) == mfBOMB || MAPCOMBOFLAG2(i,bx,by) == mfBOMB ||
					  MAPFLAG2(i,bx,by) == mfSBOMB || MAPCOMBOFLAG2(i,bx,by) == mfSBOMB);
					  
		for(int32_t i=0; i<4; i++)
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
				
		sfx(hollow ? WAV_ZN1TAP2 : WAV_ZN1TAP,pan(x.getInt()));
	}
	
}

void HeroClass::fairycircle(int32_t type)
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
		if (IsSideSwim()) {action=sideswimfreeze; FFCore.setHeroAction(sideswimfreeze);} 
		else {action=freeze; FFCore.setHeroAction(freeze);} 
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
		action=none; FFCore.setHeroAction(none);
		fairyclk=0;
		holdclk=0;
		refill_why = 0;
		refilling=REFILL_NONE;
		map_bkgsfx(true);
	}
}

int32_t touchcombo(int32_t x,int32_t y)
{
	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && !_walkflag_layer(x,y,i)) return 0;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,x,y)].type == cBRIDGE && _effectflag_layer(x,y,i)) return 0;
		}
	}
	if (!_effectflag(x,y,1, -1)) return 0;
	newcombo const& cmb = combobuf[MAPCOMBO(x,y)];
	if(cmb.triggerflags[0] & combotriggerONLYGENTRIG)
		return 0;
	switch(cmb.type)
	{
		case cBSGRAVE:
		case cGRAVE:
			if(MAPFLAG(x,y)||MAPCOMBOFLAG(x,y)) //!DIMITODO: all flags break graves, not just push flags
			{
				break;
			}

			[[fallthrough]];
		case cARMOS:
		{
			return cmb.type;
		}
	}
    
	return 0;
}

static int32_t GridX(int32_t x) 
{
	return (x >> 4) << 4;
}

//Snaps 'y' to the combo grid
//Equivalent to calling ComboY(ComboAt(foo,y));
static int32_t GridY(int32_t y) 
{
	return (y >> 4) << 4;
}

int32_t grabComboFromPos(int32_t pos, int32_t type)
{
	int x = COMBOX_REGION_EXTENDED(pos);
	int y = COMBOY_REGION_EXTENDED(pos);
	int normalpos = COMBOPOS(x%256, y%176);
	int scr = get_screen_index_for_world_xy(x, y);

	for(int32_t lyr = 6; lyr >= 0; --lyr)
	{
		int32_t id = get_layer_scr(currmap, scr, lyr - 1)->data[normalpos];
		if(combobuf[id].type == type)
			return id;
	}
	return -1;
}

typedef word spot_t;
static std::vector<int32_t> typeMap;
static std::vector<int32_t> customTypeMap;
static std::vector<int32_t> istrig;
static std::map<int32_t, std::map<size_t, word>> MAPS_prism_dir_seen_map;
static int32_t heropos = -1;
static const int32_t SPTYPE_SOLID = -1;
enum
{
	beamoffs_gr, beamoffs_up, beamoffs_down, beamoffs_left, beamoffs_right,
	beamoffs_uleft, beamoffs_uright, beamoffs_dleft, beamoffs_dright, beamoffs_vert,
	beamoffs_horz, beamoffs_notup, beamoffs_notdown, beamoffs_notleft, beamoffs_notright,
	beamoffs_all, beamoffs_max
};
struct lightbeam_xy
{
	int16_t x;
	int16_t y;
	lightbeam_xy(int32_t nx, int32_t ny)
	{
		x = vbound(nx,-32768,32767);
		y = vbound(ny,-32768,32767);
	}
	lightbeam_xy(dword ffpos)
	{
		x = int16_t(ffpos >> 16);
		y = int16_t(ffpos & 0xFFFF);
	}
	bool valid() const
	{
		return valid(x,y);
	}
	void bound()
	{
		x = vbound(x,0-16,world_w-1+16);
		y = vbound(y,0-16,world_h-1+16);
	}
	dword ffpos() const
	{
		return (word(x)<<16)|word(y&0xFFFF);
	}
	size_t pos() const
	{
		return (size_t)COMBOPOS_REGION(vbound(x,0,world_w-1),vbound(y,0,world_h-1));
	}
	bool herocollide(byte beamwid)
	{
		int bx = x-beamwid/2, by = y-beamwid/2;
		int hx = Hero.x.getInt(), hy = Hero.y.getInt();
		return hx+15 >= bx && hx < bx+beamwid
			&& hy+15 >= by && hy < by+beamwid;
	}
	static bool valid(int32_t x, int32_t y)
	{
		return x+16 >= 0 && x-16 < world_w && y+16 >= 0 && y-16 < world_h;
	}
};
#define SP_VISITED       0x1
#define SP_FLAGS         0x01E
#define SP_GOFLAGS       0x1E0
#define SP_MASK          (SP_VISITED|SP_FLAGS)
#define SP_FLAG(dir)     (0x2<<dir)
#define SP_GOFLAG(dir)   (0x20<<dir)
#define BEAM_AGE_LIMIT   512
static void handleBeam(spot_t* grid, size_t age, byte spotdir, int32_t curpos, byte set, bool block, bool refl, std::map<size_t, word>& prism_dir_seen_map)
{
	if(spotdir > 3) return; //invalid dir

	int map_size = region_scr_width * 16 * region_scr_height * 11;
	int combos_wide = region_scr_width  * 16;
	int32_t trigflag = set ? (1 << (set-1)) : ~0;
	bool doAge = true;
	spot_t f = 0;
	while(unsigned(curpos) < map_size)
	{
		bool block_light = false;
		f = SP_GOFLAG(spotdir);
		if((grid[curpos] & f) == f)
			return;
		else grid[curpos] |= f;
		f = SP_FLAG(spotdir);
		if((grid[curpos] & f) != f)
		{
			grid[curpos] |= f;
			istrig[curpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		switch(spotdir)
		{
			case up:
				curpos -= combos_wide;
				break;
			case down:
				curpos += combos_wide;
				break;
			case left:
				if(!(curpos%combos_wide))
					curpos = -1;
				else --curpos;
				break;
			case right:
				++curpos;
				if(!(curpos%combos_wide))
					curpos = -1;
				break;
		}
		if(unsigned(curpos) >= map_size) return;
		switch(typeMap[curpos])
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				curpos = -1;
				break;
		}
		if((curpos==heropos) && block && (spotdir == oppositeDir[Hero.getDir()]))
			curpos = -1;
		if(unsigned(curpos) >= map_size) return;
		
		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[curpos] & f) != f)
		{
			grid[curpos] |= f;
			istrig[curpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		if(doAge)
		{
			if(++age > BEAM_AGE_LIMIT)
				return;
		}
		else doAge = true;
		
		if(curpos==heropos && refl)
			spotdir = Hero.getDir();
		else switch(typeMap[curpos])
		{
			case cLIGHTTARGET:
			{
				auto cid = grabComboFromPos(curpos, cLIGHTTARGET);
				if(cid > -1 && combobuf[cid].usrflags&cflag3) //Blocks light
					return;
				if(get_qr(qr_BROKEN_LIGHTBEAM_HITBOX))
					spotdir = oppositeDir[spotdir];
				break;
			}
			case cMIRROR:
				spotdir = oppositeDir[spotdir];
				break;
			case cMIRRORSLASH:
				switch(spotdir)
				{
					case up:
						spotdir = right; break;
					case right:
						spotdir = up; break;
					case down:
						spotdir = left; break;
					case left:
						spotdir = down; break;
				}
				break;
			case cMIRRORBACKSLASH:
				switch(spotdir)
				{
					case up:
						spotdir = left; break;
					case left:
						spotdir = up; break;
					case down:
						spotdir = right; break;
					case right:
						spotdir = down; break;
				}
				break;
			case cMAGICPRISM:
				if (prism_dir_seen_map[curpos] & (1<<spotdir)) return;
				if (prism_dir_seen_map[curpos]) prism_dir_seen_map[curpos] = 0xF;
				else                            prism_dir_seen_map[curpos] |= 1<<spotdir;
				for(byte d = 0; d < 4; ++d)
				{
					if(d == oppositeDir[spotdir]) continue;
					handleBeam(grid, age, d, curpos, set, block, refl, prism_dir_seen_map);
				}
				return;
			case cMAGICPRISM4:
			{
				if (prism_dir_seen_map[curpos]) return;
				prism_dir_seen_map[curpos] = 0xF;
				for(byte d = 0; d < 4; ++d)
				{
					handleBeam(grid, age, d, curpos, set, block, refl, prism_dir_seen_map);
				}
				return;
			}
			case cMIRRORNEW:
			{
				auto cid = customTypeMap[curpos];
				if(unsigned(cid) >= MAXCOMBOS) break;
				newcombo const& cmb = combobuf[cid];
				byte newdir = cmb.attribytes[spotdir];
				if(newdir > 3) break;
				spotdir = newdir;
				break;
			}
		}
	}
}

static void handleFFBeam(std::map<dword,spot_t>& grid, size_t age, byte spotdir, lightbeam_xy curxy, byte set, bool block, bool refl, byte beamwid)
{
	if(spotdir > 3) return; //invalid dir
	int32_t trigflag = set ? (1 << (set-1)) : ~0;
	bool doAge = true;
	byte f = 0;
	while(curxy.valid())
	{
		bool block_light = false;
		f = SP_FLAG(spotdir);
		if((grid[curxy.ffpos()] & f) != f)
		{
			grid[curxy.ffpos()] |= f;
			istrig[curxy.pos()] |= trigflag;
			doAge = false;
			age = 0;
		}
		switch(spotdir)
		{
			case up:
				curxy.y -= 16;
				break;
			case down:
				curxy.y += 16;
				break;
			case left:
				curxy.x -= 16;
				break;
			case right:
				curxy.x += 16;
				break;
		}
		auto curpos = curxy.pos();
		switch(typeMap[curpos])
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				return;
		}
		bool collided_hero = heropos > -1 && curxy.herocollide(beamwid);
		if(block && (spotdir == oppositeDir[Hero.getDir()]) && collided_hero)
			return;
		
		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[curxy.ffpos()] & f) != f)
		{
			grid[curxy.ffpos()] |= f;
			istrig[curpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		if(!curxy.valid()) return;
		if(doAge)
		{
			if(++age > BEAM_AGE_LIMIT)
				return;
		}
		else doAge = true;
		
		if(refl && collided_hero)
			spotdir = Hero.getDir();
		else switch(typeMap[curpos])
		{
			case cLIGHTTARGET:
			{
				auto cid = grabComboFromPos(curpos, cLIGHTTARGET);
				if(cid > -1 && combobuf[cid].usrflags&cflag3) //Blocks light
					return;
				if(get_qr(qr_BROKEN_LIGHTBEAM_HITBOX))
					spotdir = oppositeDir[spotdir];
				break;
			}
			case cMIRROR:
				spotdir = oppositeDir[spotdir];
				break;
			case cMIRRORSLASH:
				switch(spotdir)
				{
					case up:
						spotdir = right; break;
					case right:
						spotdir = up; break;
					case down:
						spotdir = left; break;
					case left:
						spotdir = down; break;
				}
				break;
			case cMIRRORBACKSLASH:
				switch(spotdir)
				{
					case up:
						spotdir = left; break;
					case left:
						spotdir = up; break;
					case down:
						spotdir = right; break;
					case right:
						spotdir = down; break;
				}
				break;
			case cMAGICPRISM:
				for(byte d = 0; d < 4; ++d)
				{
					if(d == oppositeDir[spotdir]) continue;
					handleFFBeam(grid, age, d, curxy, set, block, refl, beamwid);
				}
				return;
			case cMAGICPRISM4:
				for(byte d = 0; d < 4; ++d)
				{
					handleFFBeam(grid, age, d, curxy, set, block, refl, beamwid);
				}
				return;
			case cMIRRORNEW:
			{
				auto cid = customTypeMap[curpos];
				if(unsigned(cid) >= MAXCOMBOS) break;
				newcombo const& cmb = combobuf[cid];
				byte newdir = cmb.attribytes[spotdir];
				if(newdir > 3) break;
				spotdir = newdir;
				break;
			}
		}
	}
}

static bool launch_lightbeam(newcombo const& cmb, int32_t pos,
	std::map<int32_t, spot_t*>& maps, bool refl, bool block)
{
	//Positive ID is a tile, negative is a color trio. 0 is nil in either case.
	int32_t id = (cmb.usrflags&cflag1)
		? std::max(0,cmb.attributes[0]/10000)|(cmb.attribytes[1]%12)<<24
		: -((cmb.attribytes[3]<<16)|(cmb.attribytes[2]<<8)|(cmb.attribytes[1]));
	if(!id) return false;
	//Get the grid array for this tile/color
	spot_t* grid;
	if(maps[id])
		grid = maps[id];
	else
	{
		int map_size = region_scr_width * 16 * region_scr_height * 11;
		grid = new spot_t[map_size];
		memset(grid, 0, sizeof(spot_t)*map_size);
		maps[id] = grid;
	}
	byte spotdir = cmb.attribytes[0];
	if(spotdir > 3)
	{
		grid[pos] |= SP_VISITED;
		istrig[pos] |= cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
	}
	if(refl && pos == heropos)
	{
		spotdir = Hero.getDir();
	}
	handleBeam(grid, 0, spotdir, pos, cmb.attribytes[4], block, refl, MAPS_prism_dir_seen_map[id]);
	return true;
}

static bool launch_fflightbeam(ffcdata const& ffc,
	std::map<int32_t, std::map<dword,spot_t>>& ffmaps, bool refl, bool block)
{
	newcombo const& cmb = combobuf[ffc.getData()];
	//Positive ID is a tile, negative is a color trio. 0 is nil in either case.
	int32_t id = (cmb.usrflags&cflag1)
		? std::max(0,cmb.attributes[0]/10000)|(cmb.attribytes[1]%12)<<24
		: -((cmb.attribytes[3]<<16)|(cmb.attribytes[2]<<8)|(cmb.attribytes[1]));
	if(!id) return false;
	//Get the grid array for this tile/color
	std::map<dword,spot_t>& grid = ffmaps[id]; // grid of (x<<16)|(y&0xFFFF)
	byte spotdir = cmb.attribytes[0];
	lightbeam_xy sxy(ffc.x+(ffc.hit_width/2), ffc.y+(ffc.hit_height/2));
	if(spotdir > 3 && sxy.valid())
	{
		grid[sxy.ffpos()] |= SP_VISITED;
		int32_t trigflag = cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
		istrig[sxy.pos()] |= trigflag;
	}
	auto beamwid = cmb.attribytes[5] < 1 ? 8 : cmb.attribytes[5];
	if(refl && heropos > -1 && sxy.herocollide(beamwid))
	{
		spotdir = Hero.getDir();
	}
	switch(spotdir)
	{
		case up:
			sxy.y = zc_min(175,sxy.y);
			break;
		case down:
			sxy.y = zc_max(0,sxy.y);
			break;
		case left:
			sxy.x = zc_min(255,sxy.x);
			break;
		case right:
			sxy.x = zc_max(0,sxy.x);
			break;
	}
	handleFFBeam(grid, 0, spotdir, sxy, cmb.attribytes[4], block, refl, beamwid);
	return true;
}

static BITMAP* generate_beam_bitmap(int32_t id)
{
	BITMAP* cbmp = create_bitmap_ex(8, 16*beamoffs_max, 16);
	clear_bitmap(cbmp);
	if(id < 0)
	{
		byte c_inner = ((-id)&0x0000FF);
		byte c_middle = ((-id)&0x00FF00)>>8;
		byte c_outter = ((-id)&0xFF0000)>>16;
		for(size_t q = 1; q < beamoffs_max; ++q)
		{
			circlefill(cbmp, 16*q+8, 8, 3, c_outter);
			circlefill(cbmp, 16*q+7, 8, 3, c_outter);
			circlefill(cbmp, 16*q+8, 7, 3, c_outter);
			circlefill(cbmp, 16*q+7, 7, 3, c_outter);
			circlefill(cbmp, 16*q+8, 8, 1, c_middle);
			circlefill(cbmp, 16*q+7, 8, 1, c_middle);
			circlefill(cbmp, 16*q+8, 7, 1, c_middle);
			circlefill(cbmp, 16*q+7, 7, 1, c_middle);
			circlefill(cbmp, 16*q+8, 8, 0, c_inner);
			circlefill(cbmp, 16*q+7, 8, 0, c_inner);
			circlefill(cbmp, 16*q+8, 7, 0, c_inner);
			circlefill(cbmp, 16*q+7, 7, 0, c_inner);
		}
		//beamoffs_gr
		circlefill(cbmp, 16*beamoffs_gr+8, 8, 7, c_outter);
		circlefill(cbmp, 16*beamoffs_gr+7, 8, 7, c_outter);
		circlefill(cbmp, 16*beamoffs_gr+8, 7, 7, c_outter);
		circlefill(cbmp, 16*beamoffs_gr+7, 7, 7, c_outter);
		circlefill(cbmp, 16*beamoffs_gr+8, 8, 5, c_middle);
		circlefill(cbmp, 16*beamoffs_gr+7, 8, 5, c_middle);
		circlefill(cbmp, 16*beamoffs_gr+8, 7, 5, c_middle);
		circlefill(cbmp, 16*beamoffs_gr+7, 7, 5, c_middle);
		circlefill(cbmp, 16*beamoffs_gr+8, 8, 3, c_inner);
		circlefill(cbmp, 16*beamoffs_gr+7, 8, 3, c_inner);
		circlefill(cbmp, 16*beamoffs_gr+8, 7, 3, c_inner);
		circlefill(cbmp, 16*beamoffs_gr+7, 7, 3, c_inner);
		//beamoffs_up
		rectfill(cbmp, 16*beamoffs_up+4, 0, 16*beamoffs_up+11, 7, c_outter);
		rectfill(cbmp, 16*beamoffs_up+6, 0, 16*beamoffs_up+9, 7, c_middle);
		rectfill(cbmp, 16*beamoffs_up+7, 0, 16*beamoffs_up+8, 7, c_inner);
		//beamoffs_down
		rectfill(cbmp, 16*beamoffs_down+4, 8, 16*beamoffs_down+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_down+6, 8, 16*beamoffs_down+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_down+7, 8, 16*beamoffs_down+8, 15, c_inner);
		//beamoffs_left
		rectfill(cbmp, 16*beamoffs_left, 4, 16*beamoffs_left+7, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_left, 6, 16*beamoffs_left+7, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_left, 7, 16*beamoffs_left+7, 8, c_inner);
		//beamoffs_right
		rectfill(cbmp, 16*beamoffs_right+8, 4, 16*beamoffs_right+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_right+8, 6, 16*beamoffs_right+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_right+8, 7, 16*beamoffs_right+15, 8, c_inner);
		//beamoffs_uleft
		rectfill(cbmp, 16*beamoffs_uleft+4, 0, 16*beamoffs_uleft+11, 7, c_outter);
		rectfill(cbmp, 16*beamoffs_uleft, 4, 16*beamoffs_uleft+7, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_uleft, 6, 16*beamoffs_uleft+7, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_uleft+6, 0, 16*beamoffs_uleft+9, 7, c_middle);
		rectfill(cbmp, 16*beamoffs_uleft+7, 0, 16*beamoffs_uleft+8, 7, c_inner);
		rectfill(cbmp, 16*beamoffs_uleft, 7, 16*beamoffs_uleft+7, 8, c_inner);
		//beamoffs_uright
		rectfill(cbmp, 16*beamoffs_uright+4, 0, 16*beamoffs_uright+11, 7, c_outter);
		rectfill(cbmp, 16*beamoffs_uright+8, 4, 16*beamoffs_uright+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_uright+8, 6, 16*beamoffs_uright+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_uright+6, 0, 16*beamoffs_uright+9, 7, c_middle);
		rectfill(cbmp, 16*beamoffs_uright+7, 0, 16*beamoffs_uright+8, 7, c_inner);
		rectfill(cbmp, 16*beamoffs_uright+8, 7, 16*beamoffs_uright+15, 8, c_inner);
		//beamoffs_dleft
		rectfill(cbmp, 16*beamoffs_dleft+4, 8, 16*beamoffs_dleft+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_dleft, 4, 16*beamoffs_dleft+7, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_dleft, 6, 16*beamoffs_dleft+7, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_dleft+6, 8, 16*beamoffs_dleft+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_dleft+7, 8, 16*beamoffs_dleft+8, 15, c_inner);
		rectfill(cbmp, 16*beamoffs_dleft, 7, 16*beamoffs_dleft+7, 8, c_inner);
		//beamoffs_dright
		rectfill(cbmp, 16*beamoffs_dright+4, 8, 16*beamoffs_dright+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_dright+8, 4, 16*beamoffs_dright+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_dright+8, 6, 16*beamoffs_dright+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_dright+6, 8, 16*beamoffs_dright+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_dright+7, 8, 16*beamoffs_dright+8, 15, c_inner);
		rectfill(cbmp, 16*beamoffs_dright+8, 7, 16*beamoffs_dright+15, 8, c_inner);
		//beamoffs_vert
		rectfill(cbmp, 16*beamoffs_vert+4, 0, 16*beamoffs_vert+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_vert+6, 0, 16*beamoffs_vert+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_vert+7, 0, 16*beamoffs_vert+8, 15, c_inner);
		//beamoffs_horz
		rectfill(cbmp, 16*beamoffs_horz, 4, 16*beamoffs_horz+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_horz, 6, 16*beamoffs_horz+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_horz, 7, 16*beamoffs_horz+15, 8, c_inner);
		//beamoffs_notup
		rectfill(cbmp, 16*beamoffs_notup, 4, 16*beamoffs_notup+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_notup+4, 8, 16*beamoffs_notup+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_notup+6, 8, 16*beamoffs_notup+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_notup, 6, 16*beamoffs_notup+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_notup, 7, 16*beamoffs_notup+15, 8, c_inner);
		rectfill(cbmp, 16*beamoffs_notup+7, 8, 16*beamoffs_notup+8, 15, c_inner);
		//beamoffs_notdown
		rectfill(cbmp, 16*beamoffs_notdown, 4, 16*beamoffs_notdown+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_notdown+4, 0, 16*beamoffs_notdown+11, 7, c_outter);
		rectfill(cbmp, 16*beamoffs_notdown+6, 0, 16*beamoffs_notdown+9, 7, c_middle);
		rectfill(cbmp, 16*beamoffs_notdown, 6, 16*beamoffs_notdown+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_notdown, 7, 16*beamoffs_notdown+15, 8, c_inner);
		rectfill(cbmp, 16*beamoffs_notdown+7, 0, 16*beamoffs_notdown+8, 7, c_inner);
		//beamoffs_notleft
		rectfill(cbmp, 16*beamoffs_notleft+4, 0, 16*beamoffs_notleft+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_notleft+8, 4, 16*beamoffs_notleft+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_notleft+8, 6, 16*beamoffs_notleft+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_notleft+6, 0, 16*beamoffs_notleft+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_notleft+7, 0, 16*beamoffs_notleft+8, 15, c_inner);
		rectfill(cbmp, 16*beamoffs_notleft+8, 7, 16*beamoffs_notleft+15, 8, c_inner);
		//beamoffs_notright
		rectfill(cbmp, 16*beamoffs_notright+4, 0, 16*beamoffs_notright+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_notright, 4, 16*beamoffs_notright+7, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_notright, 6, 16*beamoffs_notright+7, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_notright+6, 0, 16*beamoffs_notright+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_notright+7, 0, 16*beamoffs_notright+8, 15, c_inner);
		rectfill(cbmp, 16*beamoffs_notright, 7, 16*beamoffs_notright+7, 8, c_inner);
		//beamoffs_all
		rectfill(cbmp, 16*beamoffs_all+4, 0, 16*beamoffs_all+11, 15, c_outter);
		rectfill(cbmp, 16*beamoffs_all, 4, 16*beamoffs_all+15, 11, c_outter);
		rectfill(cbmp, 16*beamoffs_all, 6, 16*beamoffs_all+15, 9, c_middle);
		rectfill(cbmp, 16*beamoffs_all+6, 0, 16*beamoffs_all+9, 15, c_middle);
		rectfill(cbmp, 16*beamoffs_all+7, 0, 16*beamoffs_all+8, 15, c_inner);
		rectfill(cbmp, 16*beamoffs_all, 7, 16*beamoffs_all+15, 8, c_inner);
	}
	else
	{
		int32_t tile = (id&0xFFFFFF);
		int32_t cs = (id>>24);
		for(size_t q = 0; q < beamoffs_max; ++q)
			puttile16(cbmp, tile+q, q*16, 0, cs, 0);
	}
	return cbmp;
}

static int32_t get_beamoffs(spot_t val)
{
	switch((val&SP_MASK)>>1)
	{
		case 0: default:
			if(val)
				return beamoffs_gr;
			return -1;
		case 0b0001:
			return beamoffs_up;
		case 0b0010:
			return beamoffs_down;
		case 0b0100:
			return beamoffs_left;
		case 0b1000:
			return beamoffs_right;
		case 0b0011:
			return beamoffs_vert;
		case 0b1100:
			return beamoffs_horz;
		case 0b0101:
			return beamoffs_uleft;
		case 0b1001:
			return beamoffs_uright;
		case 0b0110:
			return beamoffs_dleft;
		case 0b1010:
			return beamoffs_dright;
		case 0b1110:
			return beamoffs_notup;
		case 0b1101:
			return beamoffs_notdown;
		case 0b1011:
			return beamoffs_notleft;
		case 0b0111:
			return beamoffs_notright;
		case 0b1111:
			return beamoffs_all;
	}
	return -1;
}

void HeroClass::handleSpotlights()
{
	int map_size = region_scr_width * 16 * region_scr_height * 11;
	typeMap.resize(map_size);
	customTypeMap.resize(map_size);
	istrig.resize(map_size);
	std::fill(typeMap.begin(), typeMap.end(), 0);
	std::fill(customTypeMap.begin(), customTypeMap.end(), -1);
	MAPS_prism_dir_seen_map.clear();

	//Store each different tile/color as grids
	std::map<int32_t, spot_t*> maps;
	std::map<int32_t, std::map<dword, spot_t>> ffmaps;
	int32_t shieldid = getCurrentShield(false);
	bool refl = shieldid > -1 && (itemsbuf[shieldid].misc2 & shLIGHTBEAM);
	bool block = !refl && shieldid > -1 && (itemsbuf[shieldid].misc1 & shLIGHTBEAM);
	heropos = COMBOPOS_REGION_EXTENDED_B(x.getInt()+8,y.getInt()+8);
	clear_bitmap(lightbeam_bmp);

	bool foundany = false;
	
	// TODO z3 for_every_rpos
	for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
		bool pos_has_seen_cmb[176] = {0};
		
		for(int32_t lyr = 6; lyr >= 0; --lyr)
		{
			mapscr* layer_scr = get_layer_scr(currmap, screen_index, lyr - 1);

			for(size_t pos = 0; pos < 176; ++pos)
			{
				if (pos_has_seen_cmb[pos]) continue;
				
				int realpos = COMBOPOS_REGION_EXTENDED(pos, region_scr_x, region_scr_y);
				newcombo const* cmb = &combobuf[layer_scr->data[pos]];
				switch(cmb->type)
				{
					case cMIRROR: case cMIRRORSLASH: case cMIRRORBACKSLASH:
					case cMAGICPRISM: case cMAGICPRISM4:
					case cBLOCKALL: case cLIGHTTARGET:
						typeMap[realpos] = cmb->type;
						break;
					case cMIRRORNEW:
						typeMap[realpos] = cMIRRORNEW;
						customTypeMap[realpos] = layer_scr->data[pos];
						break;
					case cGLASS:
						// Even if solid, is always OK to pass through.
						// Already been initialized to zero.
						// typeMap[realpos] = 0;
						break;
					case cSPOTLIGHT:
						foundany = true;
						[[fallthrough]];
					default:
					{
						if(lyr < 3 && (cmb->walk & 0xF))
						{
							typeMap[realpos] = SPTYPE_SOLID;
						}
						continue; //next layer
					}
				}

				pos_has_seen_cmb[pos] = true;
			}
		}

		for (int pos = 0; pos < 176; pos++)
		{
			int realpos = COMBOPOS_REGION_EXTENDED(pos, region_scr_x, region_scr_y);
			if (!get_qr(qr_SPOTLIGHT_IGNR_SOLIDOBJ) && !typeMap[pos])
			{
				if(collide_object(COMBOX(pos),COMBOY(pos),16,16,this))
					typeMap[pos] = SPTYPE_SOLID;
			}
		}
	});

	// The world is dark and full of terror.
	if (!foundany) return;

	switch(typeMap[heropos])
	{
		case SPTYPE_SOLID: case cBLOCKALL:
			heropos = -1; //Blocked from hitting player
	}

	//For each spotlight combo on each layer...
	for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
		for(size_t layer = 0; layer < 7; ++layer)
		{
			mapscr* curlayer = get_layer_scr(currmap, screen_index, layer - 1);
			for(size_t pos = 0; pos < 176; ++pos)
			{
				newcombo const& cmb = combobuf[curlayer->data[pos]];
				if(cmb.type == cSPOTLIGHT)
				{
					int realpos = COMBOPOS_REGION_EXTENDED(pos, region_scr_x, region_scr_y);
					launch_lightbeam(cmb,realpos,maps,refl,block);
				}
			}
		}
	});

	// TODO z3 !!
	word c = tmpscr->numFFC();
	for(word i=0; i<c; i++)
	{
		ffcdata& ffc = tmpscr->ffcs[i];
		newcombo const& cmb = combobuf[ffc.getData()];
		if(cmb.type == cSPOTLIGHT && (cmb.usrflags&cflag2))
			launch_fflightbeam(ffc,ffmaps,refl,block);
	}
	
	lightbeam_present = !maps.empty() || !ffmaps.empty();
	
	//Draw visuals
	for(auto it = maps.begin(); it != maps.end();)
	{
		int32_t id = it->first;
		spot_t* grid = it->second;
		BITMAP* cbmp = generate_beam_bitmap(id);
		for(size_t pos = 0; pos < map_size; ++pos)
		{
			int32_t offs = get_beamoffs(grid[pos]);
			if(offs > -1)
				masked_blit(cbmp, lightbeam_bmp, offs*16, 0, COMBOX_REGION_EXTENDED(pos)-viewport.x, COMBOY_REGION_EXTENDED(pos)-viewport.y, 16, 16);
		}
		destroy_bitmap(cbmp);
		delete[] it->second;
		it = maps.erase(it);
	}
	for(auto it = ffmaps.begin(); it != ffmaps.end();)
	{
		int32_t id = it->first;
		std::map<dword,spot_t> const& grid = it->second;
		BITMAP* cbmp = generate_beam_bitmap(id);
		for(auto it2 = grid.begin(); it2 != grid.end(); ++it2)
		{
			lightbeam_xy ffxy(it2->first);
			int32_t offs = get_beamoffs(it2->second);
			if(offs > -1)
				masked_blit(cbmp, lightbeam_bmp, offs*16, 0, ffxy.x-8-viewport.x, ffxy.y-8-viewport.y, 16, 16);
		}
		destroy_bitmap(cbmp);
		it = ffmaps.erase(it);
	}
	//Check triggers
	bool hastrigs = false, istrigged = true;
	bool alltrig = getmapflag(mLIGHTBEAM);
	for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
		for(size_t layer = 0; layer < 7; ++layer)
		{
			mapscr* curlayer = get_layer_scr(currmap, screen_index, layer - 1);
			for(size_t pos = 0; pos < 176; ++pos)
			{
				newcombo const* cmb = &combobuf[curlayer->data[pos]];
				if(cmb->type == cLIGHTTARGET)
				{
					int32_t trigflag = cmb->attribytes[4] ? (1 << (cmb->attribytes[4]-1)) : ~0;
					hastrigs = true;
					bool trigged = (istrig[COMBOPOS_REGION_EXTENDED(pos, region_scr_x, region_scr_y)]&trigflag);
					if(cmb->usrflags&cflag2) //Invert
						trigged = !trigged;
					if(cmb->usrflags&cflag1) //Solved Version
					{
						if(!(alltrig || trigged)) //Revert
						{
							curlayer->data[pos] -= 1;
							istrigged = false;
						}
					}
					else //Unsolved version
					{
						if(alltrig || trigged) //Light
							curlayer->data[pos] += 1;
						else istrigged = false;
					}
				}
				else if(cmb->triggerflags[1] & (combotriggerLIGHTON|combotriggerLIGHTOFF))
				{
					int32_t trigflag = cmb->triglbeam ? (1 << (cmb->triglbeam-1)) : ~0;
					bool trigged = (istrig[pos]&trigflag);
					if(trigged ? (cmb->triggerflags[1] & combotriggerLIGHTON)
						: (cmb->triggerflags[1] & combotriggerLIGHTOFF))
					{
						// TODO z3
						do_trigger_combo(layer, pos);
					}
				}
			}
		}
	});

	for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
		newcombo const* cmb = &combobuf[ffc_handle.data()];
		auto ffc = ffc_handle.ffc;
		size_t pos = get_qr(qr_BROKEN_LIGHTBEAM_HITBOX)
			? COMBOPOS(ffc->x+8, ffc->y+8)
			: COMBOPOS(ffc->x+(ffc->hit_width/2), ffc->y+(ffc->hit_height/2));
		if(cmb->type == cLIGHTTARGET)
		{
			int32_t trigflag = cmb->attribytes[4] ? (1 << (cmb->attribytes[4]-1)) : ~0;
			hastrigs = true;
			bool trigged = (istrig[pos]&trigflag);
			if(cmb->usrflags&cflag2) //Invert
				trigged = !trigged;
			if(cmb->usrflags&cflag1) //Solved Version
			{
				if(!(alltrig || trigged)) //Revert
				{
					ffc_handle.ffc->incData(-1);
					istrigged = false;
				}
			}
			else //Unsolved version
			{
				if(alltrig || trigged) //Light
					ffc_handle.ffc->incData(1);
				else istrigged = false;
			}
		}
		else if(cmb->triggerflags[1] & (combotriggerLIGHTON|combotriggerLIGHTOFF))
		{
			int32_t trigflag = cmb->triglbeam ? (1 << (cmb->triglbeam-1)) : ~0;
			bool trigged = (istrig[pos]&trigflag);
			if(trigged ? (cmb->triggerflags[1] & combotriggerLIGHTON)
				: (cmb->triggerflags[1] & combotriggerLIGHTOFF))
			{
				do_trigger_combo_ffc(ffc_handle);
			}
		}
	});

	if(hastrigs && istrigged && !alltrig)
	{
		// TODO z3 what screen? main screen?
		trigger_secrets_for_screen(TriggerSource::LightTrigger, false);
		sfx(tmpscr->secretsfx);
		if(!(tmpscr->flags5&fTEMPSECRETS))
		{
			setmapflag(mSECRET);
			setmapflag(mLIGHTBEAM);
		}
	}

	typeMap.clear();
	istrig.clear();
}

void HeroClass::checktouchblk()
{
	if(toogam) return;
	
	if(!pushing)
		return;
		
	int32_t tdir = dir; //Bad hack #2. _L_, your welcome to fix this properly. ;)
	
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
	
	int32_t tx=0,ty=-1;
	
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
		if (getAction() != hopping || isSideViewHero())
		{
			trigger_armos_grave(get_rpos_handle_for_world_xy(tx, ty, 0), dir);
		}
	}
}

int32_t HeroClass::nextcombo(int32_t cx, int32_t cy, int32_t cdir)
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
    if(cx<0 || cy<0 || cx>=world_w || cy>=world_h)
    {
        int32_t ns = nextscr(cdir);
        
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

		if (cx < 0 || cx >= world_w || cy < 0 || cy >= world_h)
			return 0;
		
		int32_t cmb = COMBOPOS(cx%256, cy%176);
		return TheMaps[ns].data[cmb];
    }
    
    return MAPCOMBO(cx,cy);
}

int32_t HeroClass::nextflag(int32_t cx, int32_t cy, int32_t cdir, bool comboflag)
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
	// TODO z3
    if(cx<0 || cy<0 || cx>=world_w || cy>=world_h)
    {
        int32_t ns = nextscr(cdir);
        
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

		if (cx < 0 || cx >= world_w || cy < 0 || cy >= world_h)
			return 0;
		
		int32_t cmb = COMBOPOS(cx%256, cy%176);
		if (!comboflag)
        {
            return TheMaps[ns].sflag[cmb];
        }
        else
        {
            return combobuf[TheMaps[ns].data[cmb]].flag;
        }
    }
    
    if(comboflag)
    {
        return MAPCOMBOFLAG(cx,cy);
    }
    
    return MAPFLAG(cx,cy);
}

bool did_secret;

void HeroClass::checkspecial()
{
    checktouchblk();

	for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
		bool hasmainguy = hasMainGuy(screen_index);
		bool loaded_enemies = loaded_enemies_for_screen.contains(screen_index);
		if (!loaded_enemies || hasmainguy)
		{
			did_secret = false;
		}
		else
		{
			// Enemies have been defeated.

			// generic 'Enemies->' trigger
			for_every_rpos_in_screen(screen, screen_index, [&](const rpos_handle_t& rpos_handle) {
				newcombo const& cmb = combobuf[rpos_handle.data()];
				if (cmb.triggerflags[2] & combotriggerENEMIESKILLED)
				{
					do_trigger_combo(rpos_handle);
				}
			});
			for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
				newcombo const& cmb = combobuf[ffc_handle.data()];
				if(cmb.triggerflags[2] & combotriggerENEMIESKILLED)
				{
					do_trigger_combo_ffc(ffc_handle);
				}
			});

			if (screen->flags9 & fENEMY_WAVES)
			{
				hasmainguy = hasMainGuy(screen_index); //possibly un-beat the enemies (another 'wave'?)
			}
			if(!hasmainguy)
			{
				ScreenItemState item_state = screen_item_get_state(screen_index);
				
				// item
				if (item_state == ScreenItemState::MustGiveToEnemy || item_state == ScreenItemState::CarriedByEnemy || item_state == ScreenItemState::WhenKillEnemies)
				{
					int32_t Item=screen->item;

					if((!getmapflag(screen_index, mITEM) || (screen->flags9&fITEMRETURN)) && (screen->hasitem != 0))
					{
						if (item_state == ScreenItemState::WhenKillEnemies)
							sfx(WAV_CLEARED);
						
						zfix x = region_scr_x*256 + screen->itemx;
						zfix y = region_scr_y*176 + ((screen->flags7&fITEMFALLS && isSideViewHero()) ? -170 : screen->itemy+1);
						add_item_for_screen(screen_index, new item(x, y, (screen->flags7&fITEMFALLS && !isSideViewHero()) ? (zfix)170 : (zfix)0,
										Item,ipONETIME|ipBIGRANGE|((itemsbuf[Item].family==itype_triforcepiece ||
												(screen->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((screen->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
					}
					
					screen_item_clear_state(screen_index);
				}
				// if room has traps, guys don't come back
				// TODO z3
				for (int32_t i=0; i<eMAXGUYS; i++)
				{
					if (guysbuf[i].family==eeTRAP&&guysbuf[i].misc2)
						if (guys.idCount(i) && !getmapflag(screen_index, mTMPNORET))
							setmapflag(screen_index, mTMPNORET);
				}
				// clear enemies and open secret
				if (!did_secret && (screen->flags2&fCLEARSECRET))
				{
					bool only16_31 = get_qr(qr_ENEMIES_SECRET_ONLY_16_31)?true:false;
					trigger_secrets_for_screen(TriggerSource::EnemiesScreenFlag, only16_31);
					
					if (screen->flags4&fENEMYSCRTPERM && canPermSecret(currdmap, screen_index))
					{
						if (!(screen->flags5&fTEMPSECRETS)) setmapflag(screen_index, mSECRET);
					}
					
					sfx(screen->secretsfx);
					did_secret=true;
				}
			}
		}

		// doors
		bool has_shutter = false;
		for(int32_t i=0; i<4; i++)
			if(screen->door[i]==dSHUTTER)
			{
				has_shutter = true;
				if(opendoors==0 && loaded_enemies)
				{
					if(!(screen->flags&fSHUTTERS) && !hasmainguy)
						opendoors=12;
				}
				else if(opendoors<0)
					++opendoors;
				else if((--opendoors)==0)
					openshutters();
					
				break;
			}
		if(!has_shutter && !opendoors && loaded_enemies && !(screen->flags&fSHUTTERS) && !hasmainguy)
		{
			openshutters();
		}

    	// set boss flag when boss is gone
		if (loaded_enemies && screen->enemyflags&efBOSS && !hasmainguy)
		{
			game->lvlitems[dlevel]|=liBOSS;
			stop_sfx(screen->bosssfx);
		}

		if (getmapflag(screen_index, mCHEST))              // if special stuff done before
		{
			remove_chests(screen, screen_index);
		}
		
		if(getmapflag(screen_index, mLOCKEDCHEST))              // if special stuff done before
		{
			remove_lockedchests(screen, screen_index);
		}
		
		if(getmapflag(screen_index, mBOSSCHEST))              // if special stuff done before
		{
			remove_bosschests(screen, screen_index);
		}

		clear_xstatecombos(screen, screen_index, true);

		// TODO z3 ! secrets
		if (screen_item_get_state(screen_index) == ScreenItemState::WhenTriggerSecrets && triggered_screen_secrets)
		{
			int32_t Item=screen->item;
			
			if((!getmapflag(screen_index, mITEM) || (screen->flags9&fITEMRETURN)) && (screen->hasitem != 0))
			{
				zfix x = region_scr_x*256 + screen->itemx;
				zfix y = region_scr_y*176 + ((screen->flags7&fITEMFALLS && isSideViewHero()) ? -170 : screen->itemy+1);
				add_item_for_screen(screen_index, new item(x, y,
								(screen->flags7&fITEMFALLS && !isSideViewHero()) ? (zfix)170 : (zfix)0,
								Item,ipONETIME|ipBIGRANGE|((itemsbuf[Item].family==itype_triforcepiece ||
										(screen->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((screen->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
			}
			
			screen_item_clear_state(screen_index);
		}
	});
}

//Gets the 4 rcomboposes indicated by the coordinates, replacing duplicates with rpos_t::None
void getRposes(rpos_t* rposes, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	x1 %= world_w;
	y1 %= world_h;
	x2 %= world_w;
	y2 %= world_h;

	rpos_t tmp;
	rposes[0] = COMBOPOS_REGION(x1,y1);
	
	tmp = COMBOPOS_REGION(x1,y2);
	if(tmp == rposes[0])
		rposes[1] = rpos_t::None;
	else rposes[1] = tmp;
	
	tmp = COMBOPOS_REGION(x2,y1);
	if(tmp == rposes[0] || tmp == rposes[1])
		rposes[2] = rpos_t::None;
	else rposes[2] = tmp;
	
	tmp = COMBOPOS_REGION(x2,y2);
	if(tmp == rposes[0] || tmp == rposes[1] || tmp == rposes[2])
		rposes[3] = rpos_t::None;
	else rposes[3] = tmp;
}

void HeroClass::checkspecial2(int32_t *ls)
{
	if(get_qr(qr_OLDSTYLEWARP) && !(diagonalMovement||NO_GRIDLOCK))
	{
		// Must run fairycircle stuff if currently active, otherwise hero gets stuck!
		if (!fairyclk)
		{
			if(y.getInt()&7)
				return;

			if(x.getInt()&7)
				return;
		}
	}
	
	if(toogam) return;
	
	bool didstrig = false;
	
	for(int32_t i=bigHitbox?0:8; i<16; i+=bigHitbox?15:7)
	{
		for(int32_t j=0; j<16; j+=15) for(int32_t k=0; k<2; k++)
		{
			newcombo const& cmb = combobuf[k>0 ? MAPFFCOMBO(x+j,y+i) : MAPCOMBO(x+j,y+i)];
			int32_t stype = cmb.type;
			int32_t warpsound = cmb.attribytes[0];
			if(cmb.triggerflags[0] & combotriggerONLYGENTRIG)
				stype = cNONE;
			if(stype==cSWARPA)
			{
				if(tmpscr->flags5&fDIRECTSWARP)
				{
					didpit=true;
					pitx=x;
					pity=y;
				}
				
				sdir=dir;
				dowarp(0,0,warpsound);
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
				dowarp(0,1,warpsound);
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
				dowarp(0,2,warpsound);
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
				dowarp(0,3,warpsound);
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
				dowarp(0,(zc_oldrand()%4),warpsound);
				return;
			}
			
			rpos_t rpos = COMBOPOS_REGION(x+j, y+i);
			if((stype==cSTRIGNOFLAG || stype==cSTRIGFLAG) && stepsecret!=rpos)
			{
				auto rpos_handle = get_rpos_handle(rpos, 0);
				
				if(stype==cSTRIGFLAG && canPermSecret(currdmap, rpos_handle.screen_index))
				{ 
					if(!didstrig)
					{
						stepsecret = rpos;
						
						if(!(rpos_handle.screen->flags5&fTEMPSECRETS))
						{
							setmapflag(rpos_handle.screen, rpos_handle.screen_index, mSECRET);
						}
						//int32_t thesfx = combobuf[MAPCOMBO(x+j,y+i)].attribytes[0];
						//zprint("Step Secrets SFX: %d\n", thesfx);
						sfx(warpsound,pan((int32_t)x));
						trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.screen_index, false);
						didstrig = true;
					}
				}
				else
				{ 
					if(!didstrig)
					{
						stepsecret = rpos;

						bool high16only = get_qr(qr_STEPTEMP_SECRET_ONLY_16_31)?true:false;
						trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.screen_index, high16only);
						didstrig = true;
						//play trigger sound
						//int32_t thesfx = combobuf[MAPCOMBO(x+j,y+i)].attribytes[0];
						//zprint("Step Secrets SFX: %d\n", thesfx);
						//sfx(thesfx,pan((int32_t)x));
						sfx(warpsound,pan((int32_t)x));
					}
				}
			}
		}
	}
	
	bool RaftPass = false;//Special case for the raft, where only the raft stuff gets checked and nothing else.
	
	// check if he's standing on a warp he just came out of
	// But if the QR is checked, it uses the old logic, cause some quests like Ballad of a Bloodline warp you onto a trigger and this new logic bricks that.
	if (!get_qr(qr_210_WARPRETURN))
	{
		if(((int32_t)y>=warpy-8&&(int32_t)y<=warpy+7)&&warpy!=-1)
		{
			if(((int32_t)x>=warpx-8&&(int32_t)x<=warpx+7)&&warpx!=-1)
			{
				if (get_qr(qr_BETTER_RAFT_2) && dir != up) RaftPass = true;
				else return;
			}
		}
	}
	else
	{
		if((int(y)&0xF8)==warpy)
		{
			if(x==warpx) 
			{
				if (get_qr(qr_BETTER_RAFT_2) && dir != up) RaftPass = true;
				else return;
			}
		}
	}
	if (!RaftPass) warpy=-1;
	
	if(((int32_t)y<raftwarpy-(get_qr(qr_BETTER_RAFT_2)?12:8)||(int32_t)y>raftwarpy+(get_qr(qr_BETTER_RAFT_2)?3:7))||raftwarpy==-1)
	{
		raftwarpy = -1;
	}
	if (((int32_t)x<raftwarpx - 8 || (int32_t)x>raftwarpx + 7) || raftwarpx == -1)
	{
		raftwarpx = -1;
	}
	int32_t tx=x;
	int32_t ty=y;
	
	int32_t flag=0;
	int32_t flag2=0;
	int32_t flag3=0;
	int32_t type=0;
	int32_t water=0;
	int32_t index = 0;
	
	bool setsave=false;
	int32_t warpsfx2 = 0;
	if (RaftPass) goto RaftingStuff;
	
	//bool gotpit=false;
	
	int32_t x1,x2,y1,y2;
	x1 = tx;
	x2 = tx+15;
	y1 = ty;
	y2 = ty+15;
	
	if((diagonalMovement||NO_GRIDLOCK))
	{
		x1 = tx+4;
		x2 = tx+11;
		y1 = ty+4;
		y2 = ty+11;
	}
	
	int32_t types[4];
	types[0]=types[1]=types[2]=types[3]=-1;
	int32_t cids[4];
	int32_t ffcids[4];
	cids[0]=cids[1]=cids[2]=cids[3]=-1;
	ffcids[0]=ffcids[1]=ffcids[2]=ffcids[3]=-1;
	//
	// First, let's find flag1 (combo flag), flag2 (inherent flag) and flag3 (FFC flag)...
	//
	types[0] = MAPFLAG(x1,y1);
	types[1] = MAPFLAG(x1,y2);
	types[2] = MAPFLAG(x2,y1);
	types[3] = MAPFLAG(x2,y2);
	
	
	//MAPFFCOMBO
	
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag = types[0];
	// 2.10 compatibility...
	else if(y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag = types[0];
		
		
	types[0] = MAPCOMBOFLAG(x1,y1);
	types[1] = MAPCOMBOFLAG(x1,y2);
	types[2] = MAPCOMBOFLAG(x2,y1);
	types[3] = MAPCOMBOFLAG(x2,y2);
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag2 = types[0];
	else if(!get_qr(qr_FAIRY_FLAG_COMPAT) && y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag2 = types[0];
		
	types[0] = MAPFFCOMBOFLAG(x1,y1);
	types[1] = MAPFFCOMBOFLAG(x1,y2);
	types[2] = MAPFFCOMBOFLAG(x2,y1);
	types[3] = MAPFFCOMBOFLAG(x2,y2);
	
	
	//
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag3 = types[0];
	else if(!get_qr(qr_FAIRY_FLAG_COMPAT) && y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag3 = types[0];
		
	//
	// Now, let's check for warp combos...
	//
	
	//
	
	cids[0] = MAPCOMBO(x1,y1);
	cids[1] = MAPCOMBO(x1,y2);
	cids[2] = MAPCOMBO(x2,y1);
	cids[3] = MAPCOMBO(x2,y2);
	
	types[0] = COMBOTYPE(x1,y1);
	
	if(MAPFFCOMBO(x1,y1))
	{
		types[0] = FFCOMBOTYPE(x1,y1);
		cids[0] = MAPFFCOMBO(x1,y1);
	}
		
	types[1] = COMBOTYPE(x1,y2);
	
	if(MAPFFCOMBO(x1,y2))
	{
		types[1] = FFCOMBOTYPE(x1,y2);
		cids[1] = MAPFFCOMBO(x1,y2);
	}
		
	types[2] = COMBOTYPE(x2,y1);
	
	if(MAPFFCOMBO(x2,y1))
	{
		types[2] = FFCOMBOTYPE(x2,y1);
		cids[2] = MAPFFCOMBO(x2,y1);
	}
	types[3] = COMBOTYPE(x2,y2);
	
	if(MAPFFCOMBO(x2,y2))
	{
		types[3] = FFCOMBOTYPE(x2,y2);
		cids[3] = MAPFFCOMBO(x2,y2);
	}
	// Change B, C and D warps into A, for the comparison below...
	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].triggerflags[0] & combotriggerONLYGENTRIG)
		{
			types[i] = cNONE;
			continue;
		}
		if(types[i]==cCAVE)
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cCAVEB)
		{
			types[i]=cCAVE;
			index=1;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cCAVEC)
		{
			types[i]=cCAVE;
			index=2;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cCAVED)
		{
			types[i]=cCAVE;
			index=3;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		
		if(types[i]==cPIT) 
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cPITB)
		{
			types[i]=cPIT;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=1;
		}
		else if(types[i]==cPITC)
		{
			types[i]=cPIT;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=2;
		}
		else if(types[i]==cPITD)
		{
			types[i]=cPIT;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=3;
		}
		else if(types[i]==cPITR)
		{
			types[i]=cPIT;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=zc_oldrand()%4;
		}
		
		if(types[i]==cSTAIR)
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cSTAIRB)
		{
			types[i]=cSTAIR;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=1;
		}
		else if(types[i]==cSTAIRC)
		{
			types[i]=cSTAIR;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=2;
		}
		else if(types[i]==cSTAIRD)
		{
			types[i]=cSTAIR;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=3;
		}
		else if(types[i]==cSTAIRR)
		{
			types[i]=cSTAIR;
			index=zc_oldrand()%4;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		
		if(types[i]==cCAVE2)
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cCAVE2B)
		{
			types[i]=cCAVE2;
			index=1;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cCAVE2C)
		{
			types[i]=cCAVE2;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=2;
		}
		else if(types[i]==cCAVE2D)
		{
			types[i]=cCAVE2;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=3;
		}
		
		if(types[i]==cSWIMWARP) 
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cSWIMWARPB)
		{
			types[i]=cSWIMWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=1;
		}
		else if(types[i]==cSWIMWARPC)
		{
			types[i]=cSWIMWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=2;
		}
		else if(types[i]==cSWIMWARPD)
		{
			types[i]=cSWIMWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=3;
		}
		
		if(types[i]==cDIVEWARP) 
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cDIVEWARPB)
		{
			types[i]=cDIVEWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=1;
		}
		else if(types[i]==cDIVEWARPC)
		{
			types[i]=cDIVEWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=2;
		}
		else if(types[i]==cDIVEWARPD)
		{
			types[i]=cDIVEWARP;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
			index=3;
		}
		
		if(types[i]==cSTEP) warpsfx2 = combobuf[cids[i]].attribytes[0];
		else if(types[i]==cSTEPSAME) { types[i]=cSTEP; warpsfx2 = combobuf[cids[i]].attribytes[0];}
		else if(types[i]==cSTEPALL) { types[i]=cSTEP; warpsfx2 = combobuf[cids[i]].attribytes[0]; }
	}
	
	// Special case for step combos; otherwise, they act oddly in some cases
	if((types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])||(types[1]==cSTEP&&types[3]==cSTEP))
	{
		if(action!=freeze&&action!=sideswimfreeze&&(!msg_active || !get_qr(qr_MSGFREEZE)))
			type = types[1];
	}
	
	//Generic Step
	if (!is_z3_scrolling_mode()) // TODO z3
	if(action!=freeze&&action!=sideswimfreeze&&(!msg_active || !get_qr(qr_MSGFREEZE)))
	{
		rpos_t rposes[4];
		rpos_t sensRposes[4];
		int32_t xPoses[4] = {tx + 4, tx + 11, tx, tx + 15};
		int32_t yPoses[4] = {ty + 4, ty + 11, ty+(bigHitbox?0:8), ty + 15};
		if(diagonalMovement||NO_GRIDLOCK)
			getRposes(rposes, tx+4, ty+4, tx+11, ty+11);
		else getRposes(rposes, tx, ty, tx+15, ty+15);
		getRposes(sensRposes, tx, ty+(bigHitbox?0:8), tx+15, ty+15);
		bool hasStep[4] = {false};
		for(auto p = 0; p < 4; ++p)
		{
			for(auto lyr = 0; lyr < 7; ++lyr)
			{
				newcombo const* cmb = rposes[p]==rpos_t::None ? nullptr : &combobuf[FFCore.tempScreens[lyr]->data[RPOS_TO_POS(rposes[p])]];
				if((cmb && (cmb->triggerflags[0] & (combotriggerSTEP|combotriggerSTEPSENS)))
					|| types[p] == cSTEP)
				{
					hasStep[p] = true;
					break;
				}
			}
		}
		bool canNormalStep = true;
		for(auto p = 0; p < 4; ++p)
		{
			if(rposes[p] == rpos_t::None) continue;
			if(!hasStep[p])
			{
				canNormalStep = false;
				break;
			}
		}
		for(auto p = 0; p < 4; ++p)
		{
			for(auto lyr = 0; lyr < 7; ++lyr)
			{
				newcombo const* cmb = rposes[p]==rpos_t::None ? nullptr : &combobuf[FFCore.tempScreens[lyr]->data[RPOS_TO_POS(rposes[p])]];
				newcombo const* cmb2 = sensRposes[p]==rpos_t::None ? nullptr : &combobuf[FFCore.tempScreens[lyr]->data[RPOS_TO_POS(sensRposes[p])]];
				if(canNormalStep && cmb && (cmb->triggerflags[0] & combotriggerSTEP))
				{
					do_trigger_combo(get_rpos_handle(rposes[p], lyr));
					if(rposes[p] == sensRposes[p]) continue;
				}
				if(cmb2 && (cmb2->triggerflags[0] & combotriggerSTEPSENS))
				{
					do_trigger_combo(get_rpos_handle(sensRposes[p], lyr));
				}
			}
		}

		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			bool found = false;
			for (int xch = 0; xch < 2; ++xch)
			{
				for(int ych = 0; ych < 2; ++ych)
				{
					if (ffcIsAt(ffc_handle, xPoses[xch], yPoses[ych]))
					{
						found = true;
					}
				}
			}
			if (found)
			{
				newcombo const* cmb = &combobuf[ffc_handle.data()];
				if (cmb->triggerflags[0] & (combotriggerSTEP|combotriggerSTEPSENS))
				{
					do_trigger_combo_ffc(ffc_handle);
				}
			}
		});
	}
	// TODO z3 !!!!!!! merge
	// if(isDiving()) //Dive-> triggerflag
	// {
	// 	int pos = COMBOPOS(x+8,y+8);
	// 	int x1=x,x2=x+15,y1=y+(bigHitbox?0:8),y2=y+15;
	// 	int xposes[] = {x1,x1,x2,x2};
	// 	int yposes[] = {y1,y2,y1,y2};
	// 	rpos_t rposes[4];
	// 	getRposes(rposes,x1,y1,x2,y2);
	// 	for(auto lyr = 0; lyr < 7; ++lyr)
	// 	{
	// 		mapscr* s = FFCore.tempScreens[lyr];
	// 		newcombo const& cmb = combobuf[s->data[pos]];
	// 		bool didtrig = false;
	// 		if (cmb.triggerflags[3] & combotriggerDIVETRIG)
	// 		{
	// 			do_trigger_combo(lyr,pos);
	// 			didtrig = true;
	// 		}
	// 		for(auto q = 0; q < 4; ++q)
	// 		{
	// 			if(rposes[q] == rpos_t::None) continue;
	// 			if(rposes[q] == rpos && didtrig) continue;
	// 			newcombo const& cmb = combobuf[s->data[rposes[q]]];
	// 			if (cmb.triggerflags[3] & combotriggerDIVESENSTRIG)
	// 				do_trigger_combo(lyr,rposes[q]);
	// 		}
	// 	}
	// 	word c = tmpscr->numFFC();
	// 	for(word i=0; i<c; i++)
	// 	{
	// 		ffcdata& ffc = tmpscr->ffcs[i];
	// 		newcombo const& cmb = combobuf[ffc.getData()];
	// 		if ((cmb.triggerflags[3] & combotriggerDIVETRIG) && ffcIsAt(i, x+8, y+8))
	// 		{
	// 			do_trigger_combo_ffc(i);
	// 		}
	// 		else if(cmb.triggerflags[3] & combotriggerDIVESENSTRIG)
	// 		{
	// 			for(auto q = 0; q < 4; ++q)
	// 			{
	// 				if(ffcIsAt(i, xposes[q], yposes[q]))
	// 				{
	// 					do_trigger_combo_ffc(i);
	// 					break;
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	
	//
	// Now, let's check for Save combos...
	//
	x1 = tx+4;
	x2 = tx+11;
	y1 = ty+4;
	y2 = ty+11;
	
	types[0] = COMBOTYPE(x1,y1);
	cids[0] = MAPCOMBO(x1,y1);
	
	if(MAPFFCOMBO(x1,y1))
	{
		types[0] = FFCOMBOTYPE(x1,y1);
		cids[0] = MAPFFCOMBO(x1,y1);
	}
	
	types[1] = COMBOTYPE(x1,y2);
	cids[1] = MAPCOMBO(x1,y2);
	
	if(MAPFFCOMBO(x1,y2))
	{
		types[1] = FFCOMBOTYPE(x1,y2);
		cids[1] = MAPFFCOMBO(x1,y2);
	}
		
	types[2] = COMBOTYPE(x2,y1);
	cids[2] = MAPCOMBO(x2,y1);
	
	if(MAPFFCOMBO(x2,y1))
	{
		types[2] = FFCOMBOTYPE(x2,y1);
		cids[2] = MAPFFCOMBO(x2,y1);
	}
		
	types[3] = COMBOTYPE(x2,y2);
	cids[3] = MAPCOMBO(x2,y2);
	
	if(MAPFFCOMBO(x2,y2))
	{
		types[3] = FFCOMBOTYPE(x2,y2);
		cids[3] = MAPFFCOMBO(x2,y2);
	}
		
	
	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].triggerflags[0] & combotriggerONLYGENTRIG)
		{
			if(types[i] == cSAVE || types[i] == cSAVE2)
			{
				types[i] = cNONE;
				setsave = false;
				break;
			}
		}
		if(types[i]==cSAVE) setsave=true;
		
		if(types[i]==cSAVE2) setsave=true;
	}
	
	if(setsave && types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
	{
		last_savepoint_id = cids[0];
		type = types[0];
	}
	//
	// Now, let's check for Drowning combos...
	//
	if(get_qr(qr_DROWN) || CanSideSwim())
	{
		y1 = ty+9;
		y2 = ty+15;
		if (get_qr(qr_SMARTER_WATER))
		{
			if (iswaterex_z3(0, -1, x1, y1, true, false) &&
			iswaterex_z3(0, -1, x1, y2, true, false) &&
			iswaterex_z3(0, -1, x2, y1, true, false) &&
			iswaterex_z3(0, -1, x2, y2, true, false)) water = iswaterex_z3(0, -1, (x2+x1)/2,(y2+y1)/2, true, false);
		}
		else
		{
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
				
			int32_t typec = COMBOTYPE((x2+x1)/2,(y2+y1)/2);
			if(MAPFFCOMBO((x2+x1)/2,(y2+y1)/2))
				typec = FFCOMBOTYPE((x2+x1)/2,(y2+y1)/2);
				
			if(combo_class_buf[types[0]].water && combo_class_buf[types[1]].water &&
					combo_class_buf[types[2]].water && combo_class_buf[types[3]].water && combo_class_buf[typec].water)
				water = typec;
		}
	}
	
	
	// Pits have a bigger 'hitbox' than stairs...
	x1 = tx+7;
	x2 = tx+8;
	y1 = ty+7+(bigHitbox?0:4);
	y2 = ty+8+(bigHitbox?0:4);
	
	types[0] = COMBOTYPE(x1,y1);
	cids[0] = MAPCOMBO(x1,y1);
	
	if(MAPFFCOMBO(x1,y1))
	{
		types[0] = FFCOMBOTYPE(x1,y1);
		cids[0] = MAPFFCOMBO(x1,y1);
	}
		
	types[1] = COMBOTYPE(x1,y2);
	cids[1] = MAPCOMBO(x1,y2);
	
	if(MAPFFCOMBO(x1,y2))
	{
		types[1] = FFCOMBOTYPE(x1,y2);
		cids[1] = MAPFFCOMBO(x1,y2);
	}
	types[2] = COMBOTYPE(x2,y1);
	cids[2] = MAPCOMBO(x2,y1);
	
	if(MAPFFCOMBO(x2,y1))
	{
		types[2] = FFCOMBOTYPE(x2,y1);
		cids[2] = MAPFFCOMBO(x2,y1);
	}
		
	types[3] = COMBOTYPE(x2,y2);
	cids[3] = MAPCOMBO(x2,y2);
	
	if(MAPFFCOMBO(x2,y2))
	{
		types[3] = FFCOMBOTYPE(x2,y2);
		cids[3] = MAPFFCOMBO(x2,y2);
	}
		
	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].triggerflags[0] & combotriggerONLYGENTRIG)
		{
			types[i] = cNONE;
			continue;
		}
		if(types[i]==cPIT) 
		{
			index=0;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cPITB)
		{
			types[i]=cPIT;
			index=1;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cPITC)
		{
			types[i]=cPIT;
			index=2;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
		else if(types[i]==cPITD)
		{
			types[i]=cPIT;
			index=3;
			warpsfx2 = combobuf[cids[i]].attribytes[0];
		}
	}
	
	if(types[0]==cPIT||types[1]==cPIT||types[2]==cPIT||types[3]==cPIT)
		if(action!=freeze&&action!=sideswimfreeze&& (!msg_active || !get_qr(qr_MSGFREEZE)))
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
	
	// Just in case Hero was moved off of the fairy flag
	if(refilling==REFILL_FAIRYDONE)
	{
		fairycircle(REFILL_NONE);
		
		if(fairyclk!=0) return;
	}
	
	if(flag==mfZELDA||flag2==mfZELDA||flag3==mfZELDA || combo_class_buf[type].win_game)
	{
		attackclk = 0; //get rid of Hero's sword if it was stuck out, charged. 
		win_game();
		return;
	}
	
	if((z>0 || fakez>0) && !(tmpscr->flags2&fAIRCOMBOS))
		return;
		
	if((type==cTRIGNOFLAG || type==cTRIGFLAG))
	{ 
		if (COMBOPOS_REGION(tx+8, ty+8)!=stepsecret || get_qr(qr_TRIGGERSREPEAT))
		{
			stepsecret = COMBOPOS_REGION(tx+8, ty+8);
			auto rpos_handle = get_rpos_handle(stepsecret, 0);
			sfx(combobuf[MAPCOMBO(rpos_handle)].attribytes[0],pan((int32_t)x));
			//zprint("Step Secrets Sound: %d\n", combobuf[tmpscr->data[stepsecret]].attribytes[0]);
			
			if(type==cTRIGFLAG && canPermSecret(currdmap, rpos_handle.screen_index))
			{ 
				if(!(rpos_handle.screen->flags5&fTEMPSECRETS)) setmapflag(rpos_handle.screen, rpos_handle.screen_index, mSECRET);
				
				trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.screen_index, false);
			}
			else 
			{
				bool only16_31 = get_qr(qr_STEPTEMP_SECRET_ONLY_16_31)?true:false;
				trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.screen_index, only16_31);
			}
		}
	}
	else if(!didstrig)
	{
		stepsecret = rpos_t::None; 
	}
	
	//Better? Dock collision
	
	// Drown if:
	// * Water (obviously walkable),
	// * Quest Rule allows it,
	// * Not on stepladder,
	// * Not jumping,
	// * Not hovering,
	// * Not rafting,
	// * Not swallowed,
	// * Not a dried lake.
	
	// This used to check for swimming too, but I moved that into the block so that you can drown in higher-leveled water. -Dimi
	
	if(water > 0 && ((get_qr(qr_DROWN) && z==0 && fakez==0 && fall>=0 && fakefall>=0) || CanSideSwim()) && !ladderx && hoverclk==0 && action!=rafting && !inlikelike && !DRIEDLAKE)
	{
		if(current_item(itype_flippers) <= 0 || current_item(itype_flippers) < combobuf[water].attribytes[0] || ((combobuf[water].usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3))) 
		{
			if(!(ladderx+laddery)) drownCombo = water;
			if (combobuf[water].usrflags&cflag1) Drown(1);
			else Drown();
			if(byte drown_sfx = combobuf[water].attribytes[4])
				sfx(drown_sfx, pan(int32_t(x)));
		}
		else if (!isSwimming())
		{
			SetSwim();
			if (!IsSideSwim()) attackclk = charging = spins = 0;
			landswim=0;
			return;
		}
	}
	
	
	if(type==cSTEP)
	{ 
	//zprint2("Hero.HasHeavyBoots(): is: %s\n", ( (Hero.HasHeavyBoots()) ? "true" : "false" ));

		if (COMBOPOS_REGION(tx+8, ty+8) != stepnext)
		{
			stepnext = COMBOPOS_REGION(tx+8, ty+8);
			auto rpos_handle = get_rpos_handle(stepnext, 0);
			int cid = MAPCOMBO(rpos_handle);
			int pos = RPOS_TO_POS(stepnext);
			
			if
		(
			COMBOTYPE(tx+8,ty+8)==cSTEP && /*required item*/
			(!combobuf[cid].attribytes[1] || (combobuf[cid].attribytes[1] && game->item[combobuf[cid].attribytes[1]]) )
			&& /*HEAVY*/
			( ( !(combobuf[cid].usrflags&cflag1) ) || ((combobuf[cid].usrflags&cflag1) && Hero.HasHeavyBoots() ) )
		)
		{
			sfx(combobuf[cid].attribytes[0],pan((int32_t)x));
			rpos_handle.screen->data[pos]++;
		}
			
			if
		(
			COMBOTYPE(tx+8,ty+8)==cSTEPSAME && /*required item*/
			(!combobuf[cid].attribytes[1] || (combobuf[cid].attribytes[1] && game->item[combobuf[cid].attribytes[1]]) )
			&& /*HEAVY*/
			( ( !(combobuf[cid].usrflags&cflag1) ) || ((combobuf[cid].usrflags&cflag1) && Hero.HasHeavyBoots() ) )
		)
			{
				sfx(combobuf[cid].attribytes[0],pan((int32_t)x));
				for(int32_t k=0; k<176; k++)
				{
					if(rpos_handle.screen->data[k]==cid)
					{
						rpos_handle.screen->data[k]++;
					}
				}
			}
			
			if
		(
			COMBOTYPE(tx+8,ty+8)==cSTEPALL && /*required item*/
			(!combobuf[MAPCOMBO(tx+8,ty+8)].attribytes[1] || (combobuf[cid].attribytes[1] && game->item[combobuf[cid].attribytes[1]]) )
			&& /*HEAVY*/
			( ( !(combobuf[cid].usrflags&cflag1) ) || ((combobuf[cid].usrflags&cflag1) && Hero.HasHeavyBoots() ) )
		)
			{
				sfx(combobuf[cid].attribytes[0],pan((int32_t)x));

				// TODO z3 for_every_rpos
				for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
					for(int32_t k=0; k<176; k++)
					{
						if (isStepType(combobuf[screen->data[k]].type))
						{
							screen->data[k]++;
						}
					}
				});				
			}
		}
	}
	else if(type==cSTEPSFX && action == walking)
	{
		trigger_stepfx(get_rpos_handle_for_world_xy(tx + 8, ty + 8, 0), true);
	}
	else stepnext = rpos_t::None;
	
	detail_int[0]=tx;
	detail_int[1]=ty;
	
	
	if(!((type==cCAVE || type==cCAVE2) && z==0 && fakez==0) && type!=cSTAIR &&
			type!=cPIT && type!=cSWIMWARP && type!=cRESET &&
			!(type==cDIVEWARP && isDiving()))
	{
RaftingStuff:
		if (get_qr(qr_BETTER_RAFT_2))
		{
			bool doraft = true;
			if(((int32_t)y>=raftwarpy-12&&(int32_t)y<=raftwarpy+3)&&raftwarpy!=-1)
			{
				if(((int32_t)x>=raftwarpx-8&&(int32_t)x<=raftwarpx+7)&&raftwarpx!=-1)
				{
					doraft = false;
				}
			}
			//if (mfRAFT)
			int32_t rafttypes[2];
			int32_t raftx1 = tx+6;
			int32_t raftx2 = tx+9;
			int32_t rafty = ty+11;
			int32_t raftflags[3];
			rafttypes[0]=rafttypes[1]=-1;
			raftflags[0]=raftflags[1]=raftflags[2]=0;
			rafttypes[0] = MAPFLAG(raftx1,rafty);
			rafttypes[1] = MAPFLAG(raftx2,rafty);
			
			if(rafttypes[0]==rafttypes[1])
				raftflags[0] = rafttypes[0];
				
				
			rafttypes[0] = MAPCOMBOFLAG(raftx1,rafty);
			rafttypes[1] = MAPCOMBOFLAG(raftx2,rafty);
			
			if(rafttypes[0]==rafttypes[1])
				raftflags[1] = rafttypes[0];
				
			rafttypes[0] = MAPFFCOMBOFLAG(raftx1,rafty);
			rafttypes[1] = MAPFFCOMBOFLAG(raftx2,rafty);
			
			if(rafttypes[0]==rafttypes[1])
				raftflags[2] = rafttypes[0];
			
			for (int32_t m = 0; m < 3; ++m)
			{
				if (raftflags[m] == mfRAFT || raftflags[m] == mfRAFT_BRANCH)
				{
					if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && action!=sideswimhit && z==0 && fakez==0 && (combo_class_buf[COMBOTYPE(tx+8, ty+11)].dock || combo_class_buf[FFCOMBOTYPE(tx+8, ty+11)].dock))
					{
						if((isRaftFlag(nextflag(tx,ty+11,dir,false))||isRaftFlag(nextflag(tx,ty+11,dir,true))))
						{
							reset_swordcharge();
							action=rafting; FFCore.setHeroAction(rafting);
							raftclk=0;
							if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
							else sfx(tmpscr->secretsfx);
						}
						else if (get_qr(qr_BETTER_RAFT) && doraft)
						{
							for (int32_t i = 0; i < 4; ++i)
							{
								if(isRaftFlag(nextflag(GridX(tx+8),GridY(ty+11),i,false))||isRaftFlag(nextflag(GridX(tx+8),GridY(ty+11),i,true)))
								{
									reset_swordcharge();
									action=rafting; FFCore.setHeroAction(rafting);
									raftclk=0;
									if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
									else sfx(tmpscr->secretsfx);
									dir = i;
									break;
								}
							}
						}
					}
				}
			}
		}
		if (RaftPass) return;
		switch(flag)
		{
		case mfDIVE_ITEM:
			if(isDiving() && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
			{
				additem(x, y, tmpscr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(tmpscr->secretsfx);
			}
			
			return;
			
		case mfRAFT:
		case mfRAFT_BRANCH:
		
			//		if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
			if (!get_qr(qr_BETTER_RAFT_2))
			{
				bool doraft = true;
				if(((int32_t)y>=raftwarpy-8&&(int32_t)y<=raftwarpy+7)&&raftwarpy!=-1)
				{
					if(((int32_t)x>=raftwarpx-8&&(int32_t)x<=raftwarpx+7)&&raftwarpx!=-1)
					{
						doraft = false;
					}
				}
				if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && action!=sideswimhit && z==0 && fakez==0 && combo_class_buf[type].dock)
				{
					if(isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true)))
					{
						reset_swordcharge();
						action=rafting; FFCore.setHeroAction(rafting);
						raftclk=0;
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
						else sfx(tmpscr->secretsfx);
					}
					else if (get_qr(qr_BETTER_RAFT) && doraft)
					{
						for (int32_t i = 0; i < 4; ++i)
						{
							if(isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,false))||isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,true)))
							{
								reset_swordcharge();
								action=rafting; FFCore.setHeroAction(rafting);
								raftclk=0;
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
								else sfx(tmpscr->secretsfx);
								dir = i;
								break;
							}
						}
					}
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
			if(isDiving() && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
			{
				additem(x, y, tmpscr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(tmpscr->secretsfx);
			}
			
			return;
			
		case mfRAFT:
		case mfRAFT_BRANCH:
		
			//		if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
			if (!get_qr(qr_BETTER_RAFT_2))
			{
				bool doraft = true;
				if(((int32_t)y>=raftwarpy-8&&(int32_t)y<=raftwarpy+7)&&raftwarpy!=-1)
				{
					if(((int32_t)x>=raftwarpx-8&&(int32_t)x<=raftwarpx+7)&&raftwarpx!=-1)
					{
						doraft = false;
					}
				}
				if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && action!=sideswimhit && z==0 && fakez==0 && combo_class_buf[type].dock)
				{
					if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
					{
						reset_swordcharge();
						action=rafting; FFCore.setHeroAction(rafting);
						raftclk=0;
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
						else sfx(tmpscr->secretsfx);
					}
					else if (get_qr(qr_BETTER_RAFT) && doraft)
					{
						for (int32_t i = 0; i < 4; ++i)
						{
							if(isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,false))||isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,true)))
							{
								reset_swordcharge();
								action=rafting; FFCore.setHeroAction(rafting);
								raftclk=0;
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
								else sfx(tmpscr->secretsfx);
								dir = i;
								break;
							}
						}
					}
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
			if(isDiving() && (!getmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (tmpscr->flags9&fBELOWRETURN)))
			{
				additem(x, y, tmpscr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((tmpscr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(tmpscr->secretsfx);
			}
			
			return;
			
		case mfRAFT:
		case mfRAFT_BRANCH:
		
			//	  if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && type==cOLD_DOCK)
			if (!get_qr(qr_BETTER_RAFT_2))
			{
				bool doraft = true;
				if(((int32_t)y>=raftwarpy-8&&(int32_t)y<=raftwarpy+7)&&raftwarpy!=-1)
				{
					if(((int32_t)x>=raftwarpx-8&&(int32_t)x<=raftwarpx+7)&&raftwarpx!=-1)
					{
						doraft = false;
					}
				}
				if(current_item(itype_raft) && action!=rafting && action!=swimhit && action!=gothit && action!=sideswimhit && z==0 && fakez==0 && combo_class_buf[type].dock)
				{
					if((isRaftFlag(nextflag(tx,ty,dir,false))||isRaftFlag(nextflag(tx,ty,dir,true))))
					{
						reset_swordcharge();
						action=rafting; FFCore.setHeroAction(rafting);
						raftclk=0;
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
						else sfx(tmpscr->secretsfx);
					}
					else if (get_qr(qr_BETTER_RAFT) && doraft)
					{
						for (int32_t i = 0; i < 4; ++i)
						{
							if(isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,false))||isRaftFlag(nextflag(GridX(tx+8),GridY(ty+8),i,true)))
							{
								reset_swordcharge();
								action=rafting; FFCore.setHeroAction(rafting);
								raftclk=0;
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x.getInt()));
								else sfx(tmpscr->secretsfx);
								dir = i;
								break;
							}
						}
					}
				}
			}
			
			return;
			
		default:
			return;
		}
	}
	
	// Either the current screen, or if in a 0x80 room the screen player came from.
	mapscr* base_scr = currscr >= 128 ? &special_warp_return_screen : get_scr(currmap, currscr);
	mapscr* cur_scr = get_scr(currmap, currscr);
	
	if((type==cCAVE || type==cCAVE2) && (base_scr->tilewarptype[index]==wtNOWARP)) return;
	
	//don't do this for canceled warps -DD
	//I have no idea why we do this skip, but I'll dutifully propagate it to all cases below...
	/*if(tmpscr[t].tilewarptype[index] != wtNOWARP)
	{
		draw_screen();
		advanceframe(true);
	}*/
	
	bool skippedaframe=false;
	
	if(type==cCAVE || type==cCAVE2 || type==cSTAIR)
	{
		// Stop music only if:
		// * entering a Guy Cave
		// * warping to a DMap whose music is different.
		
		int32_t tdm = base_scr->tilewarpdmap[index];
		
		if(base_scr->tilewarptype[index]<=wtPASS)
		{
			if (FFCore.can_dmap_change_music(tdm))
			{
				if ((DMaps[currdmap].flags & dmfCAVES) && base_scr->tilewarptype[index] == wtCAVE)
					music_stop();
			}
		}
		else
		{
			if (FFCore.can_dmap_change_music(tdm))
			{
				if (zcmusic != NULL)
				{
					if (strcmp(zcmusic->filename, DMaps[tdm].tmusic) != 0 ||
						(zcmusic->type == ZCMF_GME && zcmusic->track != DMaps[tdm].tmusictrack))
						music_stop();
				}
				else if (DMaps[cur_scr->tilewarpdmap[index]].midi != (currmidi - ZC_MIDI_COUNT + 4) &&
					TheMaps[(DMaps[tdm].map * MAPSCRS + (base_scr->tilewarpscr[index] + DMaps[tdm].xoff))].screen_midi != (currmidi - ZC_MIDI_COUNT + 4))
					music_stop();
			}
		}
		
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
		bool opening = (base_scr->tilewarptype[index]<=wtPASS && !(DMaps[currdmap].flags&dmfCAVES && base_scr->tilewarptype[index]==wtCAVE)
						? false : COOLSCROLL);
						
		FFCore.warpScriptCheck();
		draw_screen();
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
		warp_sound = warpsfx2;
	}
	
	
	if (DMaps[currdmap].flags&dmf3STAIR && (currscr==129 || !(DMaps[currdmap].flags&dmfGUYCAVES))
			&& (specialcave > 0 && DMaps[currdmap].flags & dmfGUYCAVES ? special_warp_return_screen : *tmpscr).room==rWARP && type==cSTAIR)
	{
		CHECK(!is_z3_scrolling_mode()); // TODO z3

		if(!skippedaframe)
		{
			FFCore.warpScriptCheck();
			draw_screen();
			advanceframe(true);
		}
		
		// "take any road you want"
		int32_t dw = x<112 ? 1 : (x>136 ? 3 : 2);
		int32_t code = WARPCODE(currdmap,homescr,dw);
		
		if(code>-1)
		{
			bool changedlevel = false;
			bool changeddmap = false;
			if(currdmap != code>>8)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[code>>8].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			currdmap = code>>8;
			dlevel = DMaps[currdmap].level;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			
			currmap = DMaps[currdmap].map;
			homescr = (code&0xFF) + DMaps[currdmap].xoff;
			init_dmap();
			
			if(canPermSecret(currdmap, currscr))
				setmapflag(mSECRET);
		}
		
		if(specialcave==STAIRCAVE) exitcave();
		
		return;
	}
	
	if(type==cRESET)
	{
		if(!skippedaframe)
		{
			FFCore.warpScriptCheck();
			draw_screen();
			advanceframe(true);
		}
		
		if(!(cur_scr->noreset&mSECRET)) unsetmapflag(mSECRET);
		
		if(!(cur_scr->noreset&mITEM)) unsetmapflag(mITEM);
		
		if(!(cur_scr->noreset&mSPECIALITEM)) unsetmapflag(mSPECIALITEM);
		
		if(!(cur_scr->noreset&mNEVERRET)) unsetmapflag(mNEVERRET);
		
		if(!(cur_scr->noreset&mCHEST)) unsetmapflag(mCHEST);
		
		if(!(cur_scr->noreset&mLOCKEDCHEST)) unsetmapflag(mLOCKEDCHEST);
		
		if(!(cur_scr->noreset&mBOSSCHEST)) unsetmapflag(mBOSSCHEST);
		
		if(!(cur_scr->noreset&mLOCKBLOCK)) unsetmapflag(mLOCKBLOCK);
		
		if(!(cur_scr->noreset&mBOSSLOCKBLOCK)) unsetmapflag(mBOSSLOCKBLOCK);
		
		if(isdungeon())
		{
			if(!(cur_scr->noreset&mDOOR_LEFT)) unsetmapflag(mDOOR_LEFT);
			
			if(!(cur_scr->noreset&mDOOR_RIGHT)) unsetmapflag(mDOOR_RIGHT);
			
			if(!(cur_scr->noreset&mDOOR_DOWN)) unsetmapflag(mDOOR_DOWN);
			
			if(!(cur_scr->noreset&mDOOR_UP)) unsetmapflag(mDOOR_UP);
		}
		
		didpit=true;
		pitx=x;
		pity=y;
		sdir=dir;
		dowarp(4,0, warpsfx2);
	}
	else
	{
		if(!skippedaframe && (base_scr->tilewarptype[index]!=wtNOWARP))
		{
			FFCore.warpScriptCheck();
			draw_screen();
			advanceframe(true);
		}
		
		sdir = dir;
		dowarp(0,index, warpsfx2);
	}
}

int32_t selectWlevel(int32_t d)
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
    for(int32_t i=0; i<guys.Count(); i++)
    {
        if(((enemy*)guys.spr(i))->bgsfx)
            stop_sfx(((enemy*)guys.spr(i))->bgsfx);
    }
    if (Hero.action!=inwind) stop_sfx(WAV_ZN1WHIRLWIND);
    if(tmpscr->room==rGANON)
        stop_sfx(WAV_ROAR);
}

bool HeroClass::HasHeavyBoots()
{
	for ( int32_t q = 0; q < MAXITEMS; ++q )
	{
		if ( game->item[q] && ( itemsbuf[q].family == itype_boots ) && /*iron*/ (itemsbuf[q].flags&ITEM_FLAG2) ) return true;
	}
	return false;
}

static bool refresh_dmap_scrollscript = false;
bool HeroClass::dowarp(int32_t type, int32_t index, int32_t warpsfx)
{
	refresh_dmap_scrollscript = false;
	byte reposition_sword_postwarp = 0;
	if(index<0)
	{
		return false;
	}
	is_warping = true;
	for ( int32_t q = 0; q < Lwpns.Count(); ++q )
	{
		weapon *swd=NULL;
		swd = (weapon*)Lwpns.spr(q);
		if(swd->id == (attack==wSword ? wSword : wWand))
		{
			Lwpns.del(q);
		}
	}
	
	attackclk = charging = spins = tapping = 0;
	attack = none;
	if ( warp_sound > 0 ) warpsfx = warp_sound;
	warp_sound = 0;
	word wdmap=0;
	byte wscr=0,wtype=0,t=0;
	bool overlay=false;
	t=(currscr<128)?0:1;
	int32_t wrindex = 0;
	bool wasSideview = isSideViewGravity(t); // (tmpscr[t].flags7 & fSIDEVIEW)!=0 && !ignoreSideview;

	mapscr* cur_scr = get_screen_for_world_xy(x.getInt(), y.getInt());
	// Either the current screen, or if in a 0x80 room the screen player came from.
	mapscr* base_scr = currscr >= 128 ? &special_warp_return_screen : cur_scr;
	
	// Drawing commands probably shouldn't carry over...
	if ( !get_qr(qr_SCRIPTDRAWSINWARPS) )
		script_drawing_commands.Clear();
	
	switch(type)
	{
		case 0:                                                 // tile warp
			wtype = base_scr->tilewarptype[index];
			wdmap = base_scr->tilewarpdmap[index];
			wscr = base_scr->tilewarpscr[index];
			overlay = get_bit(&base_scr->tilewarpoverlayflags,index)?1:0;
			wrindex=(cur_scr->warpreturnc>>(index*2))&3;
			break;
			
		case 1:                                                 // side warp
			wtype = base_scr->sidewarptype[index];
			wdmap = base_scr->sidewarpdmap[index];
			wscr = base_scr->sidewarpscr[index];
			overlay = get_bit(&base_scr->sidewarpoverlayflags,index)?1:0;
			wrindex=(cur_scr->warpreturnc>>(8+(index*2)))&3;
			//tmpscr->doscript = 0; //kill the currebt screen's script so that it does not continue to run during the scroll.
			//there is no doscript for screen scripts. They run like ffcs. 

			break;
			
		case 2:                                                 // whistle warp
		{
			wtype = wtWHISTLE;
			int32_t wind = whistleitem>-1 ? itemsbuf[whistleitem].misc2 : 8;
			int32_t level=0;
			
			if(blowcnt==0)
				level = selectWlevel(0);
			else
			{
				for(int32_t i=0; i<abs(blowcnt); i++)
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
	int32_t olddmap = currdmap;
	rehydratelake(type!=wtSCROLL);
	bool updatemusic = FFCore.can_dmap_change_music(wdmap);
	bool musicnocut = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_NOCUT;
	bool musicrevert = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_REVERT;
	
	switch(wtype)
	{
	case wtCAVE:
	{
		// cave/item room
		ALLOFF();

		// some draw_screen code (the passive subscreen compass dot) depends
		// on currscr and homescr being set first when drawing the screen during a warp.
		// Without this the compass dot would remain drawn while warping.
		// This might be better, but for now this code keeps the rendering
		// equivalent to before z3 draw_screen refactor.
		// demosp253.zplay and first_quest_layered.zplay showcases this behavior.
		// TODO z3: remove this, but when we have more replay coverage.
		currscr_for_passive_subscr = 0x80;
		
		if(DMaps[currdmap].flags&dmfCAVES)                                         // cave
		{
			if (updatemusic || !musicnocut || !get_qr(qr_SCREEN80_OWN_MUSIC))
				music_stop();
			kill_sfx();
			
			int destscr = 0x80;
			if(cur_scr->room==rWARP)
			{
				destscr=0x81;
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
			loadscr(wdmap, destscr, up, false);
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
			if(reposition_sword_postwarp)
			{
				weapon *swd=NULL;
				for(int32_t i=0; i<Lwpns.Count(); i++)
				{
					swd = (weapon*)Lwpns.spr(i);
					
					if(swd->id == (attack==wSword ? wSword : wWand))
					{
					int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
					int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
					positionSword(swd,item_id);
					break;
					}
				}
			}
			stepforward(diagonalMovement?5:6, false);
		}
		else                                                  // item room
		{
			specialcave = ITEMCELLAR;
			map_bkgsfx(false);
			kill_enemy_sfx();

			draw_screen(false);
			
			//unless the room is already dark, fade to black
			if (!get_qr(qr_NEW_DARKROOM))
			{
				if(!darkroom)
				{
					darkroom = true;
					fade(DMaps[currdmap].color,true,false);
				}
			}
			else
				fade(DMaps[currdmap].color, true, false);
			
			blackscr(30,true);

			bool no_x80_dir = true; // TODO: is this necessary?
			loadscr(wdmap, 0x80, down, false, no_x80_dir);
			if ( dontdraw < 2 ) {  dontdraw=1; }
			draw_screen(false);
			fade(0xB,true,true);
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
			if(reposition_sword_postwarp)
			{
				weapon *swd=NULL;
				for(int32_t i=0; i<Lwpns.Count(); i++)
				{
					swd = (weapon*)Lwpns.spr(i);
					
					if(swd->id == (attack==wSword ? wSword : wWand))
					{
					int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
					int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
					positionSword(swd,item_id);
					break;
					}
				}
			}
			lighting(false, true);
			if ( dontdraw < 2 ) { dontdraw=0; }
			stepforward(diagonalMovement?16:18, false);
		}
		if (get_qr(qr_SCREEN80_OWN_MUSIC) && (updatemusic || !musicnocut))
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		break;
	}
	
	case wtPASS:                                            // passageway
	{
		currscr_for_passive_subscr = 0x81;

		map_bkgsfx(false);
		kill_enemy_sfx();
		ALLOFF();
		//play sound
		if(warpsfx > 0) sfx(warpsfx,pan(x.getInt()));
		specialcave = PASSAGEWAY;
		byte warpscr2 = wscr + DMaps[wdmap].xoff;
		draw_screen(false);
		
		if(!get_qr(qr_NEW_DARKROOM))
		{
			if(!darkroom)
				fade(DMaps[currdmap].color,true,false);
				
			darkroom=true;
		}
		else
			fade(DMaps[currdmap].color,true,false);
		blackscr(30,true);
		bool no_x80_dir = true; // TODO z3: is this necessary?
		loadscr(wdmap, 0x81, down, false, no_x80_dir);
		//preloaded freeform combos
		ffscript_engine(true);
		if ( dontdraw < 2 ) { dontdraw=1; }
		draw_screen(false);
		lighting(false, true);
		if (get_qr(qr_NEW_DARKROOM))
			fade(0xB, false, true);
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
		
		y=0;
		set_respawn_point();
		trySideviewLadder();
		reset_hookshot();
		if(reposition_sword_postwarp)
		{
			weapon *swd=NULL;
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				swd = (weapon*)Lwpns.spr(i);
				
				if(swd->id == (attack==wSword ? wSword : wWand))
				{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}
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
		lighting(false,false,pal_litRESETONLY);//Reset permLit, and do nothing else; lighting was not otherwise called on a wtEXIT warp.
		ALLOFF();
		if(updatemusic||!musicnocut)
			music_stop();
		kill_sfx();
		blackscr(30,false);
		bool changedlevel = false;
		bool changeddmap = false;
		if(currdmap != wdmap)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
			changeddmap = true;
		}
		if(dlevel != DMaps[wdmap].level)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
			changedlevel = true;
		}
		dlevel = DMaps[wdmap].level;
		currdmap = wdmap;
		if(changeddmap)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
		}
		if(changedlevel)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
		}
		
		currmap=DMaps[currdmap].map;
		init_dmap();
		update_subscreens(wdmap);
		loadfullpal();
		ringcolor(false);
		loadlvlpal(DMaps[currdmap].color);
		//lastentrance_dmap = currdmap;
		int destscr = wscr + DMaps[currdmap].xoff;
		loadscr(currdmap, destscr, -1, overlay);
		
		if((hero_screen->flags&fDARK) && !get_qr(qr_NEW_DARKROOM))
		{
			if(get_qr(qr_FADE))
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
		
		int32_t wrx,wry;
		
		if(get_qr(qr_NOARRIVALPOINT))
		{
			wrx=hero_screen->warpreturnx[0];
			wry=hero_screen->warpreturny[0];
		}
		else
		{
			wrx=hero_screen->warparrivalx;
			wry=hero_screen->warparrivaly;
		}
		
		if(((wrx>0||wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))&&(!(hero_screen->flags6&fNOCONTINUEHERE)))
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
			if(get_qr(qr_NOARRIVALPOINT))
			{
				x=hero_screen->warpreturnx[wrindex];
				y=hero_screen->warpreturny[wrindex];
			}
			else
			{
				x=hero_screen->warparrivalx;
				y=hero_screen->warparrivaly;
			}
		}
		else
		{
			x=hero_screen->warpreturnx[wrindex];
			y=hero_screen->warpreturny[wrindex];
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

		x += region_scr_dx * 256;
		y += region_scr_dy * 176;
		
		if(dlevel)
		{
			// reset enemy kill counts
			for(int32_t i=0; i<128; i++)
			{
				game->guys[(currmap*MAPSCRSNORMAL)+i] = 0;
				game->maps[(currmap*MAPSCRSNORMAL)+i] &= ~mTMPNORET;
			}
		}
		
		markBmap(dir^1);
		//preloaded freeform combos
		ffscript_engine(true);
	
		reset_hookshot();
		if(reposition_sword_postwarp)
		{
			weapon *swd=NULL;
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				swd = (weapon*)Lwpns.spr(i);
				
				if(swd->id == (attack==wSword ? wSword : wWand))
				{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}

		if(isdungeon())
		{
			openscreen();
			if(get_er(er_SHORTDGNWALK)==0 && get_qr(qr_SHORTDGNWALK)==0)
				stepforward(diagonalMovement?11:12, false);
			else
				// Didn't walk as far pre-1.93, and some quests depend on that
				stepforward(8, false);
		}
		else
		{
			if(!COOLSCROLL)
				openscreen();
				
			int32_t type1 = combobuf[MAPCOMBO(x,y-16)].type; // Old-style blue square placement
			int32_t type2 = combobuf[MAPCOMBO(x,y)].type;
			int32_t type3 = combobuf[MAPCOMBO(x,y+16)].type; // More old-style blue square placement
			
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
		if (updatemusic || !musicnocut)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[currdmap].color;
		dointro();
		set_respawn_point();
		trySideviewLadder();
		
		for(int32_t i=0; i<6; i++)
			visited[i]=-1;
			
		break;
	}
	
	case wtSCROLL:                                          // scrolling warp
	{
		int32_t c = DMaps[currdmap].color;
		scrolling_dmap = currdmap;
		scrolling_map = currmap;
		currmap = DMaps[wdmap].map;
		update_subscreens(wdmap);
		
		dlevel = DMaps[wdmap].level;
		//check if Hero has the map for the new location before updating the subscreen. ? -Z
		//This works only in one direction, if Hero had a map, to not having one.
		//If Hero does not have a map, and warps somewhere where he does, then the map still briefly shows. 
		update_subscreens(wdmap);
		
		/*if ( has_item(itype_map, dlevel) ) 
		{
			//Blank the map during an intra-dmap scrolling warp. 
			dlevel = -1; //a hack for the minimap. This works!! -Z
		}*/
		
		// fix the scrolling direction, if it was a tile or instant warp
		if(type==0 || type>=3)
		{
			sdir = dir;
		}
		
		scrollscr(sdir, wscr+DMaps[wdmap].xoff, wdmap);
		//dlevel = DMaps[wdmap].level; //Fix dlevel and draw the map (end hack). -Z
	
		reset_hookshot();
		if(reposition_sword_postwarp)
		{
			weapon *swd=NULL;
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				swd = (weapon*)Lwpns.spr(i);
				
				if(swd->id == (attack==wSword ? wSword : wWand))
				{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}
		if(!intradmap)
		{
			// TODO z3 I think this can be removed.
			// homescr = currscr = wscr + DMaps[wdmap].xoff;
			init_dmap();
			
			int32_t wrx,wry;
			
			if(get_qr(qr_NOARRIVALPOINT))
			{
				wrx=hero_screen->warpreturnx[0];
				wry=hero_screen->warpreturny[0];
			}
			else
			{
				wrx=hero_screen->warparrivalx;
				wry=hero_screen->warparrivaly;
			}
			
			if(((wrx>0||wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))&&(!get_qr(qr_NOSCROLLCONTINUE))&&(!(tmpscr->flags6&fNOCONTINUEHERE)))
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
		
		if (updatemusic)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[currdmap].color;
		dointro();
	}
	break;
	
	case wtWHISTLE:                                         // whistle warp
	{
		scrolling_dmap = currdmap;
		scrolling_map = currmap;
		currmap = DMaps[wdmap].map;
		scrollscr(index, wscr+DMaps[wdmap].xoff, wdmap);
		reset_hookshot();
		currdmap=wdmap;
		dlevel=DMaps[currdmap].level;
		lighting(false, true);
		init_dmap();
		
		if (updatemusic)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[currdmap].color;
		dointro();
		action=inwind; FFCore.setHeroAction(inwind);
		int32_t wry;
		
		if(get_qr(qr_NOARRIVALPOINT))
			wry=hero_screen->warpreturny[0];
		else wry=hero_screen->warparrivaly;
		
		int32_t wrx;
		
		if(get_qr(qr_NOARRIVALPOINT))
			wrx=hero_screen->warpreturnx[0];
		else wrx=hero_screen->warparrivalx;
		
		Lwpns.add(new weapon((zfix)(index==left?240:index==right?0:wrx),(zfix)(index==down?0:index==up?160:wry),
							 (zfix)0,wWind,1,0,index,whistleitem,getUID(),false,false,true,1));
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
		bool old_192 = false;
		if (get_qr(qr_192b163_WARP))
		{
			if ( wtype == wtIWARPWAVE )
			{
				wtype = wtIWARPWAVE;
				old_192 = true;
			}
			if ( old_192 )
			{
				al_trace("Encountered a warp in a 1.92b163 style quest, that was set as a Wave Warp.\n %s\n", "Trying to redirect it into a Cancel Effect");
				didpit=false;
				update_subscreens();
				warp_sound = 0;
				is_warping = false;
				return false;
			}
		}
		//for determining whether to exit cave
		int32_t type1 = combobuf[MAPCOMBO(x,y-16)].type;
		int32_t type2 = combobuf[MAPCOMBO(x,y)].type;
		int32_t type3 = combobuf[MAPCOMBO(x,y+16)].type;
		
		bool cavewarp = ((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED)
						 ||(type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D));
					
		if(!(hero_screen->flags3&fIWARPFULLSCREEN))
		{
			//ALLOFF kills the action, but we want to preserve Hero's action if he's swimming or diving -DD
			bool wasswimming = (action == swimming);
			int32_t olddiveclk = diveclk;
			ALLOFF();
			
			if(wasswimming)
			{
				Hero.SetSwim();
				diveclk = olddiveclk;
			}
			
			kill_sfx();
		}
		//play sound
		if(warpsfx > 0) sfx(warpsfx,pan(x.getInt()));
		if(wtype==wtIWARPZAP)
		{
			zapout();
		}
		else if(wtype==wtIWARPWAVE)
		{
			//only draw Hero if he's not in a cave -DD
			wavyout(!cavewarp);
		}
		else if(wtype!=wtIWARP)
		{
			bool b2 = COOLSCROLL&&cavewarp;
			blackscr(30,b2?false:true);
		}
		
		int32_t c = DMaps[currdmap].color;
		bool changedlevel = false;
		bool changeddmap = false;
		if(currdmap != wdmap)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
			changeddmap = true;
		}
		if(dlevel != DMaps[wdmap].level)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
			changedlevel = true;
		}
		dlevel = DMaps[wdmap].level;
		currdmap = wdmap;
		if(changeddmap)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
		}
		if(changedlevel)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
		}

		currmap = DMaps[currdmap].map;
		init_dmap();
		update_subscreens(wdmap);
		
		ringcolor(false);
		
		if(DMaps[currdmap].color != c)
			loadlvlpal(DMaps[currdmap].color);
		
		loadscr(currdmap, wscr + DMaps[currdmap].xoff, -1, overlay);
		lightingInstant(); // Also sets naturaldark
		
		x = hero_screen->warpreturnx[wrindex];
		y = hero_screen->warpreturny[wrindex];
		
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

		x += region_scr_dx * 256;
		y += region_scr_dy * 176;

		z3_update_viewport();
		
		markBmap(dir^1);
		
		int32_t checkwater = iswaterex_z3(MAPCOMBO(x,y+8), -1, x,y+(bigHitbox?8:12)); //iswaterex can be intensive, so let's avoid as many calls as we can.
		
		if(checkwater && _walkflag(x,y+(bigHitbox?8:12),0,SWITCHBLOCK_STATE) && current_item(itype_flippers) > 0 && current_item(itype_flippers) >= combobuf[checkwater].attribytes[0] && (!(combobuf[checkwater].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & ITEM_FLAG3)))
		{
			hopclk=0xFF;
			SetSwim();
			if (!IsSideSwim()) attackclk = charging = spins = 0;
		}
		else
		{
			action = none; FFCore.setHeroAction(none);
		}
		//preloaded freeform combos
		ffscript_engine(true);
		
		putscr(scrollbuf,0,0,hero_screen);
		putscrdoors(scrollbuf,0,0,hero_screen);
		
		if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
		{
			reset_pal_cycling();
			putscr(scrollbuf,0,0,hero_screen);
			putscrdoors(scrollbuf,0,0,hero_screen);
			walkup(COOLSCROLL);
		}
		else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
		{
			reset_pal_cycling();
			putscr(scrollbuf,0,0,hero_screen);
			putscrdoors(scrollbuf,0,0,hero_screen);
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
		if(reposition_sword_postwarp)
		{
			weapon *swd=NULL;
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				swd = (weapon*)Lwpns.spr(i);
				
				if(swd->id == (attack==wSword ? wSword : wWand))
				{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}
		show_subscreen_life=true;
		show_subscreen_numbers=true;
		if (updatemusic)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[currdmap].color;
		dointro();
		set_respawn_point();
		trySideviewLadder();
	}
	break;
	
	
	case wtNOWARP:
	{
		bool old_192 = false;
		if (get_qr(qr_192b163_WARP))
		{
			wtype = wtIWARPWAVE;
			old_192 = true;
		}
		if ( old_192 )
		{
			al_trace("Encountered a warp in a 1.92b163 style quest, that was set as a Cancel Warp.\n %s\n", "Trying to redirect it into a Wave Effect");
			//for determining whether to exit cave
			int32_t type1 = combobuf[MAPCOMBO(x,y-16)].type;
			int32_t type2 = combobuf[MAPCOMBO(x,y)].type;
			int32_t type3 = combobuf[MAPCOMBO(x,y+16)].type;
			
			bool cavewarp = ((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED)
					 ||(type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D));
					 
			if(!(hero_screen->flags3&fIWARPFULLSCREEN))
			{
				//ALLOFF kills the action, but we want to preserve Hero's action if he's swimming or diving -DD
				bool wasswimming = (action == swimming);
				int32_t olddiveclk = diveclk;
				ALLOFF();
				
				if(wasswimming)
				{
					Hero.SetSwim();
					diveclk = olddiveclk;
				}
				
				kill_sfx();
			}
			//play sound
			if(warpsfx > 0) sfx(warpsfx,pan(x.getInt()));	
			if(wtype==wtIWARPZAP)
			{
				zapout();
			}
			else if(wtype==wtIWARPWAVE)
			{
				//only draw Hero if he's not in a cave -DD
				wavyout(!cavewarp);
			}
			else if(wtype!=wtIWARP)
			{
				bool b2 = COOLSCROLL&&cavewarp;
				blackscr(30,b2?false:true);
			}
			
			int32_t c = DMaps[currdmap].color;
			bool changedlevel = false;
			bool changeddmap = false;
			if(currdmap != wdmap)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[wdmap].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			dlevel = DMaps[wdmap].level;
			currdmap = wdmap;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			currmap = DMaps[currdmap].map;
			init_dmap();
			update_subscreens(wdmap);
			
			ringcolor(false);
			
			if(DMaps[currdmap].color != c)
				loadlvlpal(DMaps[currdmap].color);
			
			loadscr(currdmap, wscr + DMaps[currdmap].xoff, -1, overlay);
			lightingInstant(); // Also sets naturaldark
			
			x = hero_screen->warpreturnx[wrindex];
			y = hero_screen->warpreturny[wrindex];
			
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

			x += region_scr_dx * 256;
			y += region_scr_dy * 176;
			
			markBmap(dir^1);
			
			if(iswaterex_z3(MAPCOMBO(x,y+8), -1, x,y+8) && _walkflag(x,y+8,0,SWITCHBLOCK_STATE) && current_item(itype_flippers))
			{
				hopclk=0xFF;
				SetSwim();
				if (!IsSideSwim()) attackclk = charging = spins = 0;
			}
			else
			{
				action = none;
				FFCore.setHeroAction(none);
			}
			//preloaded freeform combos
			ffscript_engine(true);
			
			putscr(scrollbuf,0,0,hero_screen);
			putscrdoors(scrollbuf,0,0,hero_screen);
			
			if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
			{
				reset_pal_cycling();
				putscr(scrollbuf,0,0,hero_screen);
				putscrdoors(scrollbuf,0,0,hero_screen);
				walkup(COOLSCROLL);
			}
			else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
			{
				reset_pal_cycling();
				putscr(scrollbuf,0,0,hero_screen);
				putscrdoors(scrollbuf,0,0,hero_screen);
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
			if(reposition_sword_postwarp)
			{
				weapon *swd=NULL;
				for(int32_t i=0; i<Lwpns.Count(); i++)
				{
					swd = (weapon*)Lwpns.spr(i);
					
					if(swd->id == (attack==wSword ? wSword : wWand))
					{
						int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
						int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
						positionSword(swd,item_id);
						break;
					}
				}
			}
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			playLevelMusic();
			currcset=DMaps[currdmap].color;
			dointro();
			set_respawn_point();
			trySideviewLadder();
			break;
		}
		else
		{
			if(reposition_sword_postwarp)
			{
				weapon *swd=NULL;
				for(int32_t i=0; i<Lwpns.Count(); i++)
				{
					swd = (weapon*)Lwpns.spr(i);
					
					if(swd->id == (attack==wSword ? wSword : wWand))
					{
						int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
						int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
						positionSword(swd,item_id);
						break;
					}
				}
			}
			didpit=false;
			update_subscreens();
			warp_sound = 0;
			is_warping = false;
			return false;
		}
	}
	default:
		didpit=false;
		update_subscreens();
		warp_sound = 0;
		is_warping = false;
		if(reposition_sword_postwarp)
		{
			weapon *swd=NULL;
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				swd = (weapon*)Lwpns.spr(i);
				
				if(swd->id == (attack==wSword ? wSword : wWand))
				{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}
		return false;
	}
	
	currscr_for_passive_subscr = -1;
	
	// Stop Hero from drowning!
	if(action==drowning || action==lavadrowning || action==sidedrowning)
	{
		drownclk=0;
		drownclk=0;
		action=none; FFCore.setHeroAction(none);
	}
	
	int32_t checkwater = iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?8:12)), -1, x,y+(bigHitbox?8:12));
	// But keep him swimming if he ought to be!
	// Unless the water is too high levelled, in which case... well, he'll drown on transition probably anyways. -Dimi
	if(action!=rafting && checkwater && (_walkflag(x,y+(bigHitbox?8:12),0,SWITCHBLOCK_STATE) || get_qr(qr_DROWN))
			//&& (current_item(itype_flippers) >= combobuf[checkwater].attribytes[0]) 
		&& (action!=inwind))
	{
		hopclk=0xFF;
		SetSwim();
	}
	
	newscr_clk=frame;
	activated_timed_warp=false;
	eat_buttons();
	
	if(wtype!=wtIWARP)
		attackclk=0;
		
	didstuff=0;
	usecounts.clear();
	map_bkgsfx(true);
	loadside=dir^1;
	whistleclk=-1;
	
	if((z>0 || fakez>0) && isSideViewHero())
	{
		y-=z;
		y-=fakez;
		fakez=0;
		z=0;
	}
	else if(!isSideViewHero())
	{
		fall=0;
		fakefall=0;
	}
	
	// If warping between top-down and sideview screens,
	// fix enemies that are carried over by Full Screen Warp
	const bool tmpscr_is_sideview = isSideViewHero();
	
	if(!wasSideview && tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(guys.spr(i)->z > 0 || guys.spr(i)->fakez > 0)
			{
				guys.spr(i)->y -= guys.spr(i)->z;
				guys.spr(i)->y -= guys.spr(i)->fakez;
				guys.spr(i)->z = 0;
				guys.spr(i)->fakez = 0;
			}
			
			if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
				guys.spr(i)->yofs += 2;
		}
	}
	else if(wasSideview && !tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->family!=eeTRAP && ((enemy*)guys.spr(i))->family!=eeSPINTILE)
				guys.spr(i)->yofs -= 2;
		}
	}
	
	if((DMaps[currdmap].type&dmfCONTINUE) || (currdmap==0&&get_qr(qr_DMAP_0_CONTINUE_BUG)))
	{
		if(dlevel)
		{
			int32_t wrx,wry;
			
			if(get_qr(qr_NOARRIVALPOINT))
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
					|| (((wtype == wtSCROLL) && !intradmap) && ((wrx>0 || wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))))
			{
				if(!(wtype==wtSCROLL)||!(get_qr(qr_NOSCROLLCONTINUE)))
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
					   (char *)moduledata.roomtype_names[special_warp_return_screen.room]);
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
	if(reposition_sword_postwarp)
	{
		weapon *swd=NULL;
		for(int32_t i=0; i<Lwpns.Count(); i++)
		{
			swd = (weapon*)Lwpns.spr(i);
			
			if(swd->id == (attack==wSword ? wSword : wWand))
			{
				int32_t itype = (attack==wFire ? itype_candle : attack==wCByrna ? itype_cbyrna : attack==wWand ? itype_wand : attack==wHammer ? itype_hammer : itype_sword);
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].family==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
			}
		}
	}
	FFCore.clear_combo_scripts();
	if (!intradmap || get_qr(qr_WARPS_RESTART_DMAPSCRIPT))
	{
		FFScript::deallocateAllScriptOwned(ScriptType::DMap, olddmap);
		FFCore.initZScriptDMapScripts();
		FFCore.initZScriptScriptedActiveSubscreen();
		if(refresh_dmap_scrollscript)
		{
			run_scrolling_script_int(false); //Pre-waitdraw
			refresh_dmap_scrollscript = false;
		}
	}
	is_warping = false;
	if(!get_qr(qr_SCROLLWARP_NO_RESET_FRAME))
		GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}

void HeroClass::exitcave()
{
	bool updatemusic = FFCore.can_dmap_change_music(currdmap);
	bool musicnocut = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_NOCUT;

    stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
    loadscr(currdmap, homescr, 255, false);                                   // bogus direction
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
	
	x += region_scr_dx*256;
	y += region_scr_dy*176;
        
    int32_t type1 = combobuf[MAPCOMBO(x,y-16)].type;
    int32_t type2 = combobuf[MAPCOMBO(x,y)].type;
    int32_t type3 = combobuf[MAPCOMBO(x,y+16)].type;
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
	if (updatemusic || !musicnocut)
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
	if (updatemusic || !musicnocut)
		playLevelMusic();
    currcset=DMaps[currdmap].color;
    dointro();
    newscr_clk=frame;
    activated_timed_warp=false;
    dir=down;
    set_respawn_point();
    eat_buttons();
    didstuff=0;
	usecounts.clear();
    map_bkgsfx(true);
    loadside=dir^1;
}


void HeroClass::stepforward(int32_t steps, bool adjust)
{
	if ( FFCore.nostepforward ) return;
	if ( FFCore.temp_no_stepforward ) { FFCore.temp_no_stepforward = 0; return; }
    zfix tx=x;           //temp x
    zfix ty=y;           //temp y
    zfix tstep(0);        //temp single step distance
    zfix s(0);            //calculated step distance for all steps
    z3step=2;
    int32_t sh=shiftdir;
    shiftdir=-1;
    
    for(int32_t i=steps; i>0; --i)
    {
		if(diagonalMovement)
        {
			if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
			{
				tstep = 1.5;
			}
			else
			{
				tstep=z3step;
				z3step=(z3step%2)+1;
			}
        }
        else
        {
			if(get_qr(qr_NEW_HERO_MOVEMENT))
			{
				tstep = 1.5;
			}
			else
			{
				tstep=lsteps[int32_t((dir<left)?ty:tx)&7];
				
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
        }
        
        s+=tstep;
    }
    
    z3step=2;
    
	x = x.getInt();
	y = y.getInt();
    while(s>=0)
    {
        if(diagonalMovement)
        {
            if((dir<left?x.getInt()&7:y.getInt()&7)&&adjust==true)
            {
				if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
				{
					walkable = false;
					shiftdir = -1;
					int32_t tdir=dir<left?(x.getInt()&8?left:right):(y.getInt()&8?down:up);
					switch(tdir)
					{
						case left:
							--x;
							break;
						case right:
							++x;
							break;
						case up:
							--y;
							break;
						case down:
							++y;
							break;
					}
				}
				else
				{
					walkable=false;
					shiftdir=dir<left?(x.getInt()&8?left:right):(y.getInt()&8?down:up);
					moveOld2(dir, 150);
				}
            }
            else
            {
				if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
				{
					s-=1.5;
				}
				else
				{
					s-=z3step;
				}
                walkable=true;
				moveOld2(dir, 150);
            }
            
            shiftdir=-1;
        }
        else
        {
			if((dir<left?x.getInt()&7:y.getInt()&7)&&adjust==true)
            {
                walkable=false;
                int32_t tdir=dir<left?(x.getInt()&8?left:right):(y.getInt()&8?down:up);
				switch(tdir)
				{
					case left:
						--x;
						break;
					case right:
						++x;
						break;
					case up:
						--y;
						break;
					case down:
						++y;
						break;
				}
            }
            else
			{
				if(get_qr(qr_NEW_HERO_MOVEMENT) || IsSideSwim())
				{
					s-=1.5;
				}
				else if(dir<left)
				{
					s-=lsteps[y.getInt()&7];
				}
				else
				{
					s-=lsteps[x.getInt()&7];
				}
				
				moveOld2(dir, 150);
			}
        }
        
        if(s<0)
        {
            // Not quite sure how this is actually supposed to work.
            // There have to be two cases for each direction or Hero
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
        

		clear_darkroom_bitmaps();
        draw_screen();
		if (canSideviewLadder()) setOnSideviewLadder(true);
        advanceframe(true);
        
        if(Quit)
            return;
    }
	if(dir==right||dir==down)
	{
		x=int32_t(x);
		y=int32_t(y);
	}
	else
	{
		x = x.getInt();
		y = y.getInt();
	}
    set_respawn_point();
    draw_screen();
    eat_buttons();
    shiftdir=sh;
}

void HeroClass::walkdown(bool opening) //entering cave
{
    if(opening)
    {
        close_black_opening(x+8, y+8+playing_field_offset, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(x.getInt()));
    clk=0;
    //  int32_t cmby=(y.getInt()&0xF0)+16;
    // Fix Hero's position to the grid
    y=TRUNCATE_TILE(y.getInt());
    action=climbcoverbottom; FFCore.setHeroAction(climbcoverbottom);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=TRUNCATE_TILE(x.getInt());
    climb_cover_y=TRUNCATE_TILE(y.getInt()) + 16;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
	viewport.centering_y_offset = 0;
    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==3)
		{
            ++y;
			--viewport.centering_y_offset;
		}

        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }
	viewport.centering_y_offset = 0;
    
    action=none; FFCore.setHeroAction(none);
}

void HeroClass::walkdown2(bool opening) //exiting cave 2
{
    int32_t type = combobuf[MAPCOMBO(x,y)].type;
    
        
    // Fix Hero's position to the grid
    y=TRUNCATE_TILE(y.getInt());
	
    if((type==cCAVE2)||(type>=cCAVE2B && type<=cCAVE2D))
        y -= 16;
	
    climb_cover_x=TRUNCATE_TILE(x.getInt());
    climb_cover_y=TRUNCATE_TILE(y.getInt());
	
    dir=down;
    z=fakez=fall=fakefall=0;
    
    if(opening)
    {
        open_black_opening(x+8, y+8+playing_field_offset+16, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(x.getInt()));
    clk=0;
    action=climbcovertop; FFCore.setHeroAction(climbcovertop);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
	viewport.centering_y_offset = 16;
    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==3)
		{
            ++y;
			--viewport.centering_y_offset;
		}
            
        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }
	viewport.centering_y_offset = 0;
    
	
    action=none; FFCore.setHeroAction(none);
}

void HeroClass::walkup(bool opening) //exiting cave
{
    int32_t type = combobuf[MAPCOMBO(x,y)].type;
    
    if((type==cCAVE)||(type>=cCAVEB && type<=cCAVED))
        y+=16;
        
    // Fix Hero's position to the grid
	y=TRUNCATE_TILE(y.getInt());
    z=fakez=fall=fakefall=0;
    
    if(opening)
    {
        open_black_opening(x+8, y+8+playing_field_offset-16, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(x.getInt()));
    dir=down;
    clk=0;
    //  int32_t cmby=y.getInt()&0xF0;
    action=climbcoverbottom; FFCore.setHeroAction(climbcoverbottom);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
    climb_cover_x=TRUNCATE_TILE(x.getInt());
    climb_cover_y=TRUNCATE_TILE(y.getInt());
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
	viewport.centering_y_offset = -16;
    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==0)
		{
            --y;
			++viewport.centering_y_offset;
		}
            
        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }
	viewport.centering_y_offset = 0;
    map_bkgsfx(true);
    loadside=dir^1;
    action=none; FFCore.setHeroAction(none);
}

void HeroClass::walkup2(bool opening) //entering cave2
{
    if(opening)
    {
        close_black_opening(x+8, y+8+playing_field_offset, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(x.getInt()));
    dir=up;
    clk=0;
    //  int32_t cmby=y.getInt()&0xF0;
    action=climbcovertop; FFCore.setHeroAction(climbcovertop);
    attack=wNone;
    attackid=-1;
    reset_swordcharge();
	TRUNCATE_TILE(x.getInt());
    climb_cover_x=TRUNCATE_TILE(x.getInt());
    climb_cover_y=TRUNCATE_TILE(y.getInt()) - 16;
    
    guys.clear();
    chainlinks.clear();
    Lwpns.clear();
    Ewpns.clear();
    items.clear();
    
    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==0)
            --y;
            
        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }
    map_bkgsfx(true);
    loadside=dir^1;
    action=none; FFCore.setHeroAction(none);
}

void HeroClass::stepout() // Step out of item cellars and passageways
{
	bool updatemusic = FFCore.can_dmap_change_music(currdmap);
	bool musicnocut = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_NOCUT;
	
	int32_t sc = specialcave; // This gets erased by ALLOFF()
    ALLOFF();
    stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
    map_bkgsfx(false);
    kill_enemy_sfx();
    draw_screen(false);
    fade(sc>=GUYCAVE?10:11,true,false);
    blackscr(30,true);
    ringcolor(false);
    
    if(sc==PASSAGEWAY && abs(x-warpx)>16) // How did Hero leave the passageway?
    {
        currdmap=stepoutdmap;
        currmap=DMaps[currdmap].map;
        dlevel=DMaps[currdmap].level;
        
        //we might have just left a passage, so be sure to update the CSet record -DD
        currcset=DMaps[currdmap].color;
        
        init_dmap();
        homescr=stepoutscr;
    }
    
    loadscr(currdmap, homescr, 255, false);                                   // bogus direction
    draw_screen(false);
    
    if(get_qr(qr_NEW_DARKROOM) || !(tmpscr->flags&fDARK))
    {
        darkroom = naturaldark = false;
        fade(DMaps[currdmap].color,true,true);
    }
    else
    {
        darkroom = naturaldark = true;
        
        if(get_qr(qr_FADE))
        {
            interpolatedfade();
        }
        else
        {
            loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
        }
		byte *si = colordata + CSET(DMaps[currdmap].color*pdLEVEL+poLEVEL)*3;
		si+=3*48;
			
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(9)+i] = _RGB(si);
			tempgreypal[CSET(9)+i] = _RGB(si); //preserve monochrome
			si+=3;
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
    
    set_respawn_point();
    
    // Let's use the 'exit cave' animation if we entered this cellar via a cave combo.
    int32_t type = combobuf[MAPCOMBO(tmpscr->warpreturnx[stepoutwr],tmpscr->warpreturny[stepoutwr])].type;
    
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
	usecounts.clear();
    eat_buttons();
    markBmap(-1);
    map_bkgsfx(true);
    
    if(!get_qr(qr_CAVEEXITNOSTOPMUSIC))
    {
		if (updatemusic || !musicnocut)
		{
			music_stop();
			playLevelMusic();
		}
    }
	else if(get_qr(qr_SCREEN80_OWN_MUSIC))
	{
		if (updatemusic || !musicnocut)
			playLevelMusic();
	}
    
    loadside=dir^1;
}

bool HeroClass::nextcombo_wf(int32_t d2)
{
    if(toogam || (action!=swimming && !IsSideSwim() && action != swimhit) || hopclk==0) //!DIMITODO: ...does swimming just let you ignore smart scrolling entirely!?
        return false;
        
    // assumes Hero is about to scroll screens
    
    int32_t ns = nextscr(d2);
    
    if(ns==0xFFFF)
        return false;
        
    // want actual screen index, not game->maps[] index
    ns = (ns&127) + (ns>>7)*MAPSCRS;
    
    int32_t cx = x;
    int32_t cy = y;
    
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

	if (cx < 0 || cx >= world_w || cy < 0 || cy >= world_h)
		return 0;
    
    int32_t cmb = COMBOPOS(cx%256, cy%176);    
    newcombo c = combobuf[TheMaps[ns].data[cmb]];
    bool dried = iswater_type(c.type) && DRIEDLAKE;
    bool swim = iswater_type(c.type) && (current_item(itype_flippers)) && !dried;
    int32_t b=1;
    
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

// TODO z3: need to test this in region
bool HeroClass::nextcombo_solid(int32_t d2)
{
	if(toogam || currscr>=128)
		return false;
		
	// assumes Hero is about to scroll screens
	
	auto [map, screen] = nextscr2(d2);
	if (map == -1)
		return false;
	
	int32_t cx = x;
	int32_t cy = y;
	cx %= 256;
	cy %= 176;
	
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
	
	int32_t initcx = cx;
	int32_t initcy = cy;
	bool smarter_scroll = get_qr(qr_SMARTER_SMART_SCROLL);
	// from MAPCOMBO()
	
	for(int32_t i=0; i<=((bigHitbox&&!(d2==up||d2==down))?((initcy&7)?2:1):((initcy&7)?1:0)) && cy < world_h; cy+=(cy%2)?7:8,i++)
	{
		cx = initcx;
		for(int32_t k=0; k<=(smarter_scroll?((initcx&7)?2:1):0) && cx < world_w; cx+=(cx%2)?7:8,k++)
		{
			newcombo const& c = combobuf[MAPCOMBO3(map, screen, -1,cx,cy, smarter_scroll)];
		
			int32_t b=1;
			
			if(cx&8) b<<=2;
			
			if(cy&8) b<<=1;
		
			//bool bridgedetected = false;
		
			int32_t walk = c.walk;
			if (smarter_scroll)
			{
				for (int32_t m = 0; m <= 1; m++)
				{
					newcombo const& cmb = combobuf[MAPCOMBO3(map, screen, m,cx,cy, true)];
					if (cmb.type == cBRIDGE) 
					{
						if (!get_qr(qr_OLD_BRIDGE_COMBOS))
						{
							int efflag = (cmb.walk & 0xF0)>>4;
							int newsolid = (cmb.walk & 0xF);
							walk = ((newsolid | walk) & (~efflag)) | (newsolid & efflag);
						}
						else walk &= cmb.walk;
					}
					else walk |= cmb.walk;
				}
			}

			bool swim = iswaterex(MAPCOMBO3(map, screen, -1,cx,cy, smarter_scroll), map, screen, -1, cx, cy, true, false, true) && (current_item(itype_flippers) || action==rafting);
			
			if((walk&b) && !swim)
			{
				return true;
			}
		}
	}
	
	return false;
}

void HeroClass::do_scroll_direction(direction dir)
{
	bool should_scroll = true;

	z3_update_heroscr();

	if((z > 0 || fakez > 0 || stomping) && get_qr(qr_NO_SCROLL_WHILE_IN_AIR))
		should_scroll = false;

	if(lift_wpn && get_qr(qr_NO_SCROLL_WHILE_CARRYING))
		should_scroll = false;

	if(nextcombo_wf(dir))
		should_scroll = false;
	
	int dir_flag = 0;
	if (dir == up)         dir_flag = wfUP;
	else if (dir == down)  dir_flag = wfDOWN;
	else if (dir == left)  dir_flag = wfLEFT;
	else if (dir == right) dir_flag = wfRIGHT;
	// else {} // TODO z3

	mapscr* scr = get_scr(currmap, heroscr);

	if(get_qr(qr_SMARTSCREENSCROLL)&&(!(scr->flags&fMAZE))&&action!=inwind &&action!=scrolling && !(scr->flags2&dir_flag))
	{
		if(nextcombo_solid(dir))
			should_scroll = false;
	}

	if (should_scroll || action == inwind)
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
				action=none; FFCore.setHeroAction(none);
				restart_level();
			}
			else
			{
				dowarp(2,dir);
			}
		}
		else if(scr->flags2&dir_flag && (!(scr->flags8&fMAZEvSIDEWARP) || checkmaze(scr,false)))
		{
			sdir=dir;
			dowarp(1, (scr->sidewarpindex >> (sdir*2))&3);
		}
		else if(!edge_of_dmap(dir) && edge_of_region(dir))
		{
			scrolling_dmap = currdmap;
			scrolling_map = currmap;
			scrollscr(dir);
			
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

// Checks if hero is beyond the bounds of the screen, and if so begins scrolling.
// Returns after scrolling is finished.
void HeroClass::checkscroll()
{
	//DO NOT scroll if Hero is vibrating due to Farore's Wind effect -DD
	if(action == casting||action==sideswimcasting)
		return;

	// This maze logic is enabled for only scrolling mode. It's a bit simpler, but hasn't
	// been tested for non-scrolling mode.
	if (!scrolling_maze_state && is_z3_scrolling_mode() && tmpscr->flags&fMAZE)
	{
		scrolling_maze_scr = currscr;
		scrolling_maze_state = 1;
	}
	int x0 = x.getInt();
	int y0 = y.getInt();
	// TODO z3

	if (scrolling_maze_state && (scrolling_maze_mode == 0 || get_screen_index_for_world_xy(x0, y0) != scrolling_maze_scr))
	{
		mapscr* scrolling_scr = &TheMaps[(currmap*MAPSCRS)+scrolling_maze_scr];
		int x0 = x.getInt();
		int y0 = y.getInt();

		direction advance_dir = dir_invalid;
		if (scrolling_maze_mode == 0)
		{
			if (x0%256 > 256-16) advance_dir = right;
			if (x0 < 0)          advance_dir = left;
			if (y0%176 > 176-16) advance_dir = down;
			if (y0 < 0)          advance_dir = up;
		}
		else if (scrolling_maze_mode == 1)
		{
			if (dir == right) advance_dir = right;
			if (dir == left)  advance_dir = left;
			if (dir == down)  advance_dir = down;
			if (dir == up)    advance_dir = up;
		}

		if (advance_dir != dir_invalid)
		{
			if (maze_enabled_sizewarp(advance_dir))
			{
				scrolling_maze_state = 0;
				return;
			}

			if (checkmaze(scrolling_scr, true))
			{
				if (scrolling_maze_mode == 0)
				{
					int destscr = heroscr;
					if (advance_dir == left)  destscr--;
					if (advance_dir == right) destscr++;
					if (advance_dir == up)    destscr -= 16;
					if (advance_dir == down)  destscr += 16;
					scrollscr(advance_dir, destscr);
				}

				scrolling_maze_state = 0;
				scrolling_maze_scr = 0;
			}
			else if (scrolling_maze_state == 2 || 1)
			{
				// Only adjust hero position if they didn't just enter the maze.
				if (advance_dir == left)  x += 256;
				if (advance_dir == right) x -= 256;
				if (advance_dir == up)    y += 176;
				if (advance_dir == down)  y -= 176;
			}

			//scrolling_maze_state = 2;
		}

		return;
	}
	scrolling_maze_state = 0;

	if (x > world_w-16)
	{
		x = world_w-16;
		do_scroll_direction(right);
	}
	if (x < 0)
	{
		x = 0;
		do_scroll_direction(left);
	}
	if (y > world_h-16)
	{
		y = world_h-16;
		do_scroll_direction(down);
	}
	if (y < 0)
	{
		y = 0;
		do_scroll_direction(up);
	}
}

// assumes current direction is in lastdir[3]
// compares directions with scr->path and scr->exitdir
bool HeroClass::checkmaze(mapscr *scr, bool sound)
{
    if(!(scr->flags&fMAZE))
        return true;
        
    if(lastdir[3]==scr->exitdir)
        return true;
        
    for(int32_t i=0; i<4; i++)
        if(lastdir[i]!=scr->path[i])
            return false;
            
    if(sound)
        sfx(scr->secretsfx);
        
    return true;
}

bool HeroClass::edge_of_dmap(int32_t side)
{
    if(checkmaze(tmpscr,false)==false)
        return false;
        
    // needs fixin'
    // should check dmap style
    switch(side)
    {
    case up:
        return heroscr<16;
        
    case down:
        return heroscr>=112;
        
    case left:
        if((heroscr&15)==0)
            return true;
            
        if((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((heroscr&15)-DMaps[currdmap].xoff)<=0);
            
        break;
        
    case right:
        if((heroscr&15)==15)
            return true;
            
        if((DMaps[currdmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((heroscr&15)-DMaps[currdmap].xoff)>=7);
            
        break;
    }
    
    return false;
}

bool HeroClass::lookaheadraftflag(int32_t d2)
{
    // Helper for scrollscr that gets next combo on next screen.
    // Can use destscr for scrolling warps,
    // but assumes currmap is correct.
    
    int32_t cx = x;
    int32_t cy = y + 8;
	
	bound(cx, 0, world_w - 16); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
	bound(cy, 0, world_h - 8);  //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
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

	auto [map, screen_index] = nextscr2(d2);
	if (map == -1)
		return false;

	mapscr* screen = get_scr(map, screen_index);

    int32_t combo = COMBOPOS(cx%256, cy%176);
    return ( isRaftFlag(combobuf[screen->data[combo]].flag) || isRaftFlag(screen->sflag[combo]));
}

int32_t HeroClass::lookahead(int32_t d2)                       // Helper for scrollscr that gets next combo on next screen.
{
    // Can use destscr for scrolling warps,
    // but assumes currmap is correct.
    
	int32_t cx = vbound(x,0_zf,240_zf); //var = vbound(val, n1, n2), not bound(var, n1, n2) -Z
	int32_t cy = vbound(y + 8,0_zf,160_zf);
	//bound(cx, 0, 240); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
	//bound(cy, 0, 168); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
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

	// TODO z3 !!!!! rm if yuurand / ss_jenny.zplay works
	if (!is_z3_scrolling_mode())
	{
		int32_t combo = (cy&0xF0)+(cx>>4);
		if(combo>175)
			return 0;

		return tmpscr->data[combo];
	}

	auto [map, screen_index] = nextscr2(d2);
	if (map == -1)
		return 0;

	mapscr* screen = get_scr(map, screen_index);

    int32_t combo = COMBOPOS(cx%256, cy%176);
    return screen->data[combo];
}

int32_t HeroClass::lookaheadflag(int32_t d2)
{
    // Helper for scrollscr that gets next combo on next screen.
    // Can use destscr for scrolling warps,
    // but assumes currmap is correct.
    
    int32_t cx = vbound(x,0,world_w-16);
    int32_t cy = vbound(y + 8,0,world_h-16);
	
	//bound(cx, 0, 240); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
	//bound(cy, 0, 168); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
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

	if (cx < 0 || cx >= world_w || cy < 0 || cy >= world_h)
	{
		return 0;
	}

	auto [map, screen_index] = nextscr2(d2);
	if (map == -1)
		return 0;

	mapscr* screen = get_scr(map, screen_index);

    int32_t combo = COMBOPOS(cx%256, cy%176);
    if(!screen->sflag[combo])
    {
        return combobuf[screen->data[combo]].flag;
    }
    
    return screen->sflag[combo];
}

void HeroClass::run_scrolling_script_int(bool waitdraw)
{
	if(waitdraw)
	{
		if ((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
			FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_GLOBAL_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.waitdraw(ScriptType::Player) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::Player, SCRIPT_PLAYER_ACTIVE);
			FFCore.waitdraw(ScriptType::Player) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[currdmap].script,currdmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script,currdmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW);

		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (screen->script != 0 && FFCore.waitdraw(ScriptType::Screen, screen_index) && screen->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, screen->script, screen_index);  
					FFCore.waitdraw(ScriptType::Screen, screen_index) = 0;
				}
			});
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_SCREEN_WAITDRAW);
		if ( !FFCore.system_suspend[susptITEMSCRIPTENGINE] )
		{
			FFCore.itemScriptEngineOnWaitdraw();
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_ITEM_WAITDRAW);
	}
	else
	{
		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (screen->script != 0 && screen->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, screen->script, screen_index);
				}
			});
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_FFCS);
		if ((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.doscript(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_GLOBAL_ACTIVE);
		if ((!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.doscript(ScriptType::Player) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255)
		{
			ZScriptVersion::RunScript(ScriptType::Player, SCRIPT_PLAYER_ACTIVE);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_ACTIVE);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[currdmap].script,currdmap);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script,currdmap);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN);
		bool old = get_qr(qr_OLD_ITEMDATA_SCRIPT_TIMING);
		if(!FFCore.system_suspend[susptITEMSCRIPTENGINE] && old)
			FFCore.itemScriptEngine();
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_OLD_ITEMDATA_SCRIPT);
		if(!FFCore.system_suspend[susptITEMSCRIPTENGINE] && !old)
			FFCore.itemScriptEngine();
	}
}

static zfix new_hero_x, new_hero_y;
static int new_region_offset_x, new_region_offset_y;

// Scripts expect coordinates to be relative to the new screen coordinate system, so we must convert them.
void HeroClass::run_scrolling_script(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
{
	// For rafting (and possibly other esoteric things)
	// Hero's action should remain unchanged while scrolling,
	// but for the sake of scripts, here's an eye-watering kludge.
	actiontype lastaction = action;
	action=scrolling; FFCore.setHeroAction(scrolling);
	if(waitdraw)
	{
		FFCore.runGenericPassiveEngine(SCR_TIMING_WAITDRAW);
	}
	else
	{
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_FFCS-1);
	}
	zfix storex = x, storey = y;
	auto store_viewport = viewport;
	switch(scrolldir)
	{
	case up:
		if(y < world_h - 16) y = world_h;
		else if(cx > 0 && !end_frames) y = sy + world_h - 20;
		else y = world_h - 16;

		x = new_hero_x;
		break;
		
	case down:
		if(y > 0) y = -16;
		else if(cx > 0 && !end_frames) y = sy - 172;
		else y = 0;

		x = new_hero_x;
		break;
		
	case left:
		if(x < world_w - 16) x = world_w;
		else if(cx > 0) x = sx + world_w - 20;
		else x = world_w - 16;

		y = new_hero_y;
		break;
		
	case right:
		if(x > 0) x = -16;
		else if(cx > 0)	x = sx - 252;
		else x = 0;

		y = new_hero_y;
		break;
	}

	// x += 
	// y += playing_field_offset;

	viewport.x -= new_region_offset_x;
	viewport.y -= new_region_offset_y;

	run_scrolling_script_int(waitdraw);
	
	x = storex, y = storey;
	viewport = store_viewport;
	
	action=lastaction; FFCore.setHeroAction(lastaction);
}

//Has solving the maze enabled a side warp?
//Only used just before scrolling screens
// Note: since scrollscr() calls this, and dowarp() calls scrollscr(),
// return true to abort the topmost scrollscr() call. -L
bool HeroClass::maze_enabled_sizewarp(int32_t scrolldir)
{
    for(int32_t i = 0; i < 3; i++) lastdir[i] = lastdir[i+1];
    
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

int32_t HeroClass::get_scroll_step(int32_t scrolldir)
{
	// For side-scrollers, where the relative speed of 'fast' scrolling is a bit slow.
	if(get_qr(qr_VERYFASTSCROLLING))
		return 16;

    if(get_qr(qr_SMOOTHVERTICALSCROLLING) != 0)
    {
        return (isdungeon() && !get_qr(qr_FASTDNGN)) ? 2 : 4;
    }
    else
    {
        if(scrolldir == up || scrolldir == down)
        {
            return 8;
        }
        else
        {
            return (isdungeon() && !get_qr(qr_FASTDNGN)) ? 2 : 4;
        }
    }
}

int32_t HeroClass::get_scroll_delay(int32_t scrolldir)
{
	if(get_qr(qr_NOSCROLL))
		return 0;
        
	if( (get_qr(qr_VERYFASTSCROLLING) != 0) ||
	    (get_qr(qr_SMOOTHVERTICALSCROLLING) != 0) )
	{
		return 1;
	}
	else
	{
		if(scrolldir == up || scrolldir == down)
		{
			return (isdungeon() && !get_qr(qr_FASTDNGN)) ? 4 : 2;
		}
		else
		{
			return 1;
		}
	}
}

void HeroClass::calc_darkroom_hero(int32_t x1, int32_t y1, BITMAP* bmp)
{
	if(!get_qr(qr_NEW_DARKROOM)) return;
	int32_t lampid = current_item_id(itype_lantern);
	if(lampid < 0) return;
	static bool lamp_paid = false;
	if(!(checkbunny(lampid) && checkmagiccost(lampid,lamp_paid)))
	{
		lamp_paid = false;
		return;
	}
	lamp_paid = true;
	paymagiccost(lampid,false,true);

	int32_t hx = x.getInt() - x1 + 8;
	int32_t hy = y.getInt() - y1 + 8;
	
	itemdata& lamp = itemsbuf[lampid];
	handle_lighting(hx, hy, lamp.misc1, lamp.misc2, dir, bmp);
}

static void for_every_nearby_screen_during_scroll(
	const std::vector<mapscr*>& old_temporary_screens,
	const std::function <void (std::array<screen_handle_t, 7>, int, int, int, bool)>& fn)
{
	int old_region = get_region_id(scrolling_dmap, scrolling_scr);
	int new_region = get_region_id(scrolling_destdmap, currscr);
	bool is_region_scrolling = old_region || new_region;

	int start_dy = -1;
	int end_dy = 1;
	if (scrolling_dir == up) start_dy -= 1;
	if (scrolling_dir == down) end_dy += 1;

	// First handle the old screens, then the new screens.
	std::vector<std::pair<int, int>> old_screen_deltas;
	std::vector<std::pair<int, int>> new_screen_deltas;

	// Note: (draw_dx = 0, draw_dy = 0) denotes the starting screen (currscr),
	// while (   < scrolling_dir >    ) denotes the destination screen (scrolling_scr).
	for (int draw_dx = -1; draw_dx <= 1; draw_dx++)
	{
		for (int draw_dy = start_dy; draw_dy <= end_dy; draw_dy++)
		{
			if (!is_region_scrolling)
			{
				if (draw_dx && draw_dy) continue;
			}

			// Depending on which direction we are scrolling, need to select the correct set of screens.
			bool use_new_screens = XY_DELTA_TO_DIR(draw_dx, 0) == scrolling_dir || XY_DELTA_TO_DIR(0, sign2(draw_dy)) == scrolling_dir;
			if (use_new_screens)
			{
				new_screen_deltas.push_back({draw_dx, draw_dy});
			}
			else
			{
				old_screen_deltas.push_back({draw_dx, draw_dy});
			}
		}
	}

	std::vector<std::pair<int, int>> screen_deltas;
	screen_deltas.insert(screen_deltas.end(), old_screen_deltas.begin(), old_screen_deltas.end());
	screen_deltas.insert(screen_deltas.end(), new_screen_deltas.begin(), new_screen_deltas.end());

	for (auto pair : screen_deltas)
	{
		int draw_dx = pair.first;
		int draw_dy = pair.second;
		bool use_new_screens = XY_DELTA_TO_DIR(draw_dx, 0) == scrolling_dir || XY_DELTA_TO_DIR(0, sign2(draw_dy)) == scrolling_dir;
		int base_map = use_new_screens ? currmap : scrolling_map;
		int base_dmap = use_new_screens ? scrolling_destdmap : scrolling_dmap;
		int base_scr = use_new_screens ? heroscr : scrolling_scr;
		int base_scr_x = base_scr % 16;
		int base_scr_y = base_scr / 16;

		if (use_new_screens)
		{
			if (scrolling_dir == up || scrolling_dir == down) base_scr_y -= sign2(draw_dy);
			else                                              base_scr_x -= draw_dx;
		}

		int scr_x = base_scr_x + draw_dx;
		int scr_y = base_scr_y + draw_dy;
		if (scr_x < 0 || scr_x >= 16 || scr_y < 0 || scr_y >= 8) continue;
		
		int scr = scr_x + scr_y * 16;
		if (!is_region_scrolling && scr != heroscr && scr != scrolling_scr) continue;

		// Only show screens that are in the old or the new regions.
		int region = get_region_id(base_dmap, scr);
		if (!(scr == scrolling_scr || scr == currscr || (old_region && old_region == region) || (new_region && region == new_region)))
			continue;

		mapscr* base_screen = use_new_screens ?
			get_scr(base_map, scr) :
			old_temporary_screens[scr*7];
		if (base_screen)
		{
			int dx = draw_dx + z3_get_region_relative_dx(scrolling_scr, scrolling_origin_scr);
			int dy = draw_dy + z3_get_region_relative_dy(scrolling_scr, scrolling_origin_scr);
			// int dx = draw_dx;
			// int dy = draw_dy;

			// if (scrolling_dir == up)    dy += 1;
			// if (scrolling_dir == down)  dy -= 1;
			// if (scrolling_dir == left)  dx += 1;
			// if (scrolling_dir == right) dx -= 1;

			std::array<screen_handle_t, 7> screen_handles;
			screen_handles[0] = {base_screen, base_screen, base_map, scr, 0};
			for (int i = 1; i < 7; i++)
			{
				mapscr* screen = use_new_screens ?
					get_layer_scr(base_map, scr, i - 1) :
					old_temporary_screens[scr*7 + i];
				screen_handles[i] = {base_screen, screen, base_map, scr, i};
			}

			fn(screen_handles, scr, dx, dy, use_new_screens);
		}
	}
}

static int dir_to_scr_offset(direction dir)
{
	if (dir == up) return -16;
	if (dir == down) return 16;
	if (dir == left) return -1;
	if (dir == right) return 1;
	return 0;
}

static void scrollscr_handle_dark(mapscr* newscr, mapscr* oldscr, std::vector<mapscr*>& old_temporary_screens)
{
	clear_darkroom_bitmaps();

	if (scrolling_use_new_dark_code)
	{
		set_clip_rect(framebuf, 0, playing_field_offset, 256, framebuf->h);

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			mapscr* base_screen = screen_handles[0].base_screen;
			int offx = draw_dx * 256;
			int offy = draw_dy * 176 + playing_field_offset;
			bool should_be_dark = (base_screen->flags & fDARK) && (scr == currscr || get_qr(qr_NEWDARK_SCROLLEDGE));
			if (!should_be_dark)
			{
				rectfill(darkscr_bmp_z3, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
				rectfill(darkscr_bmp_z3_trans, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
			}
		});

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			if (scr != currscr && !get_qr(qr_NEWDARK_SCROLLEDGE))
				return;

			mapscr* base_screen = screen_handles[0].base_screen;
			int offx = draw_dx * 256;
			int offy = draw_dy * 176 + playing_field_offset;
			if (base_screen->flags & fDARK)
			{
				calc_darkroom_combos(scr, offx, offy, darkscr_bmp_z3);
			}
		});
		Hero.calc_darkroom_hero(0, -playing_field_offset, darkscr_bmp_z3);

		color_map = &trans_table2;
		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			mapscr* base_screen = screen_handles[0].base_screen;

			int offx = draw_dx * 256;
			int offy = draw_dy * 176 + playing_field_offset;
			int left = offx - viewport.x;
			int top = offy - viewport.y;
			set_clip_rect(framebuf, left, top, left + 256, top + 176);
			add_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);

			if (base_screen->flags9 & fDARK_DITHER)
			{
				set_clip_rect(darkscr_bmp_z3, left, top, left + 256, top + 176);
				set_clip_rect(darkscr_bmp_z3_trans, left, top, left + 256, top + 176);
				ditherblit_clipped(darkscr_bmp_z3,darkscr_bmp_z3,0,game->get_dither_type(),game->get_dither_arg());
				ditherblit_clipped(darkscr_bmp_z3_trans,darkscr_bmp_z3_trans,0,game->get_dither_type(),game->get_dither_arg());
				set_clip_rect(darkscr_bmp_z3, 0, 0, darkscr_bmp_z3->w, darkscr_bmp_z3->h);
				set_clip_rect(darkscr_bmp_z3_trans, 0, 0, darkscr_bmp_z3_trans->w, darkscr_bmp_z3_trans->h);
			}

			if (base_screen->flags9 & fDARK_TRANS) //draw the dark as transparent
				draw_trans_sprite(framebuf, darkscr_bmp_z3, 0, 0);
			else
				masked_blit(darkscr_bmp_z3, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
		});
		draw_trans_sprite(framebuf, darkscr_bmp_z3_trans, 0, 0);
		color_map = &trans_table;

		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
	}
	else
	{
		calc_darkroom_combos_old(currscr, viewport.x, viewport.y, true);
		Hero.calc_darkroom_hero(FFCore.ScrollingData[SCROLLDATA_NX], FFCore.ScrollingData[SCROLLDATA_NY], darkscr_bmp_curscr);
		Hero.calc_darkroom_hero(FFCore.ScrollingData[SCROLLDATA_OX], FFCore.ScrollingData[SCROLLDATA_OY], darkscr_bmp_scrollscr);

		set_clip_rect(framebuf, 0, playing_field_offset, 256, 168+playing_field_offset);
		int32_t dx1 = FFCore.ScrollingData[SCROLLDATA_NX], dy1 = FFCore.ScrollingData[SCROLLDATA_NY]+playing_field_offset;
		int32_t dx2 = FFCore.ScrollingData[SCROLLDATA_OX], dy2 = FFCore.ScrollingData[SCROLLDATA_OY]+playing_field_offset;
		if(newscr->flags & fDARK)
		{
			if(newscr->flags9 & fDARK_DITHER) //dither the entire bitmap
			{
				ditherblit(darkscr_bmp_curscr,darkscr_bmp_curscr,0,game->get_dither_type(),game->get_dither_arg());
				ditherblit(darkscr_bmp_curscr_trans,darkscr_bmp_curscr_trans,0,game->get_dither_type(),game->get_dither_arg());
			}
			
			color_map = &trans_table2;
			if(newscr->flags9 & fDARK_TRANS) //draw the dark as transparent
				draw_trans_sprite(framebuf, darkscr_bmp_curscr, dx1, dy1);
			else 
				masked_blit(darkscr_bmp_curscr, framebuf, 0, 0, dx1, dy1, 256, 176);
			draw_trans_sprite(framebuf, darkscr_bmp_curscr_trans, dx1, dy1);
			color_map = &trans_table;
		}
		if(oldscr->flags & fDARK)
		{
			if(oldscr->flags9 & fDARK_DITHER) //dither the entire bitmap
			{
				ditherblit(darkscr_bmp_scrollscr,darkscr_bmp_scrollscr,0,game->get_dither_type(),game->get_dither_arg());
				ditherblit(darkscr_bmp_scrollscr_trans,darkscr_bmp_scrollscr_trans,0,game->get_dither_type(),game->get_dither_arg());
			}
			
			color_map = &trans_table2;
			if(oldscr->flags9 & fDARK_TRANS) //draw the dark as transparent
				draw_trans_sprite(framebuf, darkscr_bmp_scrollscr, dx2, dy2);
			else 
				masked_blit(darkscr_bmp_scrollscr, framebuf, 0, 0, dx2, dy2, 256, 176);
			draw_trans_sprite(framebuf, darkscr_bmp_scrollscr_trans, dx2, dy2);
			color_map = &trans_table;
		}
		set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
	}
}

// TODO z3 ! cleanup dead code
void HeroClass::scrollscr(int32_t scrolldir, int32_t destscr, int32_t destdmap)
{
	if(action==freeze||action==sideswimfreeze)
	{
		return;
	}

	bool overlay = false;
	if(scrolldir >= 0 && scrolldir <= 3)
	{
		overlay = get_bit(&(currscr >= 128 ? special_warp_return_screen : *tmpscr).sidewarpoverlayflags, scrolldir) ? true : false;
	}

	int old_dmap = currdmap;
	int new_dmap = destdmap >= 0 ? destdmap : currdmap;

	bool updatemusic = FFCore.can_dmap_change_music(destdmap);
	bool musicrevert = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_REVERT;

	if (!is_z3_scrolling_mode() && maze_enabled_sizewarp(scrolldir))  // dowarp() was called
		return;

	int original_destscr = destscr;
	if (destscr == -1)
	{
		destscr = heroscr;
		if (checkmaze(tmpscr, false) && !edge_of_dmap(scrolldir)) {
			destscr += dir_to_scr_offset((direction)scrolldir);
		}
	}

	// Determine what the player position will be after scrolling (within the new screen's coordinate system),
	// and what the new viewport will be.
	// zfix new_hero_x, new_hero_y;
	viewport_t new_viewport = {0};
	region new_region;
	new_hero_x = 0;
	new_hero_y = 0;
	{
		int scr_dx, scr_dy;
		int odmap = currdmap;
		int oscr = currscr;
		// TODO z3 !!!!
		currdmap = new_dmap;
		currscr = destscr;
		z3_calculate_region(new_dmap, destscr, new_region, scr_dx, scr_dy);
		currdmap = odmap;
		currscr = oscr;

		switch(scrolldir)
		{
			case up:
			{
				new_hero_x.val = (scr_dx*256) * 10000L + x.val%(256*10000L);
				new_hero_y = new_region.height - 16;
			}
			break;
			
			case down:
			{
				new_hero_x.val = (scr_dx*256) * 10000L + x.val%(256*10000L);
				new_hero_y = 0;
			}
			break;
			
			case left:
			{
				new_hero_x = new_region.width - 16;
				new_hero_y.val = (scr_dy*176) * 10000L + y.val%(176*10000L);
			}
			break;
			
			case right:
			{
				new_hero_x = 0;
				new_hero_y.val = (scr_dy*176) * 10000L + y.val%(176*10000L);
			}
			break;

			// Should never happen ...
			default:
			{
				abort();
			}
		}

		z3_calculate_viewport(new_dmap, destscr, new_region.width, new_region.height, new_hero_x, new_hero_y, new_viewport);
	}

	int step = get_scroll_step(scrolldir);
	int delay = get_scroll_delay(scrolldir);
	int scroll_counter, dx, dy;
	{
		int scroll_height = std::min(viewport.h, new_viewport.h);
		int scroll_width = std::min(viewport.w, new_viewport.w);
		int scroll_amount = scrolldir == up || scrolldir == down ? scroll_height : scroll_width;
		scroll_counter = scroll_amount / step;

		dx = 0;
		dy = 0;
		if (scrolldir == up)    dy = -1;
		if (scrolldir == down)  dy = 1;
		if (scrolldir == left)  dx = -1;
		if (scrolldir == right) dx = 1;
	}

	// Determine by how much we need to align to the new region's viewport.
	// This sets `secondary_axis_alignment_amount` to the number of pixels needed to adjust along the secondary axis
	// to move (the position of link relative to the display) from the old viewport to the new viewport.
	int secondary_axis_alignment_amount;
	{
		int old_origin_scr_x = cur_origin_screen_index % 16;
		int old_origin_scr_y = cur_origin_screen_index / 16;
		int old_hero_screen_x = x.getInt() - viewport.x;
		int old_hero_screen_y = y.getInt() - viewport.y + (232 - viewport.h);
		// int old_hero_screen_y = y.getInt() - viewport.y;
		int new_hero_screen_x = new_hero_x - new_viewport.x;
		int new_hero_screen_y = new_hero_y - new_viewport.y + (232 - new_viewport.h);
		// int new_hero_screen_y = new_hero_y - new_viewport.y;
		if (dx)      secondary_axis_alignment_amount = new_hero_screen_y - old_hero_screen_y;
		else if (dy) secondary_axis_alignment_amount = new_hero_screen_x - old_hero_screen_x;
		else         secondary_axis_alignment_amount = 0;
	}

	bool isForceFaceUp = getOnSideviewLadder() && canSideviewLadder() &&
		!(jumping<0 || fall!=0 || fakefall!=0) && get_qr(qr_SIDEVIEWLADDER_FACEUP);
	if (isForceFaceUp) dir = up;

	bool is_unsmooth_vertical_scrolling =
		(scrolldir == up || scrolldir == down) && get_qr(qr_SMOOTHVERTICALSCROLLING) == 0;

	// These mark the top-left coordinate of the new region and the old region, relative to the old region world coordinates.
	new_region_offset_x = (z3_get_region_relative_dx(new_region.origin_screen_index, cur_origin_screen_index)) * 256;
	new_region_offset_y = (z3_get_region_relative_dy(new_region.origin_screen_index, cur_origin_screen_index)) * 176;

	kill_enemy_sfx();
	stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
	screenscrolling = true;
	FFCore.ScrollingData[SCROLLDATA_DIR] = scrolldir;
	switch(scrolldir)
	{
		case up:
			FFCore.ScrollingData[SCROLLDATA_NX] = 0;
			FFCore.ScrollingData[SCROLLDATA_NY] = -176;
			break;
		case down:
			FFCore.ScrollingData[SCROLLDATA_NX] = 0;
			FFCore.ScrollingData[SCROLLDATA_NY] = 176;
			break;
		case left:
			FFCore.ScrollingData[SCROLLDATA_NX] = -256;
			FFCore.ScrollingData[SCROLLDATA_NY] = 0;
			break;
		case right:
			FFCore.ScrollingData[SCROLLDATA_NX] = 256;
			FFCore.ScrollingData[SCROLLDATA_NY] = 0;
			break;
	}
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_OFFSET_X] = new_region_offset_x;
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_OFFSET_Y] = new_region_offset_y;
	FFCore.ScrollingData[SCROLLDATA_NRX] = new_region_offset_x - viewport.x;
	FFCore.ScrollingData[SCROLLDATA_NRY] = new_region_offset_y - viewport.y;
	FFCore.ScrollingData[SCROLLDATA_ORX] = -viewport.x;
	FFCore.ScrollingData[SCROLLDATA_ORY] = -viewport.y;

	// Get the screen coords of the top-left of the screen we are scrolling away from.
	std::tie(FFCore.ScrollingData[SCROLLDATA_OX], FFCore.ScrollingData[SCROLLDATA_OY]) =
		translate_screen_coordinates_to_world(heroscr);
	FFCore.ScrollingData[SCROLLDATA_OX] -= viewport.x;
	FFCore.ScrollingData[SCROLLDATA_OY] -= viewport.y;

	FFCore.ScrollingData[SCROLLDATA_NPX] = new_hero_x.getInt();
	FFCore.ScrollingData[SCROLLDATA_NPY] = new_hero_y.getInt();

	FFCore.ScrollingData[SCROLLDATA_OPX] = x.getInt();
	FFCore.ScrollingData[SCROLLDATA_OPY] = y.getInt();

	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_WIDTH] = new_region.screen_width;
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_HEIGHT] = new_region.screen_height;

	FFCore.ScrollingData[SCROLLDATA_OLD_REGION_WIDTH] = region_scr_width;
	FFCore.ScrollingData[SCROLLDATA_OLD_REGION_HEIGHT] = region_scr_height;

	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_WIDTH] = new_viewport.w;
	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_HEIGHT] = new_viewport.h;

	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_WIDTH] = viewport.w;
	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_HEIGHT] = viewport.h;

	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_X] = new_viewport.x;
	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_Y] = new_viewport.y;

	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_X] = viewport.x;
	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_Y] = viewport.y;

	FFCore.clear_combo_scripts();
	
	// expose previous screen to scripting.
	special_warp_return_screen = *tmpscr;
	
	for(int32_t i = 0; i < 6; i++)
	{
		tmpscr3[i] = tmpscr2[i];
	}

	mapscr *oldscr = &special_warp_return_screen;
	conveyclk = 2;
	scrolling_dir = (direction) scrolldir;
	scrolling_scr = heroscr;
	scrolling_origin_scr = cur_origin_screen_index;

	int32_t scx = get_qr(qr_FASTDNGN) ? 30 : 0;
	if(get_qr(qr_VERYFASTSCROLLING)) //just a minor adjustment.
		scx = 32; //for sideview very fast screolling.
	
	auto hero_x_before_scripts = x;
	auto hero_y_before_scripts = y;

	int32_t lastattackclk = attackclk, lastspins = spins, lastcharging = charging; bool lasttapping = tapping;
	actiontype lastaction = action;
	{
		ALLOFF(false, false);
		// for now, restore Hero's previous action
		if(!get_qr(qr_SCROLLING_KILLS_CHARGE))
			attackclk = lastattackclk; spins = lastspins; charging = lastcharging; tapping = lasttapping;
		action=lastaction; FFCore.setHeroAction(lastaction);

		lstep = (lstep + 6) % 12;
		FFCore.runGenericPassiveEngine(SCR_TIMING_WAITDRAW);
		if((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
			FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_GLOBAL_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.waitdraw(ScriptType::Player) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::Player, SCRIPT_PLAYER_ACTIVE);
			FFCore.waitdraw(ScriptType::Player) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[currdmap].script,currdmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script,currdmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW);

		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_screen_in_region([&](mapscr* screen, int screen_index, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (screen->script != 0 && FFCore.waitdraw(ScriptType::Screen, screen_index))
				{
					ZScriptVersion::RunScript(ScriptType::Screen, screen->script, screen_index);  
					FFCore.waitdraw(ScriptType::Screen, screen_index) = 0;
				}
			});
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_SCREEN_WAITDRAW);

		for_every_ffc_in_region([&](const ffc_handle_t& ffc_handle) {
			if (ffc_handle.ffc->script != 0 && FFCore.waitdraw(ScriptType::FFC, ffc_handle.id))
			{
				ZScriptVersion::RunScript(ScriptType::FFC, ffc_handle.ffc->script, ffc_handle.id);
				FFCore.waitdraw(ScriptType::FFC, ffc_handle.id) = false;
			}
		});

		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_FFC_WAITDRAW);
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_COMBO_WAITDRAW);
		//Waitdraw for item scripts. 
		FFCore.itemScriptEngineOnWaitdraw();
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_ITEM_WAITDRAW);
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_NPC_WAITDRAW);
		
		//Sprite scripts on Waitdraw
		FFCore.eweaponScriptEngineOnWaitdraw();
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_EWPN_WAITDRAW);
		FFCore.itemSpriteScriptEngineOnWaitdraw();
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_ITEMSPRITE_WAITDRAW);
		
		//This is no longer a do-while, as the first iteration is now slightly different. -Em
		draw_screen(true,true);
		
		rehydratelake(false);
			
		FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
	}
	
	advanceframe(true);
	if(Quit)
	{
		screenscrolling = false;
		return;
	}

	// Calling functions are responsible for setting currmap (but not currscr...), but before we _actually_
	// start to scroll we draw a few frames of the current screen (draw_screen). So we need currmap to be the
	// old value initially. Callers also set the old map value to `scrolling_map`, so we can use that.
	int destmap = currmap;
	currmap = scrolling_map;

	// Wait
	{
		int wait_counter = scx + 1;
		while (wait_counter < 32)
		{
			if(get_qr(qr_FIXSCRIPTSDURINGSCROLLING))
			{
				script_drawing_commands.Clear();
				FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);
				ZScriptVersion::RunScrollingScript(scrolldir, wait_counter, 0, 0, false, false); // Prewaitdraw
				ZScriptVersion::RunScrollingScript(scrolldir, wait_counter, 0, 0, false, true); // Waitdraw
			}
			else FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);
			draw_screen(true,true);
			
			if (wait_counter == scx)
				rehydratelake(false);
				
			FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
			advanceframe(true);
			
			if(Quit)
			{
				screenscrolling = false;
				return;
			}
			
			++wait_counter;
		}
	}

	// Can't change currdmap before scrolling is done, because some places require currdmap to remain the same:
	//   - fade
	//   - lighting
	//   - more?
	scrolling_destdmap = new_dmap;

	// For the duration of the scrolling, the old screen/region viewport is used for all drawing operations.
	// This means that the new screens are drawn with offsets relative to the old coordinate system.
	// This is handled in for_every_nearby_screen_during_scroll.

	script_drawing_commands.Clear();
	FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);

	// Just trying to play the sound.
	if (original_destscr == -1)
		checkmaze(tmpscr, true);

	switch(DMaps[currdmap].type&dmfTYPE)
	{
		case dmDNGN:
			if(!get_qr(qr_DUNGEONS_USE_CLASSIC_CHARTING))
			{
				markBmap(scrolldir);
			}
			break;
		case dmOVERW: case dmBSOVERW:
			if(get_qr(qr_NO_OVERWORLD_MAP_CHARTING))
				break;
			[[fallthrough]];
		case dmCAVE:
			markBmap(scrolldir);
			break;
	}

	if(fixed_door)
	{
		unsetmapflag(mSECRET);
		fixed_door = false;
	}

	currmap = destmap;

	bool region_scrolling = get_region_id(currdmap, currscr) || get_region_id(currdmap, destscr);

	// Remember everything about the current region, because `loadscr` is about to reset this data.
	std::vector<mapscr*> old_temporary_screens = z3_take_temporary_screens();
	int old_origin_scr = cur_origin_screen_index;
	int old_region_scr_width = region_scr_width;
	int old_region_scr_height = region_scr_height;
	int old_region_scr_dx = region_scr_dx;
	int old_region_scr_dy = region_scr_dy;
	int old_world_w = world_w;
	int old_world_h = world_h;
	int old_playing_field_offset = playing_field_offset;
	int old_original_playing_field_offset = original_playing_field_offset;
	bool old_extended_height_mode = is_extended_height_mode();
	viewport_t old_viewport = viewport;

	loadscr(destdmap, destscr, scrolldir, overlay);

	mapscr* newscr = get_scr(destmap, destscr);
	// TODO z3 !! remove
	bool scrolling_extended_height = old_extended_height_mode || is_extended_height_mode();
	// TODO z3
	int new_playing_field_offset = playing_field_offset;
	playing_field_offset = old_original_playing_field_offset;

	// Old scrolling code maintained the previous playing field offset exactly, which only mattered
	// if during a quake. Just a couple replays show this behavior. It actually looks bad and messes up
	// the passive subscreen during the entire scroll, but for now let's not update them.
	bool freedom_in_chains_hack = false;
	if (replay_is_active() && (replay_get_meta_str("qst") == "freedom_in_chains.qst" || replay_get_meta_str("qst") == "yuurand.qst"))
	{
		freedom_in_chains_hack = true;
		playing_field_offset = old_playing_field_offset;
	}

	// TODO z3 !!!!! rm dupe
	// We must recalculate the new hero position and viewport, if a script run above just change the hero position.
	if (hero_x_before_scripts != x || hero_y_before_scripts != y)
	{
		{
			new_viewport = {0};
			new_hero_x = 0; new_hero_y = 0;
			// The above `loadscr` has loaded the destination screen's region information into these global variables.
			int new_origin_scr = cur_origin_screen_index;

			switch(scrolldir)
			{
				case up:
				{
					new_hero_x.val = (region_scr_dx*256) * 10000L + x.val%(256*10000L);
					new_hero_y = world_h - 16;
				}
				break;
				
				case down:
				{
					new_hero_x.val = (region_scr_dx*256) * 10000L + x.val%(256*10000L);
					new_hero_y = 0;
				}
				break;
				
				case left:
				{
					new_hero_x = world_w - 16;
					new_hero_y.val = (region_scr_dy*176) * 10000L + y.val%(176*10000L);
				}
				break;
				
				case right:
				{
					new_hero_x = 0;
					new_hero_y.val = (region_scr_dy*176) * 10000L + y.val%(176*10000L);
				}
				break;

				// Should never happen ...
				default:
				{
					abort();
				}
			}
			z3_calculate_viewport(new_dmap, destscr, world_w, world_h, new_hero_x, new_hero_y, new_viewport);
		}

		{
			int scroll_height = std::min(old_viewport.h, new_viewport.h);
			int scroll_width = std::min(old_viewport.w, new_viewport.w);
			int scroll_amount = scrolldir == up || scrolldir == down ? scroll_height : scroll_width;
			scroll_counter = scroll_amount / step;

			dx = 0;
			dy = 0;
			if (scrolldir == up)    dy = -1;
			if (scrolldir == down)  dy = 1;
			if (scrolldir == left)  dx = -1;
			if (scrolldir == right) dx = 1;
		}

		{
			int old_hero_screen_x = x.getInt() - old_viewport.x;
			int old_hero_screen_y = y.getInt() - old_viewport.y + old_original_playing_field_offset;
			int new_hero_screen_x = new_hero_x - new_viewport.x;
			int new_hero_screen_y = new_hero_y - new_viewport.y + original_playing_field_offset;
			if (dx)      secondary_axis_alignment_amount = new_hero_screen_y - old_hero_screen_y;
			else if (dy) secondary_axis_alignment_amount = new_hero_screen_x - old_hero_screen_x;
			else         secondary_axis_alignment_amount = 0;
		}

		FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_X] = new_viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_Y] = new_viewport.y;

		FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_X] = viewport.x;
		FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_Y] = viewport.y;

		FFCore.ScrollingData[SCROLLDATA_NPX] = new_hero_x.getInt();
		FFCore.ScrollingData[SCROLLDATA_NPY] = new_hero_y.getInt();
	}

	int sx = viewport.x + (scrolldir == left ? viewport.w : 0);
	int sy = viewport.y + (scrolldir == up ? viewport.h : 0);
	if (is_unsmooth_vertical_scrolling) sy += 3;

	// change Hero's state if entering water
	int32_t ahead = lookahead(scrolldir);
	int32_t lookaheadx = vbound(x+8,0,240); //var = vbound(val, n1, n2), not bound(var, n1, n2) -Z
	int32_t lookaheady = vbound(y + (bigHitbox?8:12),0,160);
		//bound(cx, 0, 240); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
		//bound(cy, 0, 168); //Fix crash during screen scroll when Hero is moving too quickly through a corner - DarkDragon
		//y+8 could be 168 //Attempt to fix a frash where scrolling through the lower-left corner could crassh ZC as reported by Lut. -Z
	switch(scrolldir)
	{
		case up:
			lookaheady=160;
			break;
			
		case down:
			lookaheady=0;
			break;
			
		case left:
			lookaheadx=240;
			break;
			
		case right:
			lookaheadx=0;
			break;
	}
	
	bool nowinwater = false;
	{
		if(lastaction != inwind)
		{
			if(lastaction == rafting ) //&& isRaftFlag(aheadflag))
			{
				if (lookaheadraftflag(scrolldir))
				{
					action=rafting; FFCore.setHeroAction(rafting);
					raftclk=0;
				}
			}
			else if(iswaterex_z3(ahead, -1, lookaheadx,lookaheady) && (current_item(itype_flippers)))
			{
				if(lastaction==swimming || lastaction == sideswimming || lastaction == sideswimattacking || lastaction == sideswimhit || lastaction == swimhit || lastaction == sideswimcasting || lastaction == sidewaterhold1 || lastaction == sidewaterhold2)
				{
					SetSwim();
					hopclk = 0xFF;
					nowinwater = true;
				}
				else
				{
					action=hopping; FFCore.setHeroAction(hopping);
					hopclk = 2;
					nowinwater = true;
				}
			}
			else if((lastaction == attacking || lastaction == sideswimattacking) && charging)
			{
				action = lastaction; FFCore.setHeroAction(lastaction);
			}
			else
			{
				action=none; FFCore.setHeroAction(none);
			}
		}
		
		// The naturaldark state can be read/set by an FFC script before
		// fade() or lighting() is called.
		naturaldark = ((TheMaps[currmap*MAPSCRS+heroscr].flags & fDARK) != 0);
		
		if(newscr->oceansfx != oldscr->oceansfx)	adjust_sfx(oldscr->oceansfx, 128, false);
		
		if(newscr->bosssfx != oldscr->bosssfx)	adjust_sfx(oldscr->bosssfx, 128, false);

		//Preloaded ffc scripts
		{
			// Kludge
			currdmap = new_dmap;
			ffscript_engine(true);
			currdmap = old_dmap;
		}
			
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
			scroll_counter++; //This was the easiest way to re-arrange the loop so drawing is in the middle
	}

	bool draw_dark = false;
	for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
		mapscr* base_screen = screen_handles[0].base_screen;
		draw_dark = draw_dark || (base_screen->flags&fDARK);
	});
	// TODO z3 !! remove old dark code
	// scrolling_use_new_dark_code = region_scrolling;
	scrolling_use_new_dark_code = true;

	int no_move = 0;
	int move_counter = 0;
	bool end_frames = false;

	scroll_counter *= delay;

	// 0 for change playing field offset, then scroll.
	// 1 for scroll, then change playing field offset.
	// Prefer changing the playing field offset first then scrolling...
	int pfo_mode = 0;
	// ... except for when the new region has a larger viewport than the old one AND moving down. That scenario can't change the
	// playing field offset first because it would have to show portions of the screen above the old one, which is bad.
	if (dy == 1 && sign(new_playing_field_offset - old_original_playing_field_offset) == -1)
		pfo_mode = 1;
	// ... or for the inverse.
	if (dy == -1 && sign(new_playing_field_offset - old_original_playing_field_offset) == 1)
		pfo_mode = 1;
	// Similar.
	if (dx && old_region_scr_dy == 0 && sign(new_playing_field_offset - old_original_playing_field_offset) == -1)
		pfo_mode = 1;
	int pfo_counter = abs(new_playing_field_offset - old_original_playing_field_offset);

	if (freedom_in_chains_hack)
		pfo_counter = 0;
	
	if (get_qr(qr_NOSCROLL))
	{
		secondary_axis_alignment_amount = 0;
		pfo_counter = 0;
	}

	// 0 for align, then scroll.
	// 1 for scroll, then align.
	int align_mode = 0;
	int align_counter = abs(secondary_axis_alignment_amount);
	// Align first, unless that would show screens outside the old region.
	if (align_counter)
	{
		viewport_t old_world_rect;
		old_world_rect.x = 0;
		old_world_rect.y = 0;
		old_world_rect.w = old_world_w;
		old_world_rect.h = old_world_h;

		viewport_t old_viewport_aligned = old_viewport;
		old_viewport_aligned.x -= (dy ? secondary_axis_alignment_amount : 0);
		old_viewport_aligned.y -= (dx ? secondary_axis_alignment_amount : 0);
		// The playing field offset is changed before aligning, so apply the delta in this check.
		old_viewport_aligned.y += new_playing_field_offset - old_original_playing_field_offset;
		if (old_world_rect.contains(old_viewport_aligned))
			align_mode = 0;
		else
			align_mode = 1;
	}

	viewport_t initial_viewport = old_viewport;
	// initial_viewport.h = std::max(old_viewport.h, new_viewport.h);
	viewport = initial_viewport;
	if (is_unsmooth_vertical_scrolling) viewport.y += 3;

	auto script_hero_x = x;
	auto script_hero_y = y;

	// FFCs coordinates are world positions, and so don't need an offset like when drawing a specific screen's combos in do_scrolling_layer.
	// But since their coordinates are in the new coordinate system, we must apply an offset based on the difference between the two coordinate systems.
	// Only used when drawing the new screens.
	// TODO z3 !!! this is really complex ...
	int ffc_offset_x = (-z3_get_region_relative_dx(0) + z3_get_region_relative_dx(0, scrolling_origin_scr)) * 256;
	int ffc_offset_y = (-z3_get_region_relative_dy(0) + z3_get_region_relative_dy(0, scrolling_origin_scr)) * 176;

	// These mark the top-left coordinate of the new screen and the old screen, relative to the old region world coordinates.
	int nx = 0;
	int ny = 0;
	int ox = 0;
	int oy = 0;
	for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
		int offx = draw_dx * 256;
		int offy = draw_dy * 176;
		if (scr == destscr && is_new_screen)
		{
			nx = offx;
			ny = offy;
		}
		else if (scr == scrolling_scr && !is_new_screen)
		{
			ox = offx;
			oy = offy;
		}
	});

	currdmap = new_dmap;
	for(word i = 0; (scroll_counter >= 0 && delay != 0) || align_counter || pfo_counter; i++, scroll_counter--) //Go!
	{
		if (replay_version_check(0, 3))
		{
			replay_poll();
		}

		if(Quit)
		{
			screenscrolling = false;
			return;
		}

		// if (region_scrolling)
		// {
		// 	ZScriptVersion::RunScrollingScript(-1, -1, -1, -1, false, false);
		// }
		// else
		{
			auto prev_x = x;
			auto prev_y = y;
			x = script_hero_x;
			y = script_hero_y;
			ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, false);
			x = prev_x;
			y = prev_y;
		}

		if(no_move > 0)
			no_move--;
			
		//Don't want to move things on the first or last iteration, or between delays, or while aligning, or while adjusting playing field offset
		if(i == 0 || scroll_counter == 0 || scroll_counter % delay != 0 || (align_mode == 0 && align_counter) || (pfo_mode == 0 && pfo_counter))
			no_move++;

		if(scrolldir == up || scrolldir == down)
		{
			if(!get_qr(qr_SMOOTHVERTICALSCROLLING))
			{
				//Add a few extra frames if on the second loop and cool scrolling is not set
				if(i == 1)
				{
					scroll_counter += (scrolldir == down) ? 3 : 2;
					no_move += (scrolldir == down) ? 3 : 2;
				}
			}
			else
			{
				//4 frames after we've finished scrolling of being still
				if (scroll_counter == 0 && pfo_counter == 0 && !end_frames)
				{
					scroll_counter += 4;
					no_move += 4;
					end_frames = true;
				}
			}
		}
		
		if (align_mode == 0)
		{
			if (align_counter > 0 && (pfo_counter == 0 || pfo_mode == 1))
			{
				align_counter = MAX(0, align_counter - 4);
				if (align_counter)
					scroll_counter++;
			}
		}
		else
		{
			if (align_counter > 0 && !(scroll_counter >= 0 && delay != 0)) 
			{
				align_counter = MAX(0, align_counter - 4);
				no_move = 1;
			}
		}

		bool do_pfo_adjust = false;
		if (pfo_mode == 0)
		{
			if (pfo_counter > 0)
			{
				do_pfo_adjust = true;
				if (pfo_counter)
					scroll_counter++;
			}
		}
		else
		{
			if (pfo_counter > 0 && !(scroll_counter >= 0 && delay != 0))
			{
				do_pfo_adjust = true;
				no_move = 1;
			}
		}

		if (do_pfo_adjust)
		{
			int dpfo = sign(new_playing_field_offset - old_original_playing_field_offset);
			pfo_counter = MAX(0, pfo_counter - 4);
			playing_field_offset = new_playing_field_offset - pfo_counter * dpfo;
			if (pfo_mode == 0)
				viewport.y = initial_viewport.y + step * move_counter * dy + (playing_field_offset - old_original_playing_field_offset);
			viewport.h = 232 - playing_field_offset;
		}

		if(!no_move)
		{
			switch(scrolldir)
			{
			case up:
				sy -= step;
				script_hero_y += step;
				break;
				
			case down:
				sy += step;
				script_hero_y -= step;
				break;
				
			case left:
				sx -= step;
				script_hero_x += step;
				break;
				
			case right:
				sx += step;
				script_hero_x -= step;
				break;
			}

			move_counter++;
			{
				viewport.x = initial_viewport.x + step * move_counter * dx;
				viewport.y = initial_viewport.y + step * move_counter * dy + playing_field_offset - old_original_playing_field_offset;
				// lol
				if (freedom_in_chains_hack)
					viewport.y -= playing_field_offset - old_original_playing_field_offset;


				// int vertical_amount = step * move_counter;
				// int y_delta = std::min(step * move_counter, initial_viewport.h);
				// vertical_amount -= y_delta;

				// viewport.y = initial_viewport.y + y_delta * dy;

				// Viewport height changes only towards the end of the scroll.
				// int y = step * move_counter;
				// int y_delta = std::min(new_viewport.h, y);
				// viewport.y = initial_viewport.y + y_delta * dy;
				// viewport.h = initial_viewport.h + (y - y_delta) * dy;

				// playing_field_offset = std::clamp(playing_field_offset,
				// 	new_playing_field_offset - viewport.y,
				// 	new_playing_field_offset + viewport.y);
				// playing_field_offset = playing_field_offset;

				// int vy_new_region = viewport.y - new_region_offset_y;

				// int dvh = 0;
				// if (new_playing_field_offset > old_original_playing_field_offset) dvh = 1;
				// if (new_playing_field_offset < old_original_playing_field_offset) dvh = -1;

				// int pfo_change = std::abs(new_playing_field_offset - old_original_playing_field_offset);
				// playing_field_offset = old_original_playing_field_offset + std::min(move_counter, pfo_change) * dvh;


				// playing_field_offset = std::max(0, new_playing_field_offset - (viewport.y + new_viewport.h));
				// viewport.h = 232 - playing_field_offset;







				// if (playing_field_offset < new_playing_field_offset - vy_new_region)
				// {
				// 	playing_field_offset = new_playing_field_offset - vy_new_region;
				// }

				// int dvh = 0;
				// if (new_playing_field_offset > old_original_playing_field_offset) dvh = 1;
				// if (new_playing_field_offset < old_original_playing_field_offset) dvh = -1;
				// playing_field_offset = old_original_playing_field_offset + step * move_counter * dvh;
				// playing_field_offset = std::clamp(playing_field_offset,
				// 	std::min(old_original_playing_field_offset, new_playing_field_offset),
				// 	std::max(old_original_playing_field_offset, new_playing_field_offset));

				// if (step * move_counter >= new_viewport.h)
				// {
				// 	int dvh = 0;
				// 	if (new_playing_field_offset > old_original_playing_field_offset) dvh = 1;
				// 	if (new_playing_field_offset < old_original_playing_field_offset) dvh = -1;
				// 	playing_field_offset = std::min(new_playing_field_offset - old_original_playing_field_offset, dvh * (step * move_counter - new_viewport.h));
				// }
			}
			
			//bound Hero when me move him off the screen in the last couple of frames of scrolling
			if(script_hero_y > old_viewport.y + old_viewport.h - 16) script_hero_y = old_viewport.y + old_viewport.h - 16;
			if(script_hero_y < 0)   script_hero_y = 0;
			if(script_hero_x > old_viewport.x + old_viewport.w - 16) script_hero_x = old_viewport.x + old_viewport.w - 16;
			if(script_hero_x < 0)   script_hero_x = 0;

			// This is the only thing that moves the hero.
			x.doClamp(viewport.x, viewport.x + viewport.w - 16);
			// int bounds_y = viewport.y + playing_field_offset;
			int bounds_y = viewport.y;
			y.doClamp(bounds_y, bounds_y + viewport.h - 16);

			if (is_unsmooth_vertical_scrolling) viewport.y += 3;

			// x += viewport.x;
			// y += viewport.y;

			// replay_step_comment(fmt::format("BOUND hero scroll x y {} {}", x.getInt(), y.getInt()));
			
			if(ladderx > 0 || laddery > 0)
			{
				// If the ladder moves on both axes, the player can
				// gradually shift it by going back and forth
				if(scrolldir==up || scrolldir==down)
					laddery = y.getInt();
				else
					ladderx = x.getInt();
			}
		} else {
			// replay_step_comment(fmt::format("no change hero scroll x y {} {}", x.getInt(), y.getInt()));
		}

		// if(ladderx > 0 || laddery > 0)
		// {
		// 	// If the ladder moves on both axes, the player can
		// 	// gradually shift it by going back and forth
		// 	if(scrolldir==up || scrolldir==down)
		// 		laddery = y.getInt();
		// 	else
		// 		ladderx = x.getInt();
		// }

		// if(ladderx > 0 || laddery > 0)
		// {
		// 	// If the ladder moves on both axes, the player can
		// 	// gradually shift it by going back and forth
		// 	if(scrolldir==up || scrolldir==down)
		// 		laddery = y.getInt();
		// 	else
		// 		ladderx = x.getInt();
		// }

		// if(!no_move)
		// {
		// 	switch(scrolldir)
		// 	{
		// 	case up:
		// 		sy -= step;
		// 		// viewport.y -= step;
		// 		// y += step;
		// 		break;
				
		// 	case down:
		// 		sy += step;
		// 		// viewport.y += step;
		// 		// y -= step;
		// 		break;
				
		// 	case left:
		// 		sx -= step;
		// 		// viewport.x -= step;
		// 		// x += step;
		// 		break;
				
		// 	case right:
		// 		sx += step;
		// 		// viewport.x += step;
		// 		// x -= step;
		// 		break;
		// 	}

		// 	move_counter++;
		// 	// script_sx -= step * dx;
		// 	// script_sy -= step * dy;
		// }

		// viewport.x = old_viewport.x + step * move_counter * dx;
		// viewport.y = old_viewport.y + step * move_counter * dy;
		// if (is_unsmooth_vertical_scrolling) viewport.y += 3;
		// if(!no_move) move_counter++;

		

		// int sx = step * move_counter * dx;
		// int sy = step * move_counter * dy;

		// viewport.x += sx;
		// viewport.y += sy;
		// viewport.x = old_viewport.x + step * move_counter * dx;
		// viewport.y = old_viewport.y + step * move_counter * dy;

		// bound Hero to screen edge, needed for the last couple of frames of scrolling.
		// Note: this is the only thing that actual moves the hero. Everything else is just moving the viewport.
		// if (dx) x = vbound(x, viewport.x, viewport.x + viewport.w - 16);
		// if (is_unsmooth_vertical_scrolling) viewport.y += 3;
		// if (dy) y = vbound(y, viewport.y, viewport.y + viewport.h - 16);
		// if (is_unsmooth_vertical_scrolling) viewport.y -= 3;

		// auto hero_draw_x = x;
		// auto hero_draw_y = y;
		// x += 256*dx;
		// y += 176*dy;

		// replay_step_comment(fmt::format("hero scroll x y {} {} no_move {}", x.getInt(), y.getInt(), no_move));

		// if (is_unsmooth_vertical_scrolling) viewport.y -= 3;
		// if (is_unsmooth_vertical_scrolling) sy += 3;

		// if (ladderx > 0 || laddery > 0)
		// {
		// 	// If the ladder moves on both axes, the player can
		// 	// gradually shift it by going back and forth
		// 	if(scrolldir==up || scrolldir==down)
		// 		laddery = y.getInt();
		// 	else
		// 		ladderx = x.getInt();
		// }

		if (secondary_axis_alignment_amount)
		{
			int delta = (align_counter - abs(secondary_axis_alignment_amount)) * sign(secondary_axis_alignment_amount);
			if (scrolldir == up || scrolldir == down) viewport.x = initial_viewport.x + delta;
			else                                      viewport.y = initial_viewport.y + delta;
			// TODO z3 ! lets set viewport to initial viewport at top of each loop ...
			//if (pfo_mode == 0)
			if (dx)
				viewport.y += playing_field_offset - old_original_playing_field_offset;
		}

		// if (pfo_mode == 0)
		// 	viewport.y = initial_viewport.y + step * move_counter * dy + (playing_field_offset - old_original_playing_field_offset);

		FFCore.ScrollingData[SCROLLDATA_NX] = nx - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NY] = ny - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_OX] = ox - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_OY] = oy - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_NRX] = new_region_offset_x - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NRY] = new_region_offset_y - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_ORX] = -viewport.x;
		FFCore.ScrollingData[SCROLLDATA_ORY] = -viewport.y;

		

		//FFScript.OnWaitdraw()
		// if (region_scrolling)
		// {
		// 	ZScriptVersion::RunScrollingScript(-1, -1, -1, -1, false, true); //Waitdraw
		// }
		// else
		{
			auto prev_x = x;
			auto prev_y = y;
			x = script_hero_x;
			y = script_hero_y;
			ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, true); //Waitdraw
			x = prev_x;
			y = prev_y;
		}
		
		FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);
		// TODO z3 draw straight to framebuf?
		clear_bitmap(scrollbuf);
		clear_bitmap(framebuf);
		clear_a5_bmp(rti_infolayer.bitmap);

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			int offx = draw_dx * 256;
			int offy = draw_dy * 176 - playing_field_offset;

			mapscr* base_screen = screen_handles[0].base_screen;
			if(XOR(base_screen->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG)) do_layer(scrollbuf, 0, screen_handles[2], offx, offy);
			if(XOR(base_screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)) do_layer(scrollbuf, 0, screen_handles[3], offx, offy);
		});

		// Draw screens' background layer primitives together, after their layers' combos.
		// Not ideal, but probably good enough for all realistic purposes.
		// Note: Not drawing for every screen because the old scrolling code only did this for the new screen...
		// TODO z3
		if(XOR((newscr->flags7&fLAYER2BG) || (oldscr->flags7&fLAYER2BG), DMaps[currdmap].flags&dmfLAYER2BG)) do_primitives(scrollbuf, 2, newscr, 0, 0);
		if(XOR((newscr->flags7&fLAYER3BG) || (oldscr->flags7&fLAYER3BG), DMaps[currdmap].flags&dmfLAYER3BG)) do_primitives(scrollbuf, 3, newscr, 0, 0);

		combotile_add_x = 0;
		combotile_add_y = playing_field_offset;
		if (is_unsmooth_vertical_scrolling) combotile_add_y -= 3;
		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			int offx = draw_dx * 256;
			int offy = draw_dy * 176;
			if (lenscheck(screen_handles[0].screen, 0))
				putscr(scrollbuf, offx, offy, screen_handles[0].screen);
		});
		combotile_add_x = 0;
		combotile_add_y = 0;

		// Minus 8 because half of the bottom row is not visible.
		// TODO z3 ?
		int mapscr_view_y = playing_field_offset;
		int mapscr_view_height = viewport.h - 8;
		blit(scrollbuf, framebuf, 0, 0, 0, mapscr_view_y, viewport.w, mapscr_view_height);
		if (lenscheck(newscr, 0))
			do_primitives(framebuf, 0, newscr, 0, playing_field_offset);

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			int offx = draw_dx * 256;
			int offy = draw_dy * 176;
			bool is_new_scr = scr == currscr;

			mapscr* base_screen = screen_handles[0].base_screen;
			bool primitives = is_new_scr;
			do_layer(framebuf, 0, screen_handles[1], offx, offy, primitives);

			if (get_qr(qr_FFCSCROLL))
			{
				int draw_ffc_x = is_new_screen ? ffc_offset_x : 0;
				int draw_ffc_y = is_new_screen ? ffc_offset_y : 0;
				do_layer(framebuf, -3, screen_handles[0], draw_ffc_x, draw_ffc_y); // ffcs
			}

			if(!(XOR(base_screen->flags7&fLAYER2BG, DMaps[currdmap].flags&dmfLAYER2BG)))
			{
				primitives &= !(oldscr->flags7&fLAYER2BG);
				do_layer(framebuf, 0, screen_handles[2], offx, offy, primitives);
			}
		});

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			int offx = draw_dx * 256;
			int offy = draw_dy * 176;
			bool is_new_scr = scr == currscr;
			mapscr* base_screen = screen_handles[0].base_screen;

			if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
			{
				do_layer(framebuf, -2, screen_handles[0], offx, offy);
				if (get_qr(qr_PUSHBLOCK_LAYER_1_2))
				{
					do_layer(framebuf, -2, screen_handles[1], offx, offy);
					do_layer(framebuf, -2, screen_handles[2], offx, offy);
				}
				// TODO z3 ?
				do_primitives(framebuf, SPLAYER_PUSHBLOCK, screen_handles[0].screen, 0, playing_field_offset);
			}

			int tempscreen = is_new_scr ? 2 : 3;
			do_walkflags(base_screen, offx, offy, tempscreen); // show walkflags if the cheat is on
			do_effectflags(base_screen, offx, offy, tempscreen); // show effectflags if the cheat is on
		});

		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			int offx = draw_dx * 256;
			int offy = draw_dy * 176 + playing_field_offset;
			putscrdoors(framebuf, offx, offy, screen_handles[0].screen);
		});

		// TODO z3 could everything happen in a single call to for_every_nearby_screen_during_scroll ?

		if (!align_counter || scroll_counter) herostep();
		
		{
			// Must draw with old-region coordinates.
			auto prev_y = y;
			auto prev_yofs = yofs;
			auto prev_pfos = playing_field_offset;

			// x = viewport.x;
			// y = viewport.y;

			// y += playing_field_offset - new_playing_field_offset;
			// playing_field_offset += old_original_playing_field_offset;
			if (is_unsmooth_vertical_scrolling) y += 3;
			// yofs = playing_field_offset - new_playing_field_offset;
			if (!freedom_in_chains_hack)
				yofs = playing_field_offset;

			if((z > 0 || fakez > 0) && (!get_qr(qr_SHADOWSFLICKER) || frame&1))
			{
				drawshadow(framebuf, get_qr(qr_TRANSSHADOWS) != 0);
			}

			if(!isdungeon() || get_qr(qr_FREEFORM))
			{
				draw_under(framebuf); //draw the ladder or raft
				decorations.draw2(framebuf, true);
				draw(framebuf); //Hero
				decorations.draw(framebuf,  true);
			}

			y = prev_y;
			yofs = prev_yofs;
			playing_field_offset = prev_pfos;
		}
		
		for_every_nearby_screen_during_scroll(old_temporary_screens, [&](std::array<screen_handle_t, 7> screen_handles, int scr, int draw_dx, int draw_dy, bool is_new_screen) {
			bool is_dest_scr = scr == destscr;
			int offx = draw_dx * 256;
			int offy = draw_dy * 176;

			mapscr* base_screen = screen_handles[0].base_screen;
			if(!(XOR(base_screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG)))
			{
				bool primitives = is_dest_scr && !(XOR(base_screen->flags7&fLAYER3BG, DMaps[currdmap].flags&dmfLAYER3BG));
				do_layer(framebuf, 0, screen_handles[3], offx, offy, primitives);
			}
			
			do_layer(framebuf, 0, screen_handles[4], offx, offy, is_new_screen); //layer 4
			do_layer(framebuf, -1, screen_handles[0], offx, offy); //overhead combos
			if(get_qr(qr_OVERHEAD_COMBOS_L1_L2))
			{
				do_layer(framebuf, -1, screen_handles[1], offx, offy); //overhead combos
				do_layer(framebuf, -1, screen_handles[2], offx, offy); //overhead combos
			}
			do_layer(framebuf, 0, screen_handles[5], offx, offy, is_dest_scr); //layer 5
			{
				int draw_ffc_x = is_new_screen ? ffc_offset_x : 0;
				int draw_ffc_y = is_new_screen ? ffc_offset_y : 0;
				do_layer(framebuf, -4, screen_handles[0], draw_ffc_x, draw_ffc_y); //overhead FFCs
			}
			do_layer(framebuf, 0, screen_handles[6], offx, offy, is_dest_scr); //layer 6
		});
		
		// pretty sure this doesn't do anything.
		// if(msg_bg_display_buf->clip == 0)
		// {
		// 	blit_msgstr_bg(framebuf, tx2, ty2, 0, playing_field_offset, 256, 168);
		// }
		// if(msg_portrait_display_buf->clip == 0)
		// {
		// 	blit_msgstr_prt(framebuf, tx2, ty2, 0, playing_field_offset, 256, 168);
		// }
		// if(msg_txt_display_buf->clip == 0)
		// {
		// 	blit_msgstr_fg(framebuf, tx2, ty2, 0, playing_field_offset, 256, 168);
		// }

		if (draw_dark && get_qr(qr_NEW_DARKROOM) && get_qr(qr_NEWDARK_L6))
		{
			scrollscr_handle_dark(newscr, oldscr, old_temporary_screens);
		}

		put_passive_subscr(framebuf, 0, passive_subscreen_offset, game->should_show_time(), sspUP);

		// switch(scrolldir)
		// {
		// case up:
		// 	primitive_offy = 176;
		// 	break;
			
		// case down:
		// 	primitive_offy = 0;
		// 	break;
			
		// case left:
		// 	primitive_offx = -nx;
		// 	// primitive_offx = 256;
		// 	// primitive_offy = new_viewport.h - old_viewport.h;
		// 	break;
			
		// case right:
		// 	primitive_offx = -nx;
		// 	// primitive_offx = -old_world_w;
		// 	// primitive_offy = new_viewport.h - old_viewport.h;
		// 	break;
		// }

		if(get_qr(qr_SUBSCREENOVERSPRITES))
			do_primitives(framebuf, 7, newscr, 0, playing_field_offset);
		
		if (draw_dark && get_qr(qr_NEW_DARKROOM) && !get_qr(qr_NEWDARK_L6))
		{
			scrollscr_handle_dark(newscr, oldscr, old_temporary_screens);
		}

		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
		
		//end drawing
		FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);
		advanceframe(true/*,true,false*/);
		if (scroll_counter > 0 || get_qr(qr_FIXSCRIPTSDURINGSCROLLING))
			script_drawing_commands.Clear();
		FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);
		actiontype lastaction = action;
		action=scrolling; FFCore.setHeroAction(scrolling);
		FFCore.runF6Engine();
		action=lastaction; FFCore.setHeroAction(lastaction);
	}//end main scrolling loop (2 spaces tab width makes me sad =( )
	currdmap = old_dmap;

	// TODO z3 old scrolling code doesn't clear darkscr_bmp_curscr at end of scroll, so first frame will have some lighting from
	// previous screen... game_loop clears these bitmaps but that should be moved to draw_screen.
	if (draw_dark && scrolling_use_new_dark_code)
	{
		// blit(darkscr_bmp_z3, darkscr_bmp_curscr, 0, playing_field_offset, 0, 0, darkscr_bmp_curscr->w, darkscr_bmp_curscr->h);
		// blit(darkscr_bmp_z3_trans, darkscr_bmp_curscr_trans, 0, playing_field_offset, 0, 0, darkscr_bmp_curscr_trans->w, darkscr_bmp_curscr_trans->h);
	}

	clear_bitmap(msg_txt_display_buf);
	set_clip_state(msg_txt_display_buf, 1);
	clear_bitmap(msg_bg_display_buf);
	set_clip_state(msg_bg_display_buf, 1);
	clear_bitmap(msg_portrait_display_buf);
	set_clip_state(msg_portrait_display_buf, 1);

	new_region_offset_x = 0;
	new_region_offset_y = 0;
	viewport = new_viewport;
	if (!freedom_in_chains_hack)
		playing_field_offset = new_playing_field_offset;
	x = new_hero_x;
	y = new_hero_y;
	if (!freedom_in_chains_hack)
		yofs = playing_field_offset;
	if(ladderx > 0 || laddery > 0)
	{
		// If the ladder moves on both axes, the player can
		// gradually shift it by going back and forth
		if(scrolldir==up || scrolldir==down)
			laddery = y.getInt();
		else
			ladderx = x.getInt();
	}

	// TODO z3 ! rm. really should separate quake draw offset from playing field offset.
	if (scrolling_extended_height)
		playing_field_offset = is_extended_height_mode() ? 0 : 56;
	else if (!freedom_in_chains_hack)
		playing_field_offset = old_original_playing_field_offset;

	//Move hero to the other side of the screen if scrolling's not turned on
	if(get_qr(qr_NOSCROLL))
	{
		switch(scrolldir)
		{
		case up:
			y = world_h - 16;
			break;
			
		case down:
			y = 0;
			break;
			
		case left:
			x = world_w - 16;
			break;
			
		case right:
			x = 0;
			break;
		}
	}
	
	if((z > 0 || fakez > 0) && isSideViewHero())
	{
		y -= z;
		y -= fakez;
		z = 0;
		fakez = 0;
	}
	
	set_respawn_point(false);
	trySideviewLadder();
	warpx   = -1;
	warpy   = -1;
	
	screenscrolling = false;
	scrolling_destdmap = -1;
	memset(FFCore.ScrollingData, 0, sizeof(int32_t) * SZ_SCROLLDATA);
	FFCore.ScrollingData[SCROLLDATA_DIR] = -1;
	
	if (destdmap != -1)
	{
		if(currdmap != destdmap)
			timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
		if(dlevel != DMaps[destdmap].level)
			timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
		currdmap = destdmap;
		dlevel = DMaps[destdmap].level;
	}
		
	//if Hero is going from non-water to water, and we set his animation to "hopping" above, we must now
	//change it to swimming - since we have manually moved Hero onto the first tile, the hopping code
	//will get confused and try to hop Hero onto the next (possibly nonexistant) water tile in his current
	//direction. -DD
	
	if(nowinwater)
	{
		SetSwim();
		hopclk = 0xFF;
	}
	
	// NES behaviour: Fade to light after scrolling
	lighting(false, false); // No, we don't need to set naturaldark...
	
	// homescr=currscr;
	init_dmap();
	putscr(scrollbuf,0,0,newscr); // TODO z3 rm?
	putscrdoors(scrollbuf,0,0,newscr); // TODO z3 rm?
	
	// Check for raft flags
	if((get_qr(qr_BROKEN_RAFT_SCROLL) || lastaction == rafting)
		&& action!=rafting && hopclk==0 && !toogam)
	{
		if(MAPFLAG(x,y)==mfRAFT||MAPCOMBOFLAG(x,y)==mfRAFT)
		{
			sfx(hero_screen->secretsfx);
			action=rafting; FFCore.setHeroAction(rafting);
			raftclk=0;
		}
		
		// Half a tile off?
		else if((dir==left || dir==right) && (MAPFLAG(x,y+8)==mfRAFT||MAPCOMBOFLAG(x,y+8)==mfRAFT))
		{
			sfx(hero_screen->secretsfx);
			action=rafting; FFCore.setHeroAction(rafting);
			raftclk=0;
		}
	}
	
	opendoors=0;
	markBmap(-1);
	
	if(isdungeon())
	{
		switch(hero_screen->door[scrolldir^1])
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
				
			putdoor(scrollbuf,0,scrolldir^1,hero_screen->door[scrolldir^1]);
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
		action=none; FFCore.setHeroAction(none);
	}
		
	if(action != attacking && action != sideswimattacking)
	{
		charging = 0;
		tapping = false;
	}
	
	map_bkgsfx(true);
	
	if(newscr->flags2&fSECRET)
	{
		sfx(newscr->secretsfx);
	}
	
	if (updatemusic)
	{
		playLevelMusic();
		if (musicrevert)
			FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
	}
	
	newscr_clk = frame;
	activated_timed_warp=false;
	loadside = scrolldir^1;
	FFCore.clear_combo_scripts();
	eventlog_mapflags();
	decorations.animate(); //continue to animate tall grass during scrolling
	if(get_qr(qr_FIXSCRIPTSDURINGSCROLLING))
	{
		if(old_dmap == new_dmap || (replay_version_check(0, 15)))
			ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, false); //Prewaitdraw
		else refresh_dmap_scrollscript = true;
	}

	// Bye!
	for (int i = 0; i < 136*7; i++)
	{
		if (old_temporary_screens[i])
		{
			free(old_temporary_screens[i]);
			old_temporary_screens[i] = NULL;
		}
	}

	if(!get_qr(qr_SCROLLWARP_NO_RESET_FRAME))
		GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
}

// How much to reduce Hero's damage, taking into account various rings.
int32_t HeroClass::ringpower(int32_t dmg, bool noPeril, bool noRing)
{
	if(dmg < 0) return dmg; //Don't reduce healing
	if ( get_qr(qr_BROKEN_RING_POWER) )
	{
		int32_t divisor = 1;
		float percentage = 1;
		int32_t itemid = current_item_id(itype_ring);
		bool usering = false;
    
		if(itemid>-1 && !noRing)  // current_item_id checks magic cost for rings
		{
			usering = true;
			paymagiccost(itemid);
			if(itemsbuf[itemid].flags & ITEM_FLAG2)//"Divisor is Percentage Multiplier" flag
			{
				percentage *= itemsbuf[itemid].power/100.0;
			}
			else
			{
				divisor *= itemsbuf[itemid].power;
			}
		}
	    
		/* Now for the Peril Ring */
		itemid = current_item_id(itype_perilring);
    
		if(itemid>-1 && !noPeril && game->get_life()<=itemsbuf[itemid].misc1*game->get_hp_per_heart() && checkmagiccost(itemid) && checkbunny(itemid))
		{
			usering = true;
			paymagiccost(itemid);
			if(itemsbuf[itemid].flags & ITEM_FLAG2)//"Divisor is Percentage Multiplier" flag
			{
				percentage *= itemsbuf[itemid].power/100.0;
			}
			else
			{
				divisor *= itemsbuf[itemid].power;
			}
		}
	
		// Ring divisor of 0 = no damage. -L
		if(usering && (divisor==0 || percentage==0)) //Change dto allow negative power rings. -Z
		return 0;
	
		if( percentage < 0 ) percentage = (percentage * -1) + 1; //Negative percentage = that percent MORE damage -V
	
		if ( divisor < 0 ) return dmg * percentage * (divisor*-1);
		return dmg*percentage/( divisor != 0 ? divisor : 1 ); //zc_max(divisor, 1); // well, better safe...
		
	}
	else
	{
		double divisor = 1;
		double percentage = 1;
		int32_t itemid = current_item_id(itype_ring);
		bool usering = false;
		    
		if(itemid>-1 && !noRing)  // current_item_id checks magic cost for rings
		{
			usering = true;
			paymagiccost(itemid);
			if(itemsbuf[itemid].flags & ITEM_FLAG2)//"Divisor is Percentage Multiplier" flag
			{
				double perc = itemsbuf[itemid].power/100.0;
				if(perc < 0) perc = -perc + 1; //Negative percentage = that percent MORE damage -V
				percentage *= perc;
			}
			else
			{
				if(itemsbuf[itemid].power < 0)
					divisor /= -(itemsbuf[itemid].power);
				else divisor *= itemsbuf[itemid].power;
			}
		}
	    
		/* Now for the Peril Ring */
		itemid = current_item_id(itype_perilring);
	    
		if(itemid>-1 && !noPeril && game->get_life()<=itemsbuf[itemid].misc1*game->get_hp_per_heart() && checkmagiccost(itemid) && checkbunny(itemid))
		{
			usering = true;
			paymagiccost(itemid);
			if(itemsbuf[itemid].flags & ITEM_FLAG2)//"Divisor is Percentage Multiplier" flag
			{
				double perc = itemsbuf[itemid].power/100.0;
				if(perc < 0) perc = -perc + 1; //Negative percentage = that percent MORE damage -V
				percentage *= perc;
			}
			else
			{
				if(itemsbuf[itemid].power < 0)
					divisor /= -(itemsbuf[itemid].power);
				else divisor *= itemsbuf[itemid].power;
			}
		}
	    
		// Ring divisor of 0 = no damage. -L
		if(usering && (divisor==0 || percentage==0)) //Change dto allow negative power rings. -Z
			return 0;
		
		//if ( divisor < 0 ) return dmg * percentage * (divisor*-1); //handle this further up now
		return dmg*percentage/( divisor != 0 ? divisor : 1 ); //zc_max(divisor, 1); // well, better safe...
	}
}

// Should swinging the hammer make the 'pound' sound?
// Or is Hero just hitting air?
bool HeroClass::sideviewhammerpound()
{
    int32_t wx=0,wy=0;
    
    switch(dir)
    {
    case up:
        wx=-1;
        wy=-15;
        
        if(isSideViewHero())  wy+=8;
        
        break;
        
    case down:
        wx=8;
        wy=28;
        
        if(isSideViewHero())  wy-=8;
        
        break;
        
    case left:
        wx=-8;
        wy=14;
        
        if(isSideViewHero()) wy+=8;
        
        break;
        
    case right:
        wx=21;
        wy=14;
        
        if(isSideViewHero()) wy+=8;
        
        break;
    }
    
    if(!isSideViewHero())
    {
        return (COMBOTYPE(x+wx,y+wy)!=cSHALLOWWATER && !iswaterex_z3(MAPCOMBO(x+wx,y+wy), -1, x+wx,y+wy));
    }
    
    if(_walkflag(x+wx,y+wy,0,SWITCHBLOCK_STATE)) return true;
    
    if(dir==left || dir==right)
    {
        wx+=16;
        
        if(_walkflag(x+wx,y+wy,0,SWITCHBLOCK_STATE)) return true;
    }
    
    return false;
}

/************************************/
/********  More Items Code  *********/
/************************************/

// The following are only used for Hero damage. Damage is in quarter hearts.
int32_t enemy_dp(int32_t index)
{
    return (((enemy*)guys.spr(index))->dp)*(game->get_ene_dmgmult());
}

int32_t ewpn_dp(int32_t index)
{
    return (((weapon*)Ewpns.spr(index))->power)*(game->get_ene_dmgmult());
}

int32_t lwpn_dp(int32_t index)
{
    return (((weapon*)Lwpns.spr(index))->power)*(game->get_ene_dmgmult());
}

bool checkbunny(int32_t itemid)
{
	return !Hero.BunnyClock() || (itemid > 0 && itemsbuf[itemid].flags&ITEM_BUNNY_ENABLED);
}

bool usesSwordJinx(int32_t itemid)
{
	itemdata const& it = itemsbuf[itemid];
	bool ret = (it.family==itype_sword);
	if(it.flags & ITEM_FLIP_JINX) return !ret;
	return ret;
}
bool checkitem_jinx(int32_t itemid)
{
	itemdata const& it = itemsbuf[itemid];
	if(it.flags & ITEM_JINX_IMMUNE) return true;
	if(usesSwordJinx(itemid)) return HeroSwordClk() == 0;
	return HeroItemClk() == 0;
}

void stopCaneOfByrna()
{
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon *w = ((weapon*)Lwpns.spr(i));
		if(w->id==wCByrna)
			w->dead=1;
	}
}

/* Crashes if used by ffscript.cpp, in case LINKITEMD
void stopCaneOfByrna()
{
	byte prnt_cane = -1; 
	weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(wCByrna)));
        prnt_cane = ew->parentitem;
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		weapon *w = ((weapon*)Lwpns.spr(i));
        
		if(w->id==wCByrna)
		{
			w->dead=1;
		}
	}
	if ( prnt_cane > -1 )
	{
		stop_sfx(itemsbuf[prnt_cane].usesound);
	}
}
*/
//Check if there are no beams, kill sfx, and reset last_cane_of_byrna_item_id
void HeroClass::cleanupByrna()
{
	if ( last_cane_of_byrna_item_id > -1 )
	{
		//al_trace("Last cane id is: %d\n", last_cane_of_byrna_item_id);
		if ( !(Lwpns.idCount(wCByrna)) )
		{
			stop_sfx(itemsbuf[last_cane_of_byrna_item_id].usesound);
			last_cane_of_byrna_item_id = -1; 
		}
	}
}

// Used to find out if an item family is attached to one of the buttons currently pressed.
bool isWpnPressed(int32_t itype)
{
    if((itype==getItemFamily(itemsbuf,Bwpn)) && DrunkcBbtn()) return true;
    if((itype==getItemFamily(itemsbuf,Awpn)) && DrunkcAbtn()) return true;
    if((itype==getItemFamily(itemsbuf,Xwpn)) && DrunkcEx1btn()) return true;
    if((itype==getItemFamily(itemsbuf,Ywpn)) && DrunkcEx2btn()) return true;
    return false;
}

int32_t getWpnPressed(int32_t itype)
{
    if((itype==getItemFamily(itemsbuf,Bwpn)) && DrunkcBbtn()) return Bwpn&0xFFF;
    if((itype==getItemFamily(itemsbuf,Awpn)) && DrunkcAbtn()) return Awpn&0xFFF;
    if((itype==getItemFamily(itemsbuf,Xwpn)) && DrunkcEx1btn()) return Xwpn&0xFFF;
    if((itype==getItemFamily(itemsbuf,Ywpn)) && DrunkcEx2btn()) return Ywpn&0xFFF;
    
    return -1;
}

int32_t getRocsPressed()
{
	int32_t jumpid = current_item_id(itype_rocs,true,true);
	
	if(unsigned(jumpid) >= MAXITEMS) jumpid = -1;
	
	if (jumpid != -1)
	{
		itemdata const& itm = itemsbuf[jumpid];
		
		byte intbtn = byte(itm.misc2&0xFF);
		if(getIntBtnInput(intbtn, false, true, false, false, true))
			return jumpid; //not pressed
	}

	if((itype_rocs==getItemFamily(itemsbuf,Bwpn)) && DrunkcBbtn())
		return Bwpn;
	if((itype_rocs==getItemFamily(itemsbuf,Awpn)) && DrunkcAbtn())
		return Awpn;
	if((itype_rocs==getItemFamily(itemsbuf,Xwpn)) && DrunkcEx1btn())
		return Xwpn;
	if((itype_rocs==getItemFamily(itemsbuf,Ywpn)) && DrunkcEx2btn())
		return Ywpn;

	return -1;
}

bool isItmPressed(int32_t itmid)
{
    if(itmid==(NEG_OR_MASK(Bwpn,0xFFF)) && DrunkcBbtn()) return true;
    if(itmid==(NEG_OR_MASK(Awpn,0xFFF)) && DrunkcAbtn()) return true;
    if(itmid==(NEG_OR_MASK(Xwpn,0xFFF)) && DrunkcEx1btn()) return true;
    if(itmid==(NEG_OR_MASK(Ywpn,0xFFF)) && DrunkcEx2btn()) return true;
    return false;
}

//helper function
static void selectNextBTNWpn(int32_t type, word& wpos, int32_t& BTNwpn,
	int32_t& directItemBTN, word f1 = 255, word f2 = 255, word f3 = 255)
{
	if(!new_subscreen_active)
		return;
	byte pgn = wpos&0xFF, pos = wpos>>8;
	bool empty = pgn==255;
	if(empty && get_qr(qr_NO_BUTTON_VERIFY)) return; //intentional nothingness
	SubscrPage* pg = new_subscreen_active->get_page(pgn==255?new_subscreen_active->curpage:pgn);
	if(!pg)
		return;
	auto fp1 = ((f1&0xFF)==255) ? 255 : ((empty || (f1&0xFF)==(wpos&0xFF)) ? f1 : 255);
	auto fp2 = ((f2&0xFF)==255) ? 255 : ((empty || (f2&0xFF)==(wpos&0xFF)) ? f2 : 255);
	auto fp3 = ((f3&0xFF)==255) ? 255 : ((empty || (f3&0xFF)==(wpos&0xFF)) ? f3 : 255);
	auto ret = pg->movepos_legacy(type, wpos, fp1, fp2, fp3);
	if((ret&0xFF) == 0xFF)
		BTNwpn = -1;
	else BTNwpn = pg->get_item_pos(ret>>8);
	directItemBTN = NEG_OR_MASK(BTNwpn,0xFFF);
	wpos = ret;
}
void selectNextAWpn(int32_t type)
{
    if(!get_qr(qr_SELECTAWPN))
        return;
	selectNextBTNWpn(type, game->awpn, Awpn, directItemA, COND_BWPN, COND_XWPN, COND_YWPN);
}

void selectNextBWpn(int32_t type)
{
	if(!new_subscreen_active)
		return;
	selectNextBTNWpn(type, game->bwpn, Bwpn, directItemB, COND_AWPN, COND_XWPN, COND_YWPN);
}

void selectNextXWpn(int32_t type)
{
	if(!get_qr(qr_SET_XBUTTON_ITEMS)) return;
	if(!new_subscreen_active)
		return;
	selectNextBTNWpn(type, game->xwpn, Xwpn, directItemX, COND_BWPN, COND_AWPN, COND_YWPN);
}

void selectNextYWpn(int32_t type)
{
	if(!get_qr(qr_SET_YBUTTON_ITEMS)) return;
	if(!new_subscreen_active)
		return;
	selectNextBTNWpn(type, game->ywpn, Ywpn, directItemY, COND_BWPN, COND_XWPN, COND_AWPN);
}

//helper function
static void verifyWpn(word& wpos, int32_t& BTNwpn, int32_t& directItemBTN, word f1 = 255, word f2 = 255, word f3 = 255)
{
	if(!new_subscreen_active)
		return;
	byte pgn = wpos&0xFF, pos = wpos>>8;
	bool empty = pgn==255;
	if(empty && get_qr(qr_NO_BUTTON_VERIFY)) return; //intentional nothingness
	SubscrPage* pg = new_subscreen_active->get_page(pgn==255?new_subscreen_active->curpage:pgn);
	if(!pg)
		return;
	auto fp1 = ((f1&0xFF)==255) ? 255 : ((empty || (f1&0xFF)==(wpos&0xFF)) ? f1 : 255);
	auto fp2 = ((f2&0xFF)==255) ? 255 : ((empty || (f2&0xFF)==(wpos&0xFF)) ? f2 : 255);
	auto fp3 = ((f3&0xFF)==255) ? 255 : ((empty || (f3&0xFF)==(wpos&0xFF)) ? f3 : 255);
	wpos = pg->movepos_legacy(SEL_VERIFY_RIGHT, wpos, fp1, fp2, fp3);
	if((wpos&0xFF) == 0xFF)
		BTNwpn = -1;
	else BTNwpn = pg->get_item_pos(wpos>>8);
	directItemBTN = NEG_OR_MASK(BTNwpn,0xFFF);
}

void verifyAWpn()
{
	if (game->forced_awpn != -1)
		return;
    if(!get_qr(qr_SELECTAWPN))
    {
        Awpn = selectSword();
        game->awpn = 255;
    }
    else
    {
		verifyWpn(game->awpn, Awpn, directItemA, COND_BWPN, COND_XWPN, COND_YWPN);
    }
}

void verifyBWpn()
{
	if (game->forced_bwpn != -1)
		return;
	verifyWpn(game->bwpn, Bwpn, directItemB, COND_AWPN, COND_XWPN, COND_YWPN);
}

void verifyXWpn()
{
	if (game->forced_xwpn != -1)
		return;
	if(!get_qr(qr_SET_XBUTTON_ITEMS))
		return;
	verifyWpn(game->xwpn, Xwpn, directItemX, COND_BWPN, COND_AWPN, COND_YWPN);
}

void verifyYWpn()
{
	if (game->forced_ywpn != -1)
		return;
	if(!get_qr(qr_SET_YBUTTON_ITEMS))
		return;
	verifyWpn(game->ywpn, Ywpn, directItemY, COND_BWPN, COND_XWPN, COND_AWPN);
}

void verifyBothWeapons()
{
    verifyAWpn();
    verifyBWpn();
    verifyXWpn();
    verifyYWpn();
}

// Select the sword for the A button if the 'select A button weapon' quest rule isn't set.
int32_t selectSword()
{
	return current_item_id(itype_sword);
}

// Adding code here for allowing hardcoding a button to a specific itemclass.
int32_t selectItemclass(int32_t itemclass)
{
    int32_t ret = current_item_id(itemclass);
    
    if(ret == -1)
        ret = 0;
        
    return ret;
}

// Used for the 'Pickup Hearts' item pickup condition.
bool canget(int32_t id)
{
    return id>=0 && (game->get_maxlife()>=(itemsbuf[id].pickup_hearts*game->get_hp_per_heart()));
}

void dospecialmoney(int32_t index)
{
	mapscr& scr = currscr >= 128 ? special_warp_return_screen : *tmpscr;
    int32_t priceindex = ((item*)items.spr(index))->PriceIndex;
    
    switch(scr.room)
    {
    case rINFO:                                             // pay for info
        if(prices[priceindex]!=100000 ) // 100000 is a placeholder price for free items
        {
            
                
            if(!current_item_power(itype_wallet))
	    {
		if (game->get_spendable_rupies() < abs(prices[priceindex])) 
			return;
		int32_t tmpprice = -abs(prices[priceindex]);
		int32_t total = game->get_drupy()-tmpprice;
		total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
		game->set_drupy(game->get_drupy()-total);
		//game->change_drupy(-abs(prices[priceindex]));
	    }
	    if ( current_item_power(itype_wallet)>0 )
	    {
		 game->change_drupy(0);   
	    }
        }
        rectfill(msg_bg_display_buf, 0, 0, msg_bg_display_buf->w, 80, 0);
        rectfill(msg_txt_display_buf, 0, 0, msg_txt_display_buf->w, 80, 0);
        donewmsg(QMisc.info[scr.catchall].str[priceindex]);
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        items.del(0);
        
        for(int32_t i=0; i<items.Count(); i++)
            ((item*)items.spr(i))->pickup=ipDUMMY;
            
        // Prevent the prices from being displayed anymore
        for(int32_t i=0; i<3; i++)
        {
            prices[i] = 0;
        }
        
        break;
        
    case rMONEY:                                            // secret money
    {
        ((item*)items.spr(0))->pickup = ipDUMMY;

        prices[0] = scr.catchall;
        if (!current_item_power(itype_wallet))
            game->change_drupy(prices[0]);
	//game->set_drupy(game->get_drupy()+price); may be needed everywhere

        putprices(false);
        setmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
        break;
    }
        
    case rGAMBLE:                                           // gamble
    {
        if(game->get_spendable_rupies()<10 && !current_item_power(itype_wallet)) return; //Why 10? 
        
        unsigned si=(zc_oldrand()%24)*3;
        
        for(int32_t i=0; i<3; i++)
            prices[i]=gambledat[si++];
            
	game->set_drupy(game->get_drupy()+prices[priceindex]);
        putprices(true);
        
        for(int32_t i=1; i<4; i++)
            ((item*)items.spr(i))->pickup=ipDUMMY;
    }
    break;
    
    case rBOMBS:
	{
        if(game->get_spendable_rupies()<scr.catchall && !current_item_power(itype_wallet))
            return;
            
		int32_t price = -scr.catchall;
		int32_t wmedal = current_item_id(itype_wealthmedal);
		if(wmedal >= 0)
		{
			if(itemsbuf[wmedal].flags & ITEM_FLAG1)
				price*=(itemsbuf[wmedal].misc1/100.0);
			else
				price+=itemsbuf[wmedal].misc1;
		}
		
		int32_t total = game->get_drupy()-price;
		total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
		game->set_drupy(game->get_drupy()-total);
        //game->set_drupy(game->get_drupy()+price);
        setmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
        game->change_maxbombs(4);
        game->set_bombs(game->get_maxbombs());
        {
            int32_t div = zinit.bomb_ratio;
            
            if(div > 0)
                game->change_maxcounter(4/div, 6);
        }
        
        //also give Hero an actual Bomb item
        for(int32_t i=0; i<MAXITEMS; i++)
        {
            if(itemsbuf[i].family == itype_bomb && itemsbuf[i].fam_type == 1)
                getitem(i, true, true);
        }
        
        ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        //    putscr(scrollbuf,0,0,tmpscr);
        verifyBothWeapons();
        break;
	}
        
    case rARROWS:
	{
        if(game->get_spendable_rupies()<scr.catchall && !current_item_power(itype_wallet))
            return;
            
        int32_t price = -scr.catchall;
		int32_t wmedal = current_item_id(itype_wealthmedal);
		if(wmedal >= 0)
		{
			if(itemsbuf[wmedal].flags & ITEM_FLAG1)
				price*=(itemsbuf[wmedal].misc1/100.0);
			else
				price+=itemsbuf[wmedal].misc1;
		}
		
	int32_t total = game->get_drupy()-price;
	total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
	game->set_drupy(game->get_drupy()-total);

	//game->set_drupy(game->get_drupy()+price);
        setmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
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
	}
        
    case rSWINDLE:
        if(items.spr(index)->id==iRupy)
        {
            if(game->get_spendable_rupies()<scr.catchall && !current_item_power(itype_wallet))
                return;
	    int32_t tmpprice = -scr.catchall;
	    int32_t total = game->get_drupy()-tmpprice;
	    total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
	    game->set_drupy(game->get_drupy()-total);
        }
        else
        {
            if(game->get_maxlife()<=game->get_hp_per_heart())
                return;
                
            game->set_life(zc_max(game->get_life()-game->get_hp_per_heart(),0));
            game->set_maxlife(zc_max(game->get_maxlife()-game->get_hp_per_heart(),(game->get_hp_per_heart())));
        }
        
        setmapflag((currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
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

void getitem(int32_t id, bool nosound, bool doRunPassive)
{
	if(unsigned(id) >= MAXITEMS)
		return;

	if (replay_is_active())
		replay_step_comment(fmt::format("getitem {}", item_string[id]));
	
	if(get_qr(qr_SCC_ITEM_COMBINES_ITEMS))
	{
		int32_t nextitem = -1;
		do
		{
			nextitem = -1;
			if((itemsbuf[id].flags & ITEM_COMBINE) && game->get_item(id))
				// Item upgrade routine.
			{
				
				for(int32_t i=0; i<MAXITEMS; i++)
				{
					// Find the item which is as close to this item's fam_type as possible.
					if(itemsbuf[i].family==itemsbuf[id].family && itemsbuf[i].fam_type>itemsbuf[id].fam_type
							&& (nextitem>-1 ? itemsbuf[i].fam_type<=itemsbuf[nextitem].fam_type : true))
					{
						nextitem = i;
					}
				}
				
				if(nextitem>-1)
				{
					id = nextitem;
					if(!get_qr(qr_ITEMCOMBINE_CONTINUOUS))
						break; //no looping
				}
			}
		} while(nextitem > -1);
	}
	
	itemdata const& idat = itemsbuf[id&0xFF];
	
	bool equipment = idat.flags &
		((idat.family == itype_triforcepiece) ? ITEM_FLAG8 : ITEM_GAMEDATA);
	
	if(equipment)
	{
		// Fix boomerang sounds.
		int32_t itemid = current_item_id(idat.family);
		
		if(itemid>=0 && (idat.family == itype_brang || idat.family == itype_divineprotection
						 || idat.family == itype_hookshot || idat.family == itype_switchhook || idat.family == itype_cbyrna)
				&& sfx_allocated(itemsbuf[itemid].usesound)
				&& idat.usesound != itemsbuf[itemid].usesound)
		{
			stop_sfx(itemsbuf[itemid].usesound);
			cont_sfx(idat.usesound);
		}
		
		int32_t curitm = current_item_id(idat.family);
		if(!get_qr(qr_KEEPOLD_APPLIES_RETROACTIVELY)
			|| curitm < 0 || (itemsbuf[curitm].fam_type <= idat.fam_type)
			|| (itemsbuf[curitm].flags & ITEM_KEEPOLD))
		{
			game->set_item(id,true);
			passiveitem_script(id, doRunPassive);
		}
		
		if(!(idat.flags & ITEM_KEEPOLD))
		{
			if(!get_qr(qr_BROKEN_KEEPOLD_FLAG) || current_item(idat.family)<idat.fam_type)
			{
				removeLowerLevelItemsOfFamily(game,itemsbuf,idat.family, idat.fam_type);
			}
		}
		
		// NES consistency: replace all flying boomerangs with the current boomerang.
		if(idat.family==itype_brang)
			for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				weapon *w = ((weapon*)Lwpns.spr(i));
				
				if(w->id==wBrang)
				{
					w->LOADGFX(idat.wpn);
				}
			}
	}
	
	if(idat.count!=-1)
	{
		if(idat.setmax)
		{
			// Bomb bags are a special case; they may be set not to increase super bombs
			if(idat.family==itype_bombbag && idat.count==2 && (idat.flags&16)==0)
			{
				int32_t max = game->get_maxbombs();
				
				if(max<idat.max) max=idat.max;
				
				game->set_maxbombs(zc_min(game->get_maxbombs()+idat.setmax,max), false);
			}
			else
			{
				int32_t max = game->get_maxcounter(idat.count);
				
				if(max<idat.max) max=idat.max;
				
				game->set_maxcounter(zc_min(game->get_maxcounter(idat.count)+idat.setmax,max), idat.count);
			}
		}
		
		// Amount is an uint16_t, but the range is -9999 to 16383
		// -1 is actually 16385 ... -9999 is 26383, and 0x8000 means use the drain counter
		if(idat.amount&0x3FFF)
		{
			if(idat.amount&0x8000)
				game->set_dcounter(
					game->get_dcounter(idat.count)+((idat.amount&0x4000)?-(idat.amount&0x3FFF):idat.amount&0x3FFF), idat.count);
			else
			{
				if(idat.amount>=16385 && game->get_counter(0)<=idat.amount-16384)
					game->set_counter(0, idat.count);
				else
					// This is too confusing to try and change...
					game->set_counter(zc_min(game->get_counter(idat.count)+((idat.amount&0x4000)?-(idat.amount&0x3FFF):idat.amount&0x3FFF),game->get_maxcounter(idat.count)), idat.count);
			}
		}
	}

	if(idat.playsound&&!nosound)
	{
		sfx(idat.playsound);
	}
	
	//add lower-level items
	if(idat.flags&ITEM_GAINOLD)
	{
		for(int32_t i=idat.fam_type-1; i>0; i--)
		{
			int32_t potid = getItemID(itemsbuf, idat.family, i);
			
			if(potid != -1)
			{
				game->set_item(potid, true);
			}
		}
	}
	
	switch(idat.family)
	{
		case itype_itmbundle:
		{
			int ids[10] = {idat.misc1, idat.misc2, idat.misc3, idat.misc4, idat.misc5,
				idat.misc6, idat.misc7, idat.misc8, idat.misc9, idat.misc10};
			bool pscript = (idat.flags & ITEM_FLAG1);
			for(auto q = 0; q < 10; ++q)
			{
				if(unsigned(ids[q]) >= MAXITEMS) continue;
				if(pscript)
					collectitem_script(ids[q]);
				getitem(ids[q], true, true);
			}
		}
		break;
		
		case itype_progressive_itm:
		{
			int32_t newid = get_progressive_item(idat);
			if(newid > -1)
				getitem(newid, nosound, true);
		}
		break;
		
		case itype_bottlefill:
		{
			if(idat.misc1)
			{
				game->fillBottle(idat.misc1);
			}
		}
		break;
		
		case itype_clock:
		{
			if(idat.flags & ITEM_FLAG1) //Active use, not passive
				break;
			if((idat.flags & ITEM_FLAG2) && clockclk) //"Can't activate while clock active"
				break;
			setClock(watch=true);
			
			clock_zoras.clear();
				
			clockclk=itemsbuf[id&0xFF].misc1;
			sfx(idat.usesound);
		}
		break;
		
		case itype_lkey:
			if(game->lvlkeys[dlevel]<255) game->lvlkeys[dlevel]++;
			break;
			
		case itype_ring:
		case itype_magicring:
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (currscr<128 || dlevel))
			{
				ringcolor(false);
			}
			break;
			
		case itype_whispring:
		{
			if(idat.flags & ITEM_FLAG1)
			{
				if(HeroSwordClk()==-1) setSwordClk(150);  // Let's not bother applying the divisor.
				
				if(HeroItemClk()==-1) setItemClk(150);  // Let's not bother applying the divisor.
			}
			
			if(idat.power==0)
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
		
			game->set_life(zc_min(game->get_life()+(idat.flags&ITEM_FLAG1 ?(int32_t)(game->get_maxlife()*(idat.misc1/100.0)):((idat.misc1*game->get_hp_per_heart()))),game->get_maxlife()));
			game->set_magic(zc_min(game->get_magic()+(idat.flags&ITEM_FLAG2 ?(int32_t)(game->get_maxmagic()*(idat.misc2/100.0)):((idat.misc2*game->get_mp_per_block()))),game->get_maxmagic()));
			break;
			
		case itype_heartpiece:
			game->change_HCpieces(1);
			
			if(game->get_HCpieces()<game->get_hcp_per_hc())
				break;
				
			game->set_HCpieces(0);
			
			getitem(heart_container_id());
			break;
			
		case itype_killem:
		{
			if(idat.flags & ITEM_FLAG1) //Active use, not passive
				break;
			kill_em_all();
			sfx(idat.usesound);
		}
		break;
	}
	
	flushItemCache();
	update_subscreens();
	refresh_subscr_items();
	verifyBothWeapons();
}

void takeitem(int32_t id)
{
    game->set_item(id, false);
    itemdata const& idat = itemsbuf[id];
    
    /* Lower the counters! */
    if(idat.count!=-1)
    {
        if(idat.setmax)
        {
            game->set_maxcounter(game->get_maxcounter(idat.count)-idat.setmax, idat.count);
        }
        
        if(idat.amount&0x3FFF)
        {
            if(idat.amount&0x8000)
                game->set_dcounter(game->get_dcounter(idat.count)-((idat.amount&0x4000)?-(idat.amount&0x3FFF):idat.amount&0x3FFF), idat.count);
            else game->set_counter(game->get_counter(idat.count)-((idat.amount&0x4000)?-(idat.amount&0x3FFF):idat.amount&0x3FFF), idat.count);
        }
    }
    
    switch(itemsbuf[id&0xFF].family)
    {
        // NES consistency: replace all flying boomerangs with the current boomerang.
		case itype_brang:
			if(current_item(itype_brang)) for(int32_t i=0; i<Lwpns.Count(); i++)
			{
				weapon *w = ((weapon*)Lwpns.spr(i));
				
				if(w->id==wBrang)
				{
					w->LOADGFX(itemsbuf[current_item_id(itype_brang)].wpn);
				}
			}
				
			break;
			
		case itype_itmbundle:
		{
			int ids[10] = {idat.misc1, idat.misc2, idat.misc3, idat.misc4, idat.misc5,
				idat.misc6, idat.misc7, idat.misc8, idat.misc9, idat.misc10};
			for(auto q = 0; q < 10; ++q)
			{
				if(unsigned(ids[q]) >= MAXITEMS) continue;
				takeitem(ids[q]);
			}
		}
		break;
		
		case itype_progressive_itm:
		{
			int32_t newid = get_progressive_item(idat, true);
			if(newid > -1)
				takeitem(newid);
		}
		break;
			
		case itype_heartpiece:
			if(game->get_maxlife()>game->get_hp_per_heart())
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
			
		case itype_ring:
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (currscr<128 || dlevel))
			{
				ringcolor(false);
			}
			break;
    }
}

void post_item_collect()
{
	std::vector<int32_t> &ev = FFCore.eventData;
	ev.clear();
	
	throwGenScriptEvent(GENSCR_EVENT_POST_COLLECT_ITEM);
}

void HeroClass::handle_triforce(int32_t id)
{
	if(unsigned(id) >= MAXITEMS)
		return;
	itemdata const& itm = itemsbuf[id];
	switch(itm.family)
	{
		case itype_itmbundle:
		{
			int ids[10] = {itm.misc1, itm.misc2, itm.misc3, itm.misc4, itm.misc5,
				itm.misc6, itm.misc7, itm.misc8, itm.misc9, itm.misc10};
			for(auto q = 0; q < 10; ++q)
			{
				if(unsigned(ids[q]) >= MAXITEMS) continue;
				handle_triforce(ids[q]);
			}
		}
		break;
		case itype_triforcepiece:
		{
			if(itm.misc2>0)
				getTriforce(id); //small
			else getBigTri(id); //big
		}
		break;
	}
}

// Attempt to pick up an item. (-1 = check items touching Hero.)
void HeroClass::checkitems(int32_t index)
{	
	if(index==-1)
	{
		for(auto ind = items.Count()-1; ind >= 0; --ind)
		{
			item* itm = (item*)items.spr(ind);
			if(itm->get_forcegrab())
			{
				checkitems(ind);
			}
		}
		if(diagonalMovement)
		{
			index=items.hit(x,y+(bigHitbox?0:8)-fakez,z,6,6,1);
		}
		else index=items.hit(x,y+(bigHitbox?0:8)-fakez,z,1,1,1);
	}
	
	if(index==-1)
		return;

	item* ptr = (item*)items.spr(index);
	int32_t pickup = ptr->pickup;
	int8_t exstate = ptr->pickupexstate;
	int32_t PriceIndex = ptr->PriceIndex;
	int32_t id2 = ptr->id;
	int32_t holdid = ptr->id;
	int32_t pstr = ptr->pstring;
	int32_t pstr_flags = ptr->pickup_string_flags;
	int32_t linked_parent = ptr->linked_parent;
	// `screen_index_spawned` is probably same as `heroscr`, but could not be if the item moved around.
	int32_t item_screen_index = ptr->screen_index_spawned;
	mapscr* item_screen = get_scr(currmap, item_screen_index);

	// For items grabbed while in a special screen.
	if (currscr >= 128)
		item_screen = &special_warp_return_screen;

	if(ptr->fallclk > 0) return; //Don't pick up a falling item
	
	if(itemsbuf[id2].family == itype_progressive_itm)
	{
		int32_t newid = get_progressive_item(itemsbuf[id2]);
		if(newid > -1)
		{
			id2 = newid;
			holdid = newid;
			pstr = itemsbuf[newid].pstring;
			pstr_flags = itemsbuf[newid].pickup_string_flags;
		}
	}
	
	bool bottledummy = (pickup&ipCHECK) && item_screen->room == rBOTTLESHOP;
	
	if(bottledummy) //Dummy bullshit! 
	{
		if(!game->canFillBottle())
			return;
		if(prices[PriceIndex]!=100000) // 100000 is a placeholder price for free items
		{
			if(!current_item_power(itype_wallet))
			{
				if( game->get_spendable_rupies()<abs(prices[PriceIndex]) ) return;
				int32_t tmpprice = -abs(prices[PriceIndex]);
				//game->change_drupy(-abs(prices[priceindex]));
				int32_t total = game->get_drupy()-tmpprice;
				total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
				game->set_drupy(game->get_drupy()-total);
			}
			else //infinite wallet
			{
				game->change_drupy(0);
			}
		}
		boughtsomething=true;
		//make the other shop items untouchable after
		//you buy something
		int32_t count = 0;
		
		for(int32_t i=0; i<3; i++)
		{
			if(QMisc.bottle_shop_types[item_screen->catchall].fill[i] != 0)
			{
				++count;
			}
		}
		
		for(int32_t i=0; i<items.Count(); i++)
		{
			if(((item*)items.spr(i))->PriceIndex >-1 && i!=index)
				((item*)items.spr(i))->pickup=ipDUMMY+ipFADE;
		}
		
		int32_t slot = game->fillBottle(QMisc.bottle_shop_types[item_screen->catchall].fill[PriceIndex]);
		id2 = find_bottle_for_slot(slot);
		ptr->id = id2;
		pstr = 0;
		pickup |= ipHOLDUP;
	}
	else
	{
		std::vector<int32_t> &ev = FFCore.eventData;
		ev.clear();
		ev.push_back(id2*10000);
		ev.push_back(pickup*10000);
		ev.push_back(pstr*10000);
		ev.push_back(pstr_flags*10000);
		ev.push_back(0);
		ev.push_back(ptr->getUID());
		ev.push_back(GENEVT_ICTYPE_COLLECT*10000);
		ev.push_back(0);
		
		throwGenScriptEvent(GENSCR_EVENT_COLLECT_ITEM);
		bool nullify = ev[4] != 0;
		if(nullify) return;
		id2 = ev[0]/10000;
		pickup = (pickup&(ipCHECK|ipDUMMY)) | (ev[1]/10000);
		pstr = ev[2] / 10000;
		pstr_flags = ev[3] / 10000;
		
		if(itemsbuf[id2].family == itype_bottlefill && !game->canFillBottle())
			return; //No picking these up unless you have a bottle to fill!
		
		if(((pickup&ipTIMER) && (ptr->clk2 < 32))&& !(pickup & ipCANGRAB))
			if(ptr->id!=iFairyMoving)
				// wait for it to stop flashing, doesn't check for other items yet
				return;
				
		if(pickup&ipENEMY)                                        // item was being carried by enemy
			if(more_carried_items()<=1)  // 1 includes this own item.
				screen_item_clear_state(item_screen_index);
				
		if(pickup&ipDUMMY)                                        // dummy item (usually a rupee)
		{
			if(pickup&ipMONEY)
				dospecialmoney(index);
				
			return;
		}
		
		if(get_qr(qr_HEARTSREQUIREDFIX) && !canget(id2))
			return;
			
		int32_t nextitem = -1;
		do
		{
			nextitem = -1;
			if((itemsbuf[id2].flags & ITEM_COMBINE) && game->get_item(id2))
				// Item upgrade routine.
			{
				
				for(int32_t i=0; i<MAXITEMS; i++)
				{
					// Find the item which is as close to this item's fam_type as possible.
					if(itemsbuf[i].family==itemsbuf[id2].family && itemsbuf[i].fam_type>itemsbuf[id2].fam_type
							&& (nextitem>-1 ? itemsbuf[i].fam_type<=itemsbuf[nextitem].fam_type : true))
					{
						nextitem = i;
					}
				}
				
				if(nextitem>-1)
				{
					id2 = nextitem;
					if(get_qr(qr_ITEMCOMBINE_NEW_PSTR))
					{
						pstr = itemsbuf[id2].pstring;
						pstr_flags = itemsbuf[id2].pickup_string_flags;
					}
					if(!get_qr(qr_ITEMCOMBINE_CONTINUOUS))
						break; //no looping
				}
			}
		} while(nextitem > -1);
		
		if(pickup&ipCHECK)                                        // check restrictions
			switch(item_screen->room)
			{
			case rSP_ITEM:                                        // special item
				if(!canget(id2)) // These ones always need the Hearts Required check
					return;
					
				break;
				
			case rP_SHOP:                                         // potion shop
				if(msg_active)
					return;
				[[fallthrough]];
			case rSHOP:                                           // shop
				if(prices[PriceIndex]!=100000) // 100000 is a placeholder price for free items
				{
					if(!current_item_power(itype_wallet))
					{
						if( game->get_spendable_rupies()<abs(prices[PriceIndex]) ) return;
						int32_t tmpprice = -abs(prices[PriceIndex]);
						//game->change_drupy(-abs(prices[priceindex]));
						int32_t total = game->get_drupy()-tmpprice;
						total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
						game->set_drupy(game->get_drupy()-total);
					}
					else //infinite wallet
					{
						game->change_drupy(0);
					}
				}
				boughtsomething=true;
				//make the other shop items untouchable after
				//you buy something
				int32_t count = 0;
				
				for(int32_t i=0; i<3; i++)
				{
					if(QMisc.shop[item_screen->catchall].hasitem[i] != 0)
					{
						++count;
					}
				}
				
				for(int32_t i=0; i<items.Count(); i++)
				{
					if(((item*)items.spr(i))->PriceIndex >-1 && i!=index)
						((item*)items.spr(i))->pickup=ipDUMMY+ipFADE;
				}
				
				break;
			}
			
		if(pickup&ipONETIME)    // set mITEM for one-time-only items
		{
			setmapflag(item_screen_index, mITEM);

			//Okay so having old source files is a godsend. You wanna know why?
			//Because the issue here was never to so with the wrong flag being set; no it's always been setting the right flag.
			//The problem here is that guy rooms were always checking for getmapflag, which used to have an internal check for the default.
			//The default would be mITEM if currscr was under 128 (AKA not in a cave), and mSPECIALITEM if in a cave.
			//However, now the check just always defaults to mSPECIALITEM, which causes this bug.
			//This means that this section of code is no longer a bunch of eggshells, cause none of these overcomplicated compats actually solved shit lmao - Dimi
			
			/*
			// WARNING - Item pickups are very volatile due to crazy compatability hacks, eg., supporting
			// broken behavior from early ZC versions. If you change things here please comment on its purpose.

			// some old quests need picking up a screen item to also disable the BELOW flag (for hunger rooms, etc)
			// What is etc?! We need to check for every valid state here. ~Gleeok
			if(get_qr(qr_ITEMPICKUPSETSBELOW))
			{
				// Most older quests need one-time-pickups to not remove special items, etc.
				if(tmpscr->room==rGRUMBLE)
				{
					setmapflag(mSPECIALITEM);
				}
			}
			*/
		}
		else if(pickup&ipONETIME2)                                // set mSPECIALITEM flag for other one-time-only items
			setmapflag(item_screen_index, (currscr < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		
		if(exstate > -1 && exstate < 32)
		{
			setxmapflag(item_screen_index, 1<<exstate);
		}

		if(pickup&ipSECRETS)                                // Trigger secrets if this item has the secret pickup
		{
			if (item_screen->flags9&fITEMSECRETPERM) setmapflag(item_screen_index, mSECRET);
			trigger_secrets_for_screen(TriggerSource::ItemsSecret, item_screen_index, false);
		}

		collectitem_script(id2);
		getitem(id2, false, true);
	}
	
	if(pickup&ipHOLDUP)
	{
		attackclk=0;
		reset_swordcharge();
		
		if(action!=swimming && !IsSideSwim())
			reset_hookshot();
			
		if(msg_onscreen)
		{
			dismissmsg();
		}
		
		clear_bitmap(pricesdisplaybuf);
		
		if(get_qr(qr_OLDPICKUP) || ((item_screen->room==rSP_ITEM || item_screen->room==rRP_HC || item_screen->room==rTAKEONE) && (pickup&ipONETIME2)) || 
		(get_qr(qr_SHOP_ITEMS_VANISH) && (item_screen->room==rBOTTLESHOP || item_screen->room==rSHOP) && (pickup&ipCHECK)))
		{
			fadeclk=66;
		}
		
		if(id2!=iBombs || action==swimming || get_qr(qr_BOMBHOLDFIX))
		{
			// don't hold up bombs unless swimming or the bomb hold fix quest rule is on
			if(action==swimming)
			{
				action=waterhold1; FFCore.setHeroAction(waterhold1);
			}
			else if(IsSideSwim())
			{
				action=sidewaterhold1; FFCore.setHeroAction(sidewaterhold1);
			}
			else
			{
				action=landhold1; FFCore.setHeroAction(landhold1);
			}
			
			if(ptr->twohand)
			{
				if(action==waterhold1)
				{
					action=waterhold2; FFCore.setHeroAction(waterhold2);
				}
				else if(action==sidewaterhold1)
				{
					action=sidewaterhold2; FFCore.setHeroAction(sidewaterhold2);
				}
				else
				{
					action=landhold2; FFCore.setHeroAction(landhold2);
				}
			}
			
			holdclk=130;
			
			//restart music
			if(get_qr(qr_HOLDNOSTOPMUSIC) == 0)
				music_stop();
			
			holditem=holdid; // NES consistency: when combining blue potions, hold up the blue potion.
			freeze_guys=true;
			//show the info string
			 
			
			//if (pstr > 0 ) //&& itemsbuf[index].pstring < msg_count && ( ( itemsbuf[index].pickup_string_flags&itemdataPSTRING_ALWAYS || itemsbuf[index].pickup_string_flags&itemdataPSTRING_IP_HOLDUP ) ) )
			
			int32_t shop_pstr = 0;
			if (PriceIndex > -1) 
			{
				switch(item_screen->room)
				{
					case rSHOP:
						shop_pstr = QMisc.shop[item_screen->catchall].str[PriceIndex];
						break;
					case rBOTTLESHOP:
						shop_pstr = QMisc.bottle_shop_types[item_screen->catchall].str[PriceIndex];
						break;
				}
			}
			if ( (pstr > 0 && pstr < msg_count) || (shop_pstr > 0 && shop_pstr < msg_count) )
			{
				if ( (pstr > 0 && pstr < msg_count) && ( ( ( pstr_flags&itemdataPSTRING_ALWAYS || pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_IP_HOLDUP || (!(FFCore.GetItemMessagePlayed(id2)))  ) ) ) )
				{
					if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
				}
				else pstr = 0;
				if(shop_pstr)
				{
					donewmsg(shop_pstr);
					enqueued_str = pstr;
				}
				else if(pstr)
				{
					donewmsg(pstr);
				}
			}
			
		}
		
		if(itemsbuf[id2].family!=itype_triforcepiece || !(itemsbuf[id2].flags & ITEM_GAMEDATA))
		{
			sfx(tmpscr->holdupsfx);
		}
		
		ptr->set_forcegrab(false);
		items.del(index);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
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
				{
					items.del(0);
					
					for(int32_t i=0; i<Lwpns.Count(); i++)
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
		ptr->set_forcegrab(false);
		items.del(index);
		
		for(int32_t i=0; i<Lwpns.Count(); i++)
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
		int32_t shop_pstr = ( item_screen->room == rSHOP && PriceIndex>=0 && QMisc.shop[item_screen->catchall].str[PriceIndex] > 0 ) ? QMisc.shop[item_screen->catchall].str[PriceIndex] : 0;
		if ( (pstr > 0 && pstr < msg_count) || (shop_pstr > 0 && shop_pstr < msg_count) )
		{
			if ( (pstr > 0 && pstr < msg_count) && ( (!(pstr_flags&itemdataPSTRING_IP_HOLDUP)) && ( pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(id2))) ) ) )
			{
				if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
			}
			else pstr = 0;
			if(shop_pstr)
			{
				donewmsg(shop_pstr);
				enqueued_str = pstr;
			}
			else if(pstr)
			{
				donewmsg(pstr);
			}
		}
		
		
		clear_bitmap(pricesdisplaybuf);
		set_clip_state(pricesdisplaybuf, 1);
	}
	
	if(itemsbuf[id2].family==itype_triforcepiece
		&& itemsbuf[id2].misc2 <= 0 && linked_parent == eeGANON)
	{
		game->lvlitems[dlevel]|=liBOSS;
	}
	handle_triforce(id2);
	if(!holdclk)
		post_item_collect();
}

void HeroClass::StartRefill(int32_t refillWhat)
{
	if(!refilling)
	{
		refillclk=21;
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
		sfx(WAV_REFILL,128,true);
		refilling=refillWhat;
		if(FFCore.quest_format[vZelda] < 0x255) 
		{
			//Yes, this isn't a QR check. This was implemented before the QRs got bumped up.
			//I attempted to change this check to a quest rule, but here's the issue: this affects
			//triforces and potions as well, not just fairy flags. This means that having a compat rule
			//would result in a rule that is checked by default for every tileset or quest made before
			//2.55, one in a place most people won't check. That means that if they were to go to use
			//the new potion or triforce flags for jinx curing behavior, they'd find that it doesn't work,
			//all because of an obscure compat rule being checked. Most peoples instincts are sadly not
			//"go through the compat rules and turn them all off", so this remains a version check instead
			//of a qr check. Don't make my mistake and waste time trying to change this in vain. -Deedee
			Start250Refill(refillWhat);
		}
		else //use 2.55+ behavior
		{
			if(refill_why>=0) // Item index
			{
				if(itemsbuf[refill_why].family==itype_potion)
				{
					if(itemsbuf[refill_why].flags & ITEM_FLAG3){swordclk=0;verifyAWpn();}
					if(itemsbuf[refill_why].flags & ITEM_FLAG4)itemclk=0;
				}
				else if(itemsbuf[refill_why].family==itype_triforcepiece)
				{
					if(itemsbuf[refill_why].flags & ITEM_FLAG3){swordclk=0;verifyAWpn();}
					if(itemsbuf[refill_why].flags & ITEM_FLAG4)itemclk=0;
				}
			}
			else if(refill_why==REFILL_FAIRY)
			{
				if(!get_qr(qr_NONBUBBLEFAIRIES)){swordclk=0;verifyAWpn();}
				if(get_qr(qr_ITEMBUBBLE))itemclk=0;
			}
		}
	}
}

void HeroClass::Start250Refill(int32_t refillWhat)
{
	if(!refilling)
	{
		refillclk=21;
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
		sfx(WAV_REFILL,128,true);
		refilling=refillWhat;
		
		if(refill_why>=0) // Item index
		{
			if((itemsbuf[refill_why].family==itype_potion)&&(!get_qr(qr_NONBUBBLEMEDICINE)))
			{
				swordclk=0;
				verifyAWpn();
				if(get_qr(qr_ITEMBUBBLE)) itemclk=0;
			}

			if((itemsbuf[refill_why].family==itype_triforcepiece)&&(!get_qr(qr_NONBUBBLETRIFORCE)))
			{
				swordclk=0;
				verifyAWpn();
				if(get_qr(qr_ITEMBUBBLE)) itemclk=0;
			}
		}
		else if((refill_why==REFILL_FAIRY)&&(!get_qr(qr_NONBUBBLEFAIRIES)))
		{
			swordclk=0;
			verifyAWpn();
			if(get_qr(qr_ITEMBUBBLE)) itemclk=0;
		}
	}
}

bool HeroClass::refill()
{
    if(refilling==REFILL_NONE || refilling==REFILL_FAIRYDONE)
    {
        return false;
    }
    
    ++refillclk;
    int32_t speed = get_qr(qr_FASTFILL) ? 6 : 22;
    int32_t refill_heart_stop=game->get_maxlife();
    int32_t refill_magic_stop=game->get_maxmagic();
    
    if(refill_why>=0 && itemsbuf[refill_why].family==itype_potion)
    {
        refill_heart_stop=zc_min(potion_life+(itemsbuf[refill_why].flags & ITEM_FLAG1 ?int32_t(game->get_maxlife()*(itemsbuf[refill_why].misc1 /100.0)):((itemsbuf[refill_why].misc1 *game->get_hp_per_heart()))),game->get_maxlife());
        refill_magic_stop=zc_min(potion_magic+(itemsbuf[refill_why].flags & ITEM_FLAG2 ?int32_t(game->get_maxmagic()*(itemsbuf[refill_why].misc2 /100.0)):((itemsbuf[refill_why].misc2 *game->get_mp_per_block()))),game->get_maxmagic());
    }
    
    if(refillclk%speed == 0)
    {
        //   game->life&=0xFFC;
        switch(refill_what)
        {
        case REFILL_LIFE:
            game->set_life(zc_min(refill_heart_stop, (game->get_life()+game->get_hp_per_heart()/2)));
            
            if(game->get_life()>=refill_heart_stop)
            {
                game->set_life(refill_heart_stop);
                //kill_sfx(); //this 1. needs to be pause resme, and 2. needs an item flag.
                for ( int32_t q = 0; q < WAV_COUNT; q++ )
				{
					if ( q == (int32_t)tmpscr->oceansfx ) continue;
					if ( q == (int32_t)tmpscr->bosssfx ) continue;
					stop_sfx(q);
				}
				sfx(QMisc.miscsfx[sfxREFILL]);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
            
        case REFILL_MAGIC:
            game->set_magic(zc_min(refill_magic_stop, (game->get_magic()+game->get_mp_per_block()/4)));
            
            if(game->get_magic()>=refill_magic_stop)
            {
                game->set_magic(refill_magic_stop);
                //kill_sfx(); //this 1. needs to be pause resme, and 2. needs an item flag.
                for ( int32_t q = 0; q < WAV_COUNT; q++ )
				{
					if ( q == (int32_t)tmpscr->oceansfx ) continue;
					if ( q == (int32_t)tmpscr->bosssfx ) continue;
					stop_sfx(q);
				}
                sfx(QMisc.miscsfx[sfxREFILL]);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
            
        case REFILL_ALL:
            game->set_life(zc_min(refill_heart_stop, (game->get_life()+game->get_hp_per_heart()/2)));
            game->set_magic(zc_min(refill_magic_stop, (game->get_magic()+game->get_mp_per_block()/4)));
            
            if((game->get_life()>=refill_heart_stop)&&(game->get_magic()>=refill_magic_stop))
            {
                game->set_life(refill_heart_stop);
                game->set_magic(refill_magic_stop);
                //kill_sfx(); //this 1. needs to be pause resme, and 2. needs an item flag.
                for ( int32_t q = 0; q < WAV_COUNT; q++ )
				{
					if ( q == (int32_t)tmpscr->oceansfx ) continue;
					if ( q == (int32_t)tmpscr->bosssfx ) continue;
					stop_sfx(q);
				}
                sfx(QMisc.miscsfx[sfxREFILL]);
                refilling=REFILL_NONE;
                return false;
            }
            
            break;
        }
    }
    
    return true;
}

void HeroClass::getTriforce(int32_t id2)
{
		
	PALETTE flash_pal;
	int32_t refill_frame = ( (itemsbuf[id2].misc5 > 0) ? itemsbuf[id2].misc5 : 88 );
	
	for(int32_t i=0; i<256; i++)
	{
		flash_pal[i] = get_qr(qr_FADE) ? _RGB(63,63,0) : _RGB(63,63,63); 
	}



	//get rid off all sprites but Hero
	guys.clear();
	items.clear();
	Ewpns.clear();
	Lwpns.clear();
	chainlinks.clear();
    
	kill_subscr_items();
	
	//decorations.clear();
	if(!COOLSCROLL)
	{
		show_subscreen_items=false;
	}
    
	sfx(itemsbuf[id2].playsound);
	if ( !(itemsbuf[id2].flags & ITEM_FLAG11) ) music_stop();
	
	//If item flag six is enabled, and a sound is set to attributes[2], play that sound.
	if ( (itemsbuf[id2].flags & ITEM_FLAG14) )
	{
		uint8_t playwav = itemsbuf[id2].misc3;
		//zprint2("playwav is: %d\n", playwav);
		sfx(playwav);
		
	}
		
	//itemsbuf[id2].flags & ITEM_FLAG9 : Don't dismiss Messages
	//itemsbuf[id2].flags & ITEM_FLAG10 : Cutscene interrupts action script..
	//itemsbuf[id2].flags & ITEM_FLAG11 : Don't change music.
	//itemsbuf[id2].flags & ITEM_FLAG12 : Run Collect Script Script On Collection
	//itemsbuf[id2].flags & ITEM_FLAG13 : Run Action Script On Collection
	//itemsbuf[id2].flags & ITEM_FLAG14 : Play second sound (WAV) from Attributes[2] (misc2)
	//itemsbuf[id2].flags & ITEM_FLAG15 : No MIDI
    
	if(!(itemsbuf[id2].flags & ITEM_FLAG15)) //No MIDI flag
	{
		if(itemsbuf[id2].misc1)
			jukebox(itemsbuf[id2].misc1+ZC_MIDI_COUNT-1);
		else
			try_zcmusic("zelda.nsf",moduledata.tf_track, ZC_MIDI_TRIFORCE);
	}
	if(itemsbuf[id2].flags & ITEM_GAMEDATA)
	{
		game->lvlitems[dlevel]|=liTRIFORCE;
	}
    
	int32_t f=0;
	int32_t x2=0;
	int32_t curtain_x=0;
	int32_t c=0;

	do
	{
		
		
		if ( (itemsbuf[id2].flags & ITEM_FLAG13) ) //Run action script on collection.
		{
			if ( itemsbuf[id2].script )
			{
				if ( !FFCore.doscript(ScriptType::Item, id2) ) 
				{
					int i = id2;
					FFCore.reset_script_engine_data(ScriptType::Item, i);
					ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id2].script, i);
					FFCore.deallocateAllScriptOwned(ScriptType::Item, i);
				}
				else
				{
					if ( !(itemsbuf[id2].flags & ITEM_FLAG10) ) //Cutscene halts the script it resumes after cutscene.
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id2].script, id2); //if flag is off, run the script every frame of the cutscene.
				}
			}
		}
		//if ( itemsbuf[id2].misc2 == 2 ) //No cutscene; what if people used '2' on older quests?
		if ( (itemsbuf[id2].flags & ITEM_FLAG12) ) //No cutscene
		{
			return;
		}
		if(f==40)
		{
			actiontype oldaction = action;
			ALLOFF((!(itemsbuf[id2].flags & ITEM_FLAG9)), false);
			action=oldaction;                                    // have to reset this flag
			FFCore.setHeroAction(oldaction);
		}
	
	
		if(f>=40 && f<88)
		{
		    if(get_qr(qr_FADE))
		    {
			if (!flash_reduction_enabled() && (f&7) == 0)
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
			if(!flash_reduction_enabled() && (f&7) == 0)
			{
			    for(int32_t cs2=2; cs2<5; cs2++)
			    {
				for(int32_t i=1; i<16; i++)
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
			if(f==refill_frame)
			{
				refill_what=REFILL_ALL;
				refill_why=id2;
				StartRefill(REFILL_ALL);
				refill();
			}
	    
			if(f==(refill_frame+1))
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
						[[fallthrough]];
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
				//draw_screen();
			}
		}
	
		draw_screen();
		//this causes bugs
		//the subscreen appearing over the curtain effect should now be fixed in draw_screen
		//so this is not necessary -DD
		//put_passive_subscr(framebuf,0,passive_subscreen_offset,false,false);
		
		//Run Triforce Script
		advanceframe(true);
		++f;
	}
	while
	(
		(f < ( (itemsbuf[id2].misc4 > 0) ? itemsbuf[id2].misc4 : 408)) 
		|| (!(itemsbuf[id2].flags & ITEM_FLAG15) /*&& !(itemsbuf[id2].flags & ITEM_FLAG11)*/ && (midi_pos > 0 && !replay_is_active())) 
		|| (/*!(itemsbuf[id2].flags & ITEM_FLAG15) &&*/ !(itemsbuf[id2].flags & ITEM_FLAG11) && (zcmusic!=NULL) && (zcmusic->position<800 && !replay_is_active())
		// Music is played at the same speed when fps is uncapped, so in replay mode we need to ignore the music position and instead
		// just count frames. 480 is the number of frames it takes for the triforce song in classic_1st.qst to finish playing, but the exact
		// value doesn't matter.
		|| (replay_is_active() && f < 480) )
	);   // 800 may not be just right, but it works

	action=none; FFCore.setHeroAction(none);
	holdclk=0;
	draw_screen_clip_rect_x1=0;
	draw_screen_clip_rect_x2=255;
	draw_screen_clip_rect_y1=0;
	draw_screen_clip_rect_y2=223;
	show_subscreen_items=true;
    
	//Warp Hero out of item cellars, in 2.10 and earlier quests. -Z ( 16th January, 2019 )
	//Added a QR for this, to Other->2, as `Triforce in Cellar Warps Hero Out`. -Z 15th March, 2019 
	if((itemsbuf[id2].flags & ITEM_FLAG1) && ( get_qr(qr_SIDEVIEWTRIFORCECELLAR) ? ( currscr < MAPSCRS192b136 ) : (currscr < MAPSCRSNORMAL) ) )
	{
		sdir=dir;
		dowarp(1,0); //side warp
	}
	else
	{
		if ( !(itemsbuf[id2].flags & ITEM_FLAG11) ) playLevelMusic();
	}
}

void red_shift()
{
    int32_t tnum=176;
    
    // set up the new palette
    for(int32_t i=CSET(2); i < CSET(4); i++)
    {
        int32_t r = (i-CSET(2)) << 1;
        RAMpal[i+tnum].r = r;
        RAMpal[i+tnum].g = r >> 3;
        RAMpal[i+tnum].b = r >> 4;
    }
    
    // color scale the game screen
    for(int32_t y=0; y<viewport.h-8; y++)
    {
        for(int32_t x=0; x<256; x++)
        {
            int32_t c = framebuf->line[y+original_playing_field_offset][x];
            int32_t r = zc_min(int32_t(RAMpal[c].r*0.4 + RAMpal[c].g*0.6 + RAMpal[c].b*0.4)>>1,31);
            framebuf->line[y+original_playing_field_offset][x] = (c ? (r+tnum+CSET(2)) : 0);
        }
    }
    
    refreshpal = true;
}

void slide_in_color(int32_t color)
{
    for(int32_t i=1; i<16; i+=3)
    {
        RAMpal[CSET(2)+i+2] = RAMpal[CSET(2)+i+1];
        RAMpal[CSET(2)+i+1] = RAMpal[CSET(2)+i];
        RAMpal[CSET(2)+i]   = NESpal(color);
    }
    
    refreshpal=true;
}


void HeroClass::heroDeathAnimation()
{
	int32_t f=0;
	int32_t deathclk=0,deathfrm=0;
    
	action=none; FFCore.setHeroAction(dying); //mayhaps a new action of 'gameover'? -Z
	
	kill_sfx();  //call before the onDeath script.
	
	if(!debug_enabled)
	{
		Paused=false;
	}
    
    /*
	game->set_deaths(zc_min(game->get_deaths()+1,999));
	dir=down;
	music_stop();
	
	attackclk=hclk=superman=0;
	scriptcoldet = 1;
    
	for(int32_t i=0; i<32; i++) miscellaneous[i] = 0;
    
	
    
	playing_field_offset=56; // otherwise, red_shift() may go past the bottom of the screen
	quakeclk=wavy=0;
    
	//in original Z1, Hero marker vanishes at death.
	//code in subscr.cpp, put_passive_subscr checks the following value.
	//color 255 is a GUI color, so quest makers shouldn't be using this value.
	//Also, subscreen is static after death in Z1.
	int32_t tmp_hero_dot = QMisc.colors.hero_dot;
	QMisc.colors.hero_dot = 255;
	//doesn't work
	//scrollbuf is tampered with by draw_screen()
	//put_passive_subscr(scrollbuf, 256, passive_subscreen_offset, false, false);//save this and reuse it.
	BITMAP *subscrbmp = create_bitmap_ex(8, framebuf->w, framebuf->h);
	clear_bitmap(subscrbmp);
	put_passive_subscr(subscrbmp, 0, passive_subscreen_offset, false, sspUP);
	QMisc.colors.hero_dot = tmp_hero_dot;
    */
	BITMAP *subscrbmp = create_bitmap_ex(8, framebuf->w, framebuf->h);
				clear_bitmap(subscrbmp);
				//get rid off all sprites but Hero
				guys.clear();
				items.clear();
				Ewpns.clear();
				Lwpns.clear();
				chainlinks.clear();
				decorations.clear();
				
				kill_subscr_items();
				
				Playing = false;
					
				game->set_deaths(zc_min(game->get_deaths()+1,USHRT_MAX));
				dir=down;
				music_stop();
				
				attackclk=hclk=superman=0;
				scriptcoldet = 1;
			    
				for(int32_t i=0; i<32; i++) miscellaneous[i] = 0;
			    
				
				playing_field_offset = original_playing_field_offset; // otherwise, red_shift() may go past the bottom of the screen
				quakeclk=wavy=0;
			    
				//in original Z1, Hero marker vanishes at death.
				//code in subscr.cpp, put_passive_subscr checks the following value.
				//color 255 is a GUI color, so quest makers shouldn't be using this value.
				//Also, subscreen is static after death in Z1.
				int32_t tmp_hero_dot = QMisc.colors.hero_dot;
				QMisc.colors.hero_dot = 255;
				//doesn't work
				//scrollbuf is tampered with by draw_screen()
				//put_passive_subscr(scrollbuf, 256, passive_subscreen_offset, false, false);//save this and reuse it.
				
				put_passive_subscr(subscrbmp, 0, passive_subscreen_offset, game->should_show_time(), sspUP);
				//Don't forget passive subscreen scripts!
				if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN))
				{
					script_drawing_commands.Clear(); //We only want draws from this script
					if(DMaps[currdmap].passive_sub_script != 0)
						ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script, currdmap);
					if (FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[currdmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
					{
						ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[currdmap].passive_sub_script, currdmap);
						FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
					}
					BITMAP* tmp = framebuf;
					framebuf = subscrbmp; //Hack; force draws to subscrbmp
					do_script_draws(framebuf, tmpscr, 0, playing_field_offset); //Draw the script draws
					framebuf = tmp;
					script_drawing_commands.Clear(); //Don't let these draws repeat during 'draw_screen()'
				}
				QMisc.colors.hero_dot = tmp_hero_dot;
        bool clearedit = false;
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
                
				herostep();
			}
            
			if(f>=194 && f<208)
			{
				if(f==194)
				{
					action=dying;
					FFCore.setHeroAction(dying);
				}
                    
				extend = 0;
				cs = wpnsbuf[spr_death].csets&15;
				tile = wpnsbuf[spr_death].tile;
				if(!get_qr(qr_HARDCODED_ENEMY_ANIMS))
				{
					tile += deathfrm;
					f = 206;
					if(++deathclk >= wpnsbuf[spr_death].speed)
					{
						deathclk=0;
						if(++deathfrm >= wpnsbuf[spr_death].frames)
						{
							f = 208;
							deathfrm = 0;
						}
					}
				}
				else if(BSZ)
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
			if(get_qr(qr_FADE))
			{
				if(f < 170)
				{
					if(f<60)
					{
						draw_screen();
						//reuse our static subscreen
						set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
						blit(subscrbmp,framebuf,0,0,0,0,256,original_playing_field_offset);
					}
                    
					if(f==60)
					{
						red_shift();
						create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
						create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
						memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                        
						for(int32_t q=0; q<PAL_SIZE; q++)
						{
							trans_table2.data[0][q] = q;
							trans_table2.data[q][q] = q;
						}
					}
                    
					if(f>=60 && f<=169)
					{
						draw_screen();
						//reuse our static subscreen
						blit(subscrbmp,framebuf,0,0,0,0,256,original_playing_field_offset);
						red_shift();
                        
					}
                    
					if(f>=139 && f<=169)//fade from red to black
					{
						fade_interpolate(RAMpal,black_palette,RAMpal, (f-138)<<1, 224, 255);
						create_rgb_table_range(&rgb_table, RAMpal, 208, 239, NULL);
						create_zc_trans_table(&trans_table, RAMpal, 128, 128, 128);
						memcpy(&trans_table2, &trans_table, sizeof(COLOR_MAP));
                        
						for(int32_t q=0; q<PAL_SIZE; q++)
						{
							trans_table2.data[0][q] = q;
							trans_table2.data[q][q] = q;
						}
                        
						refreshpal=true;
					}
				}
				else //f>=170
				{
					if(f==170)//make Hero grayish
					{
						fade_interpolate(RAMpal,black_palette,RAMpal,64, 224, 255);
                        
						for(int32_t i=CSET(6); i < CSET(7); i++)
						{
							int32_t g = (RAMpal[i].r + RAMpal[i].g + RAMpal[i].b)/3;
							RAMpal[i] = _RGB(g,g,g);
						}
                        
						refreshpal = true;
					}
                    
					//draw only hero. otherwise black layers might cover him.
					rectfill(framebuf,0,original_playing_field_offset,framebuf->w,framebuf->h,0);
					draw(framebuf);
					blit(subscrbmp,framebuf,0,0,0,0,256,original_playing_field_offset);
				}
			}
			else //!qr_FADE
			{
				if(f==58)
				{
					for(int32_t i = 0; i < 96; i++)
						tmpscr->cset[i] = 3;
                        
					for(int32_t j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int32_t i=0; i<96; i++)
								tmpscr2[j].cset[i] = 3;
				}
                
				if(f==59)
				{
					for(int32_t i = 96; i < 176; i++)
						tmpscr->cset[i] = 3;
                        
					for(int32_t j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int32_t i=96; i<176; i++)
								tmpscr2[j].cset[i] = 3;
				}
                
				if(f==60)
				{
					for(int32_t i=0; i<176; i++)
					{
						tmpscr->cset[i] = 2;
					}
                    
					for(int32_t j=0; j<6; j++)
						if(tmpscr->layermap[j]>0)
							for(int32_t i=0; i<176; i++)
								tmpscr2[j].cset[i] = 2;
                                
					for(int32_t i=1; i<16; i+=3)
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
					for(int32_t i=1; i<16; i+=3)
					{
						RAMpal[CSET(6)+i]   = NESpal(0x10);
						RAMpal[CSET(6)+i+1] = NESpal(0x30);
						RAMpal[CSET(6)+i+2] = NESpal(0x00);
						refreshpal = true;
					}
				}
                
				if(f < 169)
				{
					draw_screen();
					//reuse our static subscreen
					blit(subscrbmp,framebuf,0,0,0,0,256,original_playing_field_offset);
				}
				else
				{
					//draw only hero. otherwise black layers might cover him.
					rectfill(framebuf,0,playing_field_offset,255,167+playing_field_offset,0);
					draw(framebuf);
					blit(subscrbmp,framebuf,0,0,0,0,256,original_playing_field_offset);
				}
			}
		}
		
		else if(f<350)//draw 'GAME OVER' text
		{
			if(get_qr(qr_INSTANT_RESPAWN) && !get_qr(qr_INSTANT_CONTINUE))
			{
				Quit = qRELOAD;
				skipcont = 1;
				clear_bitmap(framebuf);
				blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
			}
			else if(!get_qr(qr_INSTANT_RESPAWN) && get_qr(qr_INSTANT_CONTINUE))
			{
				Quit = qCONT;
				skipcont = 1;
				clear_bitmap(framebuf);
				blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
			}
			else
			{
				clear_a5_bmp(rti_infolayer.bitmap);
				clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
				blit(subscrbmp,framebuf,0,0,0,0,256,passive_subscreen_height);
				textout_ex(framebuf,get_zc_font(font_zfont),"GAME OVER",96,playing_field_offset+80,SaveScreenSettings[SAVESC_TEXT],-1);
			}
		}
		else
		{
			clear_bitmap(framebuf);
		}
        
		//SFX... put them all here
		switch(f)
		{
		case   0:
			sfx(getHurtSFX(),pan(x.getInt()));
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
		//adv:
		clear_darkroom_bitmaps();
		advanceframe(true);
		++f;
	}
	while(f<353 && !Quit);
    
	destroy_bitmap(subscrbmp);
	action=none; FFCore.setHeroAction(none);
	if ( dontdraw < 2 ) { dontdraw=0; }
}

void HeroClass::ganon_intro()
{
	auto [offx, offy] = translate_screen_coordinates_to_world(heroscr);

    /*
    ************************
    * GANON INTRO SEQUENCE *
    ************************
    -25 DOT updates
    -24 HERO in
    0 TRIFORCE overhead - code begins at this point (f == 0)
    47 GANON in
    58 LIGHT step
    68 LIGHT step
    78 LIGHT step
    255 TRIFORCE out
    256 TRIFORCE in
    270 TRIFORCE out
    271 GANON out, HERO face up
    */
    loaded_guys=true;
    loaditem(tmpscr, currscr, offx, offy);
    
    if(game->lvlitems[dlevel]&liBOSS)
    {
        return;
    }
    
    dir=down;
    if ( !isSideViewHero() )
    {
	fall = 0; //Fix midair glitch on holding triforce. -Z
	fakefall = 0;
	z = 0;
	fakez = 0;
    }
    action=landhold2; FFCore.setHeroAction(landhold2);
    holditem=getItemID(itemsbuf,itype_triforcepiece, 1);
    //not good, as this only returns the highest level that Hero possesses. -DD
    //getHighestLevelOfFamily(game, itemsbuf, itype_triforcepiece, false));
    
    for(int32_t f=0; f<271 && !Quit; f++)
    {
        if(f==47)
        {
            music_stop();
            stop_sfx(WAV_ROAR);
            sfx(WAV_GASP);
            sfx(WAV_GANON);
            int32_t Id=0;
            
            for(int32_t i=0; i<eMAXGUYS; i++)
            {
                if(guysbuf[i].flags2&eneflag_ganon)
                {
                    Id=i;
                    break;
                }
            }
            
            if(current_item(itype_ring))
            {
                addenemy(currscr,offx+160,offy+96,Id,0);
            }
            else
            {
                addenemy(currscr,offx+80,offy+32,Id,0);
            }
        }
        
        if(f==48)
        {
            lighting(true,true); // Hmm. -L
            f += 30;
        }
        
        //NES Z1, the triforce vanishes for one frame in two cases
        //while still showing Hero's two-handed overhead sprite.
	//This should be a Quest Rule for NES Accuracy. -Z
        if(f==255 || f==270)
        {
            holditem=-1;
        }
        
        if(f==256)
        {
            holditem=getItemID(itemsbuf,itype_triforcepiece,1);
        }
        
        draw_screen();
        advanceframe(true);
        
        if(rSbtn())
        {
            conveyclk=3;
            int32_t tmp_subscr_clk = frame;
            dosubscr();
            newscr_clk += frame - tmp_subscr_clk;
        }
        
    }
    
    action=none; FFCore.setHeroAction(none);
    dir=up;
    
    if((!getmapflag() || (tmpscr->flags9&fBELOWRETURN)) && (tunes[MAXMIDIS-1].data))
        jukebox(MAXMIDIS-1);
    else
        playLevelMusic();
        
    currcset=DMaps[currdmap].color;
    if (get_qr(qr_GANONINTRO) ) 
    {
	dointro();
	//Yes, I checked. This is literally in 2.10 (minus this if statement of course).
	//I have no clue why it's here; Literally the only difference between dointro in 2.10 and dointro in this version is an 'else' that sets introclk and intropos to 74.
	//I have no idea what was going through the original devs heads and I'm extremely worried I'm missing something, cause at first glance this looks like 
	//a hack solution to an underlying bug, but no! There's just a fucking dointro() call in older versions and I don't know *why*. -Deedee
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

void HeroClass::win_game()
{
    replay_step_comment("win_game");
    Playing=Paused=false;
    action=won; FFCore.setHeroAction(won);
    Quit=qWON;
    hclk=0;
    x = 136;
    y = (isdungeon() && currscr<128) ? 75 : 73;
    z = fakez = fall = fakefall = spins = 0;
    dir=left;
}

void HeroClass::reset_swordcharge()
{
    charging=spins=tapping=0;
}

void HeroClass::reset_hookshot()
{
	if(action!=walking && action!=rafting && action!=landhold1 && action!=landhold2 && action!=sidewaterhold1 && action!=sidewaterhold2)
	{
		action=none; FFCore.setHeroAction(none);
	}
	
	hookshot_frozen=false;
	hookshot_used=false;
	pull_hero=false;
	hs_fix=false;
	switchhookclk = switchhookmaxtime = switchhookstyle = switchhookarg = 0;
	switch_hooked = false;
	if(switching_object)
		switching_object->switch_hooked = false;
	switching_object = NULL;
	hooked_comborpos = rpos_t::None;
	switchhook_cost_item = -1;
	hooked_layerbits = 0;
	for(auto q = 0; q < 7; ++q)
		hooked_undercombos[q] = -1;
	Lwpns.del(Lwpns.idFirst(wHSHandle));
	Lwpns.del(Lwpns.idFirst(wHookshot));
	chainlinks.clear();
	int32_t index=directWpn>-1 ? directWpn : current_item_id(hs_switcher ? itype_switchhook : itype_hookshot);
	hs_switcher = false;
	
	if(index>=0)
	{
		stop_sfx(itemsbuf[index].usesound);
	}
	
	hs_xdist=0;
	hs_ydist=0;
}


bool HeroClass::can_deploy_ladder()
{
    bool ladderallowed = ((!get_qr(qr_LADDERANYWHERE) && (tmpscr->flags&fLADDER)) || isdungeon()
                          || (get_qr(qr_LADDERANYWHERE) && !(tmpscr->flags&fLADDER)));
    return (current_item_id(itype_ladder)>-1 && ladderallowed && !ilswim && z==0 && fakez==0 &&
            (!isSideViewHero() || on_sideview_solid_oldpos(x,y,old_x,old_y)));
}

void HeroClass::reset_ladder()
{
    ladderx=laddery=0;
}

bool is_conveyor(int32_t type);
int32_t get_conveyor(int32_t x, int32_t y);

void HeroClass::check_conveyor()
{
	++newconveyorclk;
	if (newconveyorclk < 0) newconveyorclk = 0;
	
	if(action==casting||action==sideswimcasting||action==drowning || action==sidedrowning||action==lavadrowning||inlikelike||pull_hero||((z>0||fakez>0) && !(tmpscr->flags2&fAIRCOMBOS)))
	{
		is_conveyor_stunned = 0;
		return;
	}
	
	WalkflagInfo info;
	int32_t xoff,yoff;
	zfix deltax(0), deltay(0);
	int32_t cmbid = get_conveyor(x+7,y+(bigHitbox?8:12));
	if(cmbid < 0) 
	{
		if (conveyclk <= 0) is_on_conveyor=false;
		return;
	}
	newcombo const* cmb = &combobuf[cmbid];
	rpos_t rpos = COMBOPOS_REGION(x+7,y+(bigHitbox?8:12));
	bool custom_spd = (cmb->usrflags&cflag2);
	if(custom_spd || conveyclk<=0) //!DIMITODO: let player be on multiple conveyors at once
	{
		int32_t ctype=cmb->type;
		auto rate = custom_spd ? zc_max(cmb->attribytes[0], 1) : 3;
		if(custom_spd && (newconveyorclk % rate)) return;
		if((cmb->usrflags&cflag5) && HasHeavyBoots())
			return;
		is_on_conveyor=false;
		conv_forcedir=-1;
		is_conveyor_stunned=0;
		
		deltax=combo_class_buf[ctype].conveyor_x_speed;
		deltay=combo_class_buf[ctype].conveyor_y_speed;
		
		if (is_conveyor(ctype) && custom_spd)
		{
			deltax = zslongToFix(cmb->attributes[0]);
			deltay = zslongToFix(cmb->attributes[1]);
		}
		
		if((deltax==0&&deltay==0)&&(isSideViewHero() && on_sideview_solid_oldpos(x,y,old_x,old_y)))
		{
			cmbid = MAPCOMBO(x+8,y+16);
			cmb = &combobuf[cmbid];
			custom_spd = cmb->usrflags&cflag2;
			ctype=(cmb->type);
			rate = custom_spd ? zc_max(cmb->attribytes[0], 1) : 3;
			deltax=combo_class_buf[ctype].conveyor_x_speed;
			deltay=combo_class_buf[ctype].conveyor_y_speed;
			if ((deltax != 0 || deltay != 0) && custom_spd)
			{
				deltax = zslongToFix(cmb->attributes[0]);
				deltay = zslongToFix(cmb->attributes[1]);
			}
		}
		
		if(deltax!=0||deltay!=0)
		{
			is_on_conveyor=true;
		}
		else return;
		
		bool forcewalk = (cmb->usrflags&cflag6) && get_qr(qr_NEW_HERO_MOVEMENT2);
		if(forcewalk)
		{
			is_conveyor_stunned = rate;
			if((cmb->usrflags&cflag3) && !spins)
			{
				if(abs(deltax) > abs(deltay))
					conv_forcedir = dir = (deltax > 0) ? right : left;
				else conv_forcedir = dir = (deltay > 0) ? down : up;
			}
			convey_forcex = deltax;
			convey_forcey = deltay;
		}
		else
		{
			bool movedx = false, movedy = false;
			if(cmb->usrflags&cflag4) //Smart corners
			{
				if(deltay<0)
				{
					info = walkflag(x,y+8-(bigHitbox*8)-2,2,up);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedy = true;
						zfix step(0);
						
						if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<(abs(deltay)*(isSideViewHero()?2:1)))
							{
								yoff=int32_t(y-step)&7;
								
								if(!yoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltay);
						}
						
						y=y-step;
						hs_starty-=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
				}
				else if(deltay>0)
				{
					info = walkflag(x,y+15+2,2,down);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedy = true;
						zfix step(0);
						
						if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltay))
							{
								yoff=int32_t(y+step)&7;
								
								if(!yoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltay);
						}
						
						y=y+step;
						hs_starty+=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
				}
				
				if(deltax<0)
				{
					info = walkflag(x-int32_t(lsteps[x.getInt()&7]),y+8-(bigHitbox ? 8 : 0),1,left);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedx = true;
						zfix step(0);
						
						if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltax))
							{
								xoff=int32_t(x-step)&7;
								
								if(!xoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltax);
						}
						
						x=x-step;
						hs_startx-=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
				}
				else if(deltax>0)
				{
					info = walkflag(x+15+2,y+8-(bigHitbox ? 8 : 0),1,right);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedx = true;
						zfix step(0);
						
						if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltax))
							{
								xoff=int32_t(x+step)&7;
								
								if(!xoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltax);
						}
						
						x=x+step;
						hs_startx+=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
				}
				if(deltax && !movedx && !deltay)
				{
					zfix oy = y;
					y = COMBOY_REGION(rpos);
					bool validpush = scr_canmove(deltax, 0, false, false);
					zfix ny = handle_movestate_zfix([&]()
					{
						movexy(deltax,0,false,false,true,true);
						return y;
					});
					y = oy;
					if (validpush || ny != COMBOY_REGION(rpos))
					{
						if (y <= ny-1)
						{
							setYfix(y+1);
						}
						else if (y >= ny+1)
						{
							setYfix(y-1);
						}
						else
						{
							setYfix(ny);
						}
					}
				}
				if(deltay && !movedy && !deltax)
				{
					zfix ox = x;
					x = COMBOX_REGION(rpos);
					bool validpush = scr_canmove(0, deltay, false, false);
					zfix nx = handle_movestate_zfix([&]()
					{
						movexy(0,deltay,false,false,true,true);
						return x;
					});
					x = ox;
					if (validpush || nx != COMBOX_REGION(rpos))
					{
						if (x <= nx-1)
						{
							setXfix(x+1);
						}
						else if (x >= nx+1)
						{
							setXfix(x-1);
						}
						else
						{
							setXfix(nx);
						}
					}
				}
			}
			if(!movedy)
			{
				if(deltay<0)
				{
					info = walkflag(x,y+8-(bigHitbox*8)-2,2,up);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedy = true;
						zfix step(0);
						
						if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<(abs(deltay)*(isSideViewHero()?2:1)))
							{
								yoff=int32_t(y-step)&7;
								
								if(!yoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltay);
						}
						
						y=y-step;
						hs_starty-=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
						movedy = true;
						zfix step(0);
						
						if((DrunkRight()||DrunkLeft())&&dir!=left&&dir!=right&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltay))
							{
								yoff=int32_t(y+step)&7;
								
								if(!yoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltay);
						}
						
						y=y+step;
						hs_starty+=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
			}
			if(!movedx)
			{
				if(deltax<0)
				{
					info = walkflag(x-int32_t(lsteps[x.getInt()&7]),y+8-(bigHitbox ? 8 : 0),1,left);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedx = true;
						zfix step(0);
						
						if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltax))
							{
								xoff=int32_t(x-step)&7;
								
								if(!xoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltax);
						}
						
						x=x-step;
						hs_startx-=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
					else checkdamagecombos(x-int32_t(lsteps[x.getInt()&7]),y+8-(bigHitbox ? 8 : 0));
				}
				else if(deltax>0)
				{
					info = walkflag(x+15+2,y+8-(bigHitbox ? 8 : 0),1,right);
					execute(info);
					
					if(!info.isUnwalkable())
					{
						movedx = true;
						zfix step(0);
						
						if((DrunkUp()||DrunkDown())&&dir!=up&&dir!=down&&!(diagonalMovement||NO_GRIDLOCK))
						{
							while(step<abs(deltax))
							{
								xoff=int32_t(x+step)&7;
								
								if(!xoff) break;
								
								step++;
							}
						}
						else
						{
							step=abs(deltax);
						}
						
						x=x+step;
						hs_startx+=step.getInt();
						
						for(int32_t j=0; j<chainlinks.Count(); j++)
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
			if(movedx || movedy)
			{
				if(cmb->usrflags&cflag1)
					is_conveyor_stunned = rate;
				if((cmb->usrflags&cflag3) && !spins)
				{
					if(abs(deltax) > abs(deltay))
						conv_forcedir = dir = (deltax > 0) ? right : left;
					else conv_forcedir = dir = (deltay > 0) ? down : up;
				}
			}
		}
	}
}

void HeroClass::setDivineProtectionShieldClk(int32_t newclk)
{
    DivineProtectionShieldClk=newclk;
    
    if(decorations.idCount(dDIVINEPROTECTIONSHIELD)==0)
    {
        decoration *dec;
        decorations.add(new dDivineProtectionShield(HeroX(), HeroY(), dDIVINEPROTECTIONSHIELD, 0));
        decorations.spr(decorations.Count()-1)->misc=0;
        decorations.add(new dDivineProtectionShield(HeroX(), HeroY(), dDIVINEPROTECTIONSHIELD, 0));
        dec=(decoration *)decorations.spr(decorations.Count()-1);
        decorations.spr(decorations.Count()-1)->misc=1;
    }
}

int32_t HeroClass::getDivineProtectionShieldClk()
{
    return DivineProtectionShieldClk;
}

int32_t HeroClass::getHoverClk()
{
    return hoverclk;
}

int32_t HeroClass::getHoldClk()
{
    return holdclk;
}

int32_t HeroClass::getLastLensID(){
	return last_lens_id;
}

void HeroClass::setLastLensID(int32_t p_item){
	last_lens_id = p_item;
}

bool HeroClass::getOnSideviewLadder()
{
	return on_sideview_ladder;
}

void HeroClass::setOnSideviewLadder(bool val)
{
	if(val)
	{
		fall = fakefall = hoverclk = jumping = 0;
		hoverflags = 0;
		inair = false;
	}
	on_sideview_ladder = val;
}

bool HeroClass::canSideviewLadder(bool down)
{
	if(!isSideViewHero()) return false;
	if(jumping < 0) return false;
	if(down && get_qr(qr_DOWN_DOESNT_GRAB_LADDERS))
	{
		bool onSolid = on_sideview_solid_oldpos(x,y,old_x,old_y,true);
		return ((isSVLadder(x+4,y+16) && (!isSVLadder(x+4,y)||onSolid)) || (isSVLadder(x+12,y+16) && (!isSVLadder(x+12,y)||onSolid)));
	}
	//Are you presently able to climb a sideview ladder?
	//x+4 / +12 are the offsets used for detecting a platform below you in sideview
	//y+0 checks your top-half for large hitbox; y+8 for small
	//y+15 checks if you are on one at all. This is necessary so you don't just fall off before reaching the top.
	//y+16 check is for going down onto a ladder you are standing on.
	return (isSVLadder(x+4,y+(bigHitbox?0:8)) || isSVLadder(x+12,y+(bigHitbox?0:8)))
		|| isSVLadder(x+4,y+15) || isSVLadder(x+12,y+15)
		|| (down && (isSVLadder(x+4,y+16) || isSVLadder(x+12,y+16)));
}

bool HeroClass::canSideviewLadderRemote(int32_t wx, int32_t wy, bool down)
{
	if(!isSideViewHero()) return false;
	if(jumping < 0) return false;
	if(down && get_qr(qr_DOWN_DOESNT_GRAB_LADDERS))
	{
		bool onSolid = on_sideview_solid_oldpos(x,y,old_x,old_y,true);
		return ((isSVLadder(wx+4,wy+16) && (!isSVLadder(wx+4,wy)||onSolid)) || (isSVLadder(wx+12,wy+16) && (!isSVLadder(wx+12,wy)||onSolid)));
	}
	//Are you presently able to climb a sideview ladder?
	//x+4 / +12 are the offsets used for detecting a platform below you in sideview
	//y+0 checks your top-half for large hitbox; y+8 for small
	//y+15 checks if you are on one at all. This is necessary so you don't just fall off before reaching the top.
	//y+16 check is for going down onto a ladder you are standing on.
	return (isSVLadder(wx+4,wy+(bigHitbox?0:8)) || isSVLadder(wx+12,wy+(bigHitbox?0:8)))
		|| isSVLadder(wx+4,wy+15) || isSVLadder(wx+12,wy+15)
		|| (down && (isSVLadder(wx+4,wy+16) || isSVLadder(wx+12,wy+16)));
}

void HeroClass::execute(HeroClass::WalkflagInfo info)
{
    int32_t flags = info.getFlags();
    
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

HeroClass::WalkflagInfo HeroClass::WalkflagInfo::operator ||(HeroClass::WalkflagInfo other)
{
    HeroClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
    
    int32_t flags1 = (flags & ~UNWALKABLE) & (other.flags & ~UNWALKABLE);
    int32_t flags2 = (flags & UNWALKABLE) | (other.flags & UNWALKABLE);
    ret.flags = flags1 | flags2;
    return ret;
}

HeroClass::WalkflagInfo HeroClass::WalkflagInfo::operator &&(HeroClass::WalkflagInfo other)
{
    HeroClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
    
    ret.flags = flags & other.flags;
    return ret;
}

HeroClass::WalkflagInfo HeroClass::WalkflagInfo::operator !()
{
    HeroClass::WalkflagInfo ret;
    ret.newhopclk = newhopclk;
    ret.newdir = newdir;
    ret.newhopdir = newhopdir;
    
    ret.flags = flags ^ UNWALKABLE;
    return ret;
}

void HeroClass::explode(int32_t type)
{
	static int32_t tempx, tempy;
	static byte herotilebuf[256];
	int32_t ltile=0;
	int32_t lflip=0;
	bool shieldModify=true;
	unpack_tile(newtilebuf, tile, flip, true);
	memcpy(herotilebuf, unpackbuf, 256);
	tempx=Hero.getX();
	tempy=Hero.getY();
	for(int32_t i=0; i<16; ++i)
	{
                for(int32_t j=0; j<16; ++j)
                {
                    if(herotilebuf[i*16+j])
                    {
                        if(type==0)  // Twilight
                        {
                            particles.add(new pTwilight(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 5, 0, 0, (zc_oldrand()%8)+i*4));
                            int32_t k=particles.Count()-1;
                            particle *p = (particles.at(k));
                            p->step=3;
                        }
                        else if(type ==1)  // Sands of Hours
                        {
                            particles.add(new pTwilight(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 5, 1, 2, (zc_oldrand()%16)+i*2));
                            int32_t k=particles.Count()-1;
                            particle *p = (particles.at(k));
                            p->step=4;
                            
                            if(zc_oldrand()%10 < 2)
                            {
                                p->color=1;
                                p->cset=0;
                            }
                        }
                        else
                        {
                            particles.add(new pDivineEscapeDust(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 5, 6, herotilebuf[i*16+j], zc_oldrand()%96));
                            
                            int32_t k=particles.Count()-1;
                            particle *p = (particles.at(k));
                            p->angular=true;
                            p->angle=zc_oldrand();
                            p->step=(((double)j)/8);
                            p->yofs=Hero.getYOfs();
                        }
                    }
                }
	}
}

void HeroClass::SetSwim()
{
	if (CanSideSwim()) 
	{
		if (action != sideswimattacking && action != attacking) {action=sideswimming; FFCore.setHeroAction(sideswimming);}
		else {action=sideswimattacking; FFCore.setHeroAction(sideswimattacking);}
		if (get_qr(qr_SIDESWIMDIR) && spins <= 0 && dir != left && dir != right) dir = sideswimdir;
	}
        else {action=swimming; FFCore.setHeroAction(swimming);}
}

void HeroClass::SetAttack()
{
	if (IsSideSwim()) {action=sideswimattacking; FFCore.setHeroAction(sideswimattacking);}
        else {action=attacking; FFCore.setHeroAction(attacking);}
}

bool HeroClass::IsSideSwim()
{
	return (action==sideswimming || action==sideswimhit || action == sideswimattacking || action == sidewaterhold1 || action == sidewaterhold2 || action == sideswimcasting || action == sideswimfreeze);
}

bool HeroClass::CanSideSwim()
{
	return (isSideViewHero() && get_qr(qr_SIDESWIM));
}

int32_t HeroClass::getTileModifier()
{
	return item_tile_mod() + bunny_tile_mod();
}
void HeroClass::setImmortal(int32_t nimmortal)
{
	immortal = nimmortal;
}
void HeroClass::kill(bool bypassFairy)
{
	dying_flags = DYING_FORCED | (bypassFairy ? DYING_NOREV : 0);
}
bool HeroClass::sideview_mode() const
{
	return isSideViewHero() && (moveflags & FLAG_OBEYS_GRAV) && !toogam;
}
bool HeroClass::is_unpushable() const
{
	return toogam;
}
