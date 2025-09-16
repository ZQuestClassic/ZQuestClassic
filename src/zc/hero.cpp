#include "base/general.h"
#include "base/handles.h"
#include "base/mapscr.h"
#include "base/util.h"
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
#include "zc/maps.h"
#include "zc/replay.h"
#include "zc/zc_ffc.h"
#include "zc/zc_subscr.h"
#include "zc/decorations.h"
#include "gamedata.h"
#include "hero_tiles.h"
#include "zc/title.h"
#include "zc/ffscript.h"
#include "drawing.h"
#include "zc/combos.h"
#include "base/zc_math.h"
#include "user_object.h"
#include "slopes.h"
#include "zc/zc_sys.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include "music_playback.h"
#include "iter.h"

extern refInfo playerScriptData;
#include "zscriptversion.h"
#include "particles.h"
#include <fmt/format.h>
#include "zc/render.h"
#include <array>

extern refInfo *ri; //= NULL;
extern int32_t(*stack)[MAX_STACK_SIZE];
extern portal mirror_portal;
using std::set;

extern int32_t skipcont;

extern int32_t draw_screen_clip_rect_x1;
extern int32_t draw_screen_clip_rect_x2;
extern int32_t draw_screen_clip_rect_y1;
extern int32_t draw_screen_clip_rect_y2;

int32_t hero_count = -1;
extern int32_t hero_animation_speed; //lower is faster animation
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

byte lsteps[8] = { 1, 1, 2, 1, 1, 2, 1, 1 };

#define CANFORCEFACEUP	(get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0 && dir!=up && (action==walking || action==none))
#define NO_GRIDLOCK		(get_qr(qr_DISABLE_4WAY_GRIDLOCK)||get_qr(qr_NEW_HERO_MOVEMENT2))
#define FIXED_Z3_ANIMATION ((zinit.heroAnimationStyle==las_zelda3||zinit.heroAnimationStyle==las_zelda3slow)&&!get_qr(qr_BROKEN_Z3_ANIMATION))

zfix HeroClass::get_standing_z_state() const
{
	if (standing_on_z < 0) // diving under stuff
		return standing_on_z;
	zfix ret = standing_on_z + z + fakez;
	if (ret < 0) // overflow?
		return STANDING_Z_MAX;
	return ret;
}

bool item_error()
{
	if(QMisc.miscsfx[sfxERROR])
		sfx(QMisc.miscsfx[sfxERROR]);
	return false;
}
static inline bool on_sideview_slope(int32_t x, int32_t y, int32_t oldx, int32_t oldy)
{
	if(check_new_slope(x, y+0.0001_zf, 16, 16, oldx, oldy, false, true) < 0) return true;
	return false;
}

static inline bool platform_fallthrough(bool doslopecheck = true)
{
	if (!getInput(btnDown, INPUT_HERO_ACTION | (get_qr(qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK) ? INPUT_DRUNK : 0)))
		return false;

	if (get_qr(qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS))
		return true;

	if (Hero.jumping < 0 && get_qr(qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS))
		return true;

	if (doslopecheck && !on_sideview_slope(Hero.x, Hero.y,Hero.old_x,Hero.old_y) && (on_sideview_slope(Hero.x,Hero.y+1,Hero.old_x,Hero.old_y) || on_sideview_slope(Hero.x, Hero.y + 2, Hero.old_x, Hero.old_y)))
		return true;

	return false;
}

static inline bool on_sideview_solid(zfix x, zfix y, bool ignoreFallthrough = false, int32_t slopesmisc = 0)
{
	if(slopesmisc != 1 && check_slope(x, y + 0.0001_zf, 16, 16, (slopesmisc == 3), true) < 0) return true;
	if(slopesmisc == 2) return false;
	if (_walkflag(x+4,y+16,1) || _walkflag(x+12,y+16,1)) return true;
	mapscr* s = get_scr_for_world_xy(x, y);
	if (y>=world_h-16 && cur_screen>=0x70 && !(s->flags2&wfDOWN)) return true;
	if (platform_fallthrough() && !ignoreFallthrough) return false;
	if(slopesmisc != 1 && check_slope(x, y + 0.0001_zf, 16, 16, false, true) < 0) return true;
	if (y.getInt() % 16 ==0 && (checkSVLadderPlatform(x+4,y+16) || checkSVLadderPlatform(x+12,y+16)))
		return true;
	return false;
}

static inline bool on_sideview_solid_oldpos(sprite* obj, bool ignoreFallthrough = false, int32_t slopesmisc = 0, zfix xofs = 0, zfix yofs = 0)
{
	zfix rx = obj->x+obj->hxofs+obj->sxofs+xofs, ry = obj->y+obj->hyofs+obj->syofs+yofs,
	rw = obj->hit_width+obj->sxsz_ofs, rh = obj->hit_height+obj->sysz_ofs,
	orx = obj->old_x+obj->hxofs+obj->sxofs, ory = obj->old_y+obj->hyofs+obj->syofs,
	x = obj->x+xofs, y = obj->y+yofs;
	if(slopesmisc != 1 && check_new_slope(rx, ry+0.0001_zf, rw, rh, orx, ory, (slopesmisc == 3), true, obj->slopeid) < 0)
		return true;
	if(slopesmisc == 2)
		return false;
	if (_walkflag(x+4,y+16,1) || _walkflag(x+12,y+16,1))
		return true;
	if (y>=world_h-16 && cur_screen>=0x70 && !(hero_scr->flags2&wfDOWN))
		return true;
	if (platform_fallthrough() && !ignoreFallthrough) return false;
	if (slopesmisc != 1 && check_new_slope(rx, ry+0.0001_zf, rw, rh, orx, ory, false, true, obj->slopeid) < 0)
		return true;
	if (y.getInt()%16==0 && (checkSVLadderPlatform(x+4,y+16) || checkSVLadderPlatform(x+12,y+16)))
		return true;
	return false;
}

static inline bool no_plat_action()
{
	if(Hero.fallclk || Hero.drownclk || walk_through_walls || Hero.getOnSideviewLadder())
		return true;
	switch(Hero.action)
	{
		case freeze: case rafting: case inwind: case scrolling: case won: case hopping:
		case climbcovertop: case climbcoverbottom: case dying: case drowning:
		case falling: case lavadrowning: case sideswimfreeze: case sidedrowning:
			return true;
	}
	return false;
}
bool HeroClass::on_ffc_platform(ffcdata const& ffc, bool old)
{
	if(no_plat_action())
		return false;
	if(!get_qr(qr_MULTI_PLATFORM_FFC))
	{
		if(platform_ffc && &ffc != platform_ffc)
			return false;
		platform_ffc = nullptr;
	}
	if(sideview_mode())
	{
		if((ffc.flags & (ffc_solid|ffc_platform|ffc_changer)) != (ffc_solid|ffc_platform))
			return false;
		zfix fx = old ? ffc.old_x : ffc.x, fy = old ? ffc.old_y : ffc.y;
		static const zfix tol = 0.5_zf;
		if((y+16-fy).getAbs() > tol)
			return false;
		if(fx > x+12)
			return false;
		if(fx+ffc.hit_width <= x+4)
			return false;
	}
	else
	{
		if((ffc.flags & (ffc_platform|ffc_changer)) != ffc_platform)
			return false;
		if(z)
			return false;
		static const int tol = 3;
		if(!(old
			? ffc.collide_old(x + 8-tol, y + (bigHitbox ? 8 : 12), tol*2, tol*2)
			: ffc.collide(x + 8-tol, y + (bigHitbox ? 8 : 12)-tol, tol*2, tol*2)))
			return false;
	}
	platform_ffc = &ffc;
	return true;
}
bool HeroClass::on_ffc_platform()
{
	if(no_plat_action())
		return false;
	if(platform_ffc && on_ffc_platform(*platform_ffc,false))
		return true;
	bool any_on_platform = find_ffc([&](const ffc_handle_t& ffc_handle) {
		return on_ffc_platform(*ffc_handle.ffc, false);
	}).has_value();
	return any_on_platform;
}

void HeroClass::check_platform_ffc()
{
	if(platform_ffc && !on_ffc_platform(*platform_ffc,false))
	{
		clear_platform_ffc();
		on_ffc_platform();
	}
}
void HeroClass::clear_platform_ffc()
{
	platform_ffc = nullptr;
}

void HeroClass::snap_platform()
{
	if(check_new_slope(x, y+1, 16, 16, old_x, old_y, false, true) < 0)
		return;
	if (y>=world_h-16 && cur_screen>=0x70 && !(hero_scr->flags2&wfDOWN))
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
	if (Hero.active_shield_id != itmid && Hero.on_cooldown(itmid)) return false;
	auto const& itm = itemsbuf[itmid];
	if(item_disabled(itmid)) return false;
	if(!checkitem_jinx(itmid)) return false;
	if(!(itm.flags & item_flag9)) return false;
	if(!isItmPressed(itmid))
	{
		byte intbtn = byte(itm.misc5&0xFF);
		if(!itm.misc5 || !getIntBtnInput(intbtn, INPUT_DRUNK | INPUT_PEEK))
			return false;
	}
	return (checkbunny(itmid) && checkmagiccost(itmid));
}
int32_t getCurrentShield(bool requireActive)
{
	if(Hero.shieldjinxclk) return -1;
	if(Hero.active_shield_id > -1 && usingActiveShield(Hero.active_shield_id))
		return Hero.active_shield_id;
	if(!requireActive) return current_item_id(itype_shield,false,true);
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
	if (getInput(btnB, INPUT_DRUNK | INPUT_HERO_ACTION))
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Bwpn, 0xFFF)];
		if (dat.type == itype_shield && (dat.flags & item_flag9))
		{
			id = NEG_OR_MASK(Bwpn, 0xFFF);
		}
	}
	if (id < 0 && getInput(btnA, INPUT_DRUNK | INPUT_HERO_ACTION))
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Awpn, 0xFFF)];
		if (dat.type == itype_shield && (dat.flags & item_flag9))
		{
			id = NEG_OR_MASK(Awpn, 0xFFF);
		}
	}
	if (id < 0 && getInput(btnEx1, INPUT_DRUNK | INPUT_HERO_ACTION))
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Xwpn, 0xFFF)];
		if (dat.type == itype_shield && (dat.flags & item_flag9))
		{
			id = NEG_OR_MASK(Xwpn, 0xFFF);
		}
	}
	if (id < 0 && getInput(btnEx2, INPUT_DRUNK | INPUT_HERO_ACTION))
	{
		itemdata const& dat = itemsbuf[NEG_OR_MASK(Ywpn, 0xFFF)];
		if (dat.type == itype_shield && (dat.flags & item_flag9))
		{
			id = NEG_OR_MASK(Ywpn, 0xFFF);
		}
	}
	if (id < 0)
	{
		auto shield_id = current_item_id(itype_shield, false, true);
		itemdata const& dat = itemsbuf[shield_id];
		if (dat.type == itype_shield && (dat.flags & item_flag9))
		{
			byte intbtn = byte(dat.misc5 & 0xFF);
			if (getIntBtnInput(intbtn, INPUT_DRUNK | INPUT_PEEK))
				id = shield_id;
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
		if(shield.flags & item_flag10) //Change Speed flag
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
	if(isSideViewHero() && !on_sideview_solid_oldpos(this)
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
	SETFLAG(liftflags, LIFTFL_DIS_SWIMMING, !(itm.flags & item_flag2));
	SETFLAG(liftflags, LIFTFL_DIS_SHIELD, itm.flags & item_flag3);
	SETFLAG(liftflags, LIFTFL_DIS_ITEMS, itm.flags & item_flag4);
	SETFLAG(liftflags, LIFTFL_DROP_ON_HIT, itm.flags & item_flag5);
}

int32_t get_grav_boots_id()
{
	if (Hero.last_grav_boots_id > -1)
		return Hero.last_grav_boots_id;;
	int32_t grav_id;
	if (Hero.sideview_mode())
	{
		if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			if ((grav_id = current_item_id(itype_gravity_down_boots, true)) >= 0)
				if (checkmagiccost(grav_id) && checkbunny(grav_id))
					return (Hero.last_grav_boots_id = grav_id);
		}
		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			if ((grav_id = current_item_id(itype_gravity_up_boots, true)) >= 0)
				if (checkmagiccost(grav_id) && checkbunny(grav_id))
					return (Hero.last_grav_boots_id = grav_id);
		}
	}
	if ((grav_id = current_item_id(itype_gravity_boots, true)) >= 0)
		if (checkmagiccost(grav_id) && checkbunny(grav_id))
			if (!(itemsbuf[grav_id].flags & item_flag1) || Hero.sideview_mode()) // sideview-only boots only work in sideview
				return (Hero.last_grav_boots_id = grav_id);
	return -1;
}
zfix HeroClass::get_gravity(bool skip_custom) const
{
	if (custom_gravity && !skip_custom)
		return custom_gravity;
	if (last_rocs_id != -1)
	{
		itemdata const& itm = itemsbuf[last_rocs_id];
		if (itm.flags & item_flag2)
		{
			if ((!(itm.flags & item_flag3) || fall < 0) &&
				(!(itm.flags & item_flag4) || fall > 0))
				return zslongToFix(itm.misc3 * 100);
		}
	}
	int32_t grav_id = get_grav_boots_id();
	if (grav_id > -1)
		return zslongToFix(itemsbuf[grav_id].misc1);
	return sprite::get_gravity(skip_custom);
}
zfix HeroClass::get_terminalv(bool skip_custom) const
{
	if (custom_terminal_v && !skip_custom)
		return custom_terminal_v;
	if (last_rocs_id != -1)
	{
		itemdata const& itm = itemsbuf[last_rocs_id];
		if (itm.flags & item_flag5)
			return zslongToFix(itm.misc4 * 100);
	}
	int32_t grav_id = get_grav_boots_id();
	if (grav_id >= 0)
		return zslongToFix(itemsbuf[grav_id].misc2);
	return sprite::get_terminalv(skip_custom);
}

void HeroClass::update_current_screen()
{
	int cx = x + 8, cy = y + 8;
	if (replay_version_check(0, 43)) // hero_screen previously used top-left, not center
	{
		cx = x;
		cy = y;
	}
	
	int scr_x = vbound(cx, 0, world_w - 1) / 256;
	int scr_y = vbound(cy, 0, world_h - 1) / 176;
	int new_screen = cur_screen + scr_x + scr_y * 16;
	if (maze_state.active == 1)
		new_screen = maze_state.scr->screen;
	if (current_screen != new_screen && scr_x >= 0 && scr_y >= 0 && scr_x < 16 && scr_y < 8 && is_in_current_region(new_screen))
	{
		region_scr_dx = scr_x;
		region_scr_dy = scr_y;
		current_screen = new_screen;
		prev_hero_scr = hero_scr;
		hero_scr = get_scr(current_screen);
		screen_spawned = current_screen;
		playLevelMusic();
	}
	if (game->get_regionmapping() == REGION_MAPPING_PHYSICAL)
		mark_visited(new_screen); // Mark each screen the hero steps foot in as visited
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
			if(cur_screen >= 0x80) break;
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
			if (onWater(false)) break;
			
			rpos_t rposes[] = {
				COMBOPOS_REGION_B(x,y+(bigHitbox?0:8)),
				COMBOPOS_REGION_B(x,y+15),
				COMBOPOS_REGION_B(x+15,y+(bigHitbox?0:8)),
				COMBOPOS_REGION_B(x+15,y+15)
				};
			for(auto rpos : rposes)
			{
				if (rpos == rpos_t::None)
					continue;

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
		respawn_scr = cur_screen;
		respawn_dmap = cur_dmap;
	}
	while(false); //run once, but 'break' works
	
	x = oldx;
	y = oldy;
}

void HeroClass::clear_ice()
{
	ice_vx = ice_vy = 0;
	ice_entry_count = ice_entry_mcount = 0;
	ice_combo = 0;
	sliding = 0;
}
void HeroClass::force_ice_velocity(optional<zfix> vx, optional<zfix> vy)
{
	auto ic = ice_combo;
	if(script_ice_combo)
	{
		if((unsigned(script_ice_combo) < MAXCOMBOS) && check_icy(combobuf[script_ice_combo], ICY_PLAYER))
			ic = script_ice_combo;
		else ic = 0;
	}
	if(!ic) return;
	
	auto const& icecmb = combobuf[ic];
	if(script_ice_combo && ic && !sliding)
	{
		//Special case to prevent the forced vx/vy from being overwritten
		sliding = 1;
		ice_vx = ice_vy = 0;
		ice_entry_count = ice_entry_mcount = icecmb.attribytes[1];
	}
	zfix cap = zslongToFix(zc_max(1,abs(icecmb.attributes[2])));
	if(vx) ice_vx = vbound(*vx, -cap, cap);
	if(vy) ice_vy = vbound(*vy, -cap, cap);
}
void HeroClass::go_respawn_point()
{
	x = respawn_x;
	y = respawn_y;
	clear_ice();
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
	
	if(cur_dmap != respawn_dmap || cur_screen != respawn_scr)
	{
		FFCore.warp_player(wtIWARP, respawn_dmap, respawn_scr-DMaps[cur_dmap].xoff,
			-1, -1, 0, 0, warpFlagNOSTEPFORWARD, -1);
	}
}

void HeroClass::trySideviewLadder()
{
	if(canSideviewLadder() && !on_sideview_solid_oldpos(this))
		setOnSideviewLadder(true);
}

bool HeroClass::can_pitfall(bool ignore_hover)
{
	return !(
		isSideViewGravity()
		|| action==rafting
		|| z>0 || fakez>0 || fall<0 || fakefall<0
		|| (hoverclk && !ignore_hover)
		|| inlikelike || inwallm || pull_hero || walk_through_walls
		|| (ladderx||laddery) || getOnSideviewLadder()
		|| drownclk || !(moveflags & move_can_pitfall)
		|| platform_ffc);
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
	uid = 1; // hardcoded hero uid
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

void HeroClass::sethitHeroUID(int32_t type, int32_t screen)
{
	lastHitBy[type][0] = screen;
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


void HeroClass::herostep()
{
    lstep = lstep<((zinit.heroAnimationStyle==las_bszelda)?27:11) ? lstep+1 : 0;
	//need to run all global/hero/dmap scripts here?
}

bool is_moving()
{
	return getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION);
}

// called by ALLOFF()
void HeroClass::resetflags(bool all)
{
    refilling=REFILL_NONE;
    inwallm=false;
    inlikelike=blowcnt=whirlwind=specialcave=hclk=fairyclk=refill_why=didstuff=0;
	usecounts.clear();
	item_cooldown.fill(0);
    
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
    drownclk=drownCombo=0;
	set_dive(0);
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
    getInput(btnA, INPUT_PRESS | INPUT_HERO_ACTION);
    getInput(btnB, INPUT_PRESS | INPUT_HERO_ACTION);
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
zfix  HeroClass::getJump()
{
    return fall / -100;
}
zfix  HeroClass::getFakeFall()
{
    return fakefall;
}
zfix  HeroClass::getFakeJump()
{
    return fakefall / -100;
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
        is_on_conveyor = -1;
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
        is_on_conveyor = -1;
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
            set_dive(0);
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
            set_dive(0);
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
        is_on_conveyor = -1;
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
        is_on_conveyor = -1;
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
            set_dive(0);
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
            set_dive(0);
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
	fall = new_fall;
	justmoved = 2;
	if (!jumping && fall || !get_qr(qr_BETTER_PLAYER_JUMP_ANIM))
		jumping = -1;
}
void HeroClass::setFakeFall(zfix new_fall)
{
	fakefall = new_fall;
	if (!jumping && fakefall || !get_qr(qr_BETTER_PLAYER_JUMP_ANIM))
		jumping = -1;
}
void HeroClass::setJump(zfix new_jump)
{
	setFall(new_jump*-100);
}
void HeroClass::setFakeJump(zfix new_jump)
{
	setFakeFall(new_jump*-100);
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
int32_t  HeroClass::getShieldClk()
{
	return shieldjinxclk;
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
void HeroClass::setShieldClk(int32_t newclk)
{
	shieldjinxclk=newclk;
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
int32_t HeroClass::getFlashingCSet()
{
	int32_t temp_cs = 6;
	if (script_hero_cset > -1) temp_cs = script_hero_cset;
	if (!get_qr(qr_HEROFLICKER))
	{
		if (superman && getCanFlicker())
		{
			temp_cs += (((~frame) >> 1) & 3);
		}
		else if (hclk && (DivineProtectionShieldClk <= 0) && getCanFlicker())
		{
			temp_cs += ((hclk >> 1) & 3);
		}
	}
	return temp_cs;
}
bool HeroClass::is_hitflickerframe()
{
	if (!(get_qr(qr_HEROFLICKER) && (superman || hclk)))
		return false;

	int32_t fr = game->get_spriteflickerspeed();
	if (fr == 0)
		return true;
	return frame % (fr * 2) >= fr;
}
int32_t  HeroClass::getAction() // Used by ZScript
{
    if(spins > 0)
        return isspinning;
    else if(charging > 0)
        return ischarging;
    else if(diveclk)
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
    else if(diveclk)
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
	
	
	if(magicitem>-1 && itemsbuf[magicitem].type==itype_divineescape)
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
		if(action==swimming && !diveclk)
		{
			int32_t flippers_id = current_item_id(itype_flippers);
			set_dive(flippers_id < 0 ? 50 : itemsbuf[flippers_id].misc1);
		}
		return;
		
	case drowning:
	case sidedrowning:
		clear_platform_ffc();
		if(!drownclk)
			Drown();
			
		break;
	
	case lavadrowning:
		//Lavadrowning is just drowning but with a different argument. Simplicity! -Dimi
		clear_platform_ffc();
		if(!drownclk)
			Drown(1);
			
		break;
		
	case falling:
		clear_platform_ffc();
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
		check_on_hit();
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
    return diveclk > 0;
}
bool HeroClass::isSwimming()
{
	bool include_swimhit = true;
	if (replay_is_active() && replay_version_check(0, 31))
	{
		std::string qst = replay_get_meta_str("qst");
		bool disable = false;
		disable |= qst == "demons_inferno.qst";
		disable |= qst == "demosp253.qst";
		disable |= qst == "hell_awaits.qst";
		disable |= qst == "lands_of_serenity.qst";
		disable |= qst == "link_to_the_heavens.qst";
		// The others are just a gfx change, but this one actual breaks without this compat check.
		disable |= qst == "link_to_the_zelda.qst";
		include_swimhit = !disable;
	}
    return ((action==swimming)||(action==sideswimming)||IsSideSwim()||
            (action==waterhold1)||(action==waterhold2)||(include_swimhit && action==swimhit)||
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
	cache_tile_mod_clear();
	usecounts.clear();
	item_cooldown.fill(0);
	scale = 0;
	rotation = 0;
	do_animation = true;
	if(lift_wpn)
	{
		delete lift_wpn;
		lift_wpn = nullptr;
	}
	clear_platform_ffc();
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
    moveflags = move_obeys_grav | move_can_pitfall | move_can_waterdrown;
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
    
	if (hero_scr)
	{
		if(get_qr(qr_NOARRIVALPOINT))
		{
			x=hero_scr->warpreturnx[0];
			y=hero_scr->warpreturny[0];
		}
		else
		{
			x=hero_scr->warparrivalx;
			y=hero_scr->warparrivaly;
		}
	}
    
    z=fakez=fall=fakefall=0;
    hzsz = 12; // So that flying peahats can still hit him.
    
    if(x==0)   dir=right;
    
    if(x==world_w-16) dir=left;
    
    if(y==0)   dir=down;
    
    if(y==world_h-16) dir=up;

	if (hero_scr)
	{
		x += region_scr_dx*256;
		y += region_scr_dy*176;
		update_viewport();
	}

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
	flickercolor=-1;
	flickertransp=-1;
    pushing=fairyclk=0;
    id=0;
    inlikelike=0;
    superman=inwallm=false;
    scriptcoldet=true;
    blowcnt=whirlwind=specialcave=0;
    hopclk=fallclk=0;
	set_dive(0);
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
    stepoutindex=stepoutwr=stepoutdmap=stepoutscreen=0;
    stepnext=stepsecret=rpos_t::None;
    ffpit = false;
    respawn_x=x;
    respawn_y=y;
	respawn_dmap=cur_dmap;
	respawn_scr=cur_screen;
    falling_oldy = y;
    magiccastclk=0;
    magicitem = div_prot_item = -1;
	last_lens_id = 0; //Should be -1 (-Z)
	last_grav_boots_id = last_rocs_id = current_rocs_jump_id = -1;
	released_jump_button = false;
	last_lift_id.reset();
	last_savepoint_id = 0;
	misc_internal_hero_flags = 0;
	last_cane_of_byrna_item_id = -1;
	on_sideview_ladder = false;
	standing_on_z = 0;
	standing_z_offset = false;
	extra_jump_count = 0;
	hoverflags = 0;
    lbunnyclock = 0;
	lamp_paid = false;
	autowalk_combo_id = -1;
	autowalk_handle = {};
    
    for(int32_t i=0; i<32; i++) miscellaneous[i] = 0;
    
    setBigHitbox(get_qr(qr_LTTPCOLLISION));
    diagonalMovement=(get_qr(qr_LTTPWALK));
    
	shield_active = false;
	shield_forcedir = -1;
	active_shield_id = -1;
	conv_forcedir = -1;
	
	preventsubscreenfalling = false;  //-Z
	walkspeed = 0; //not used, yet. -Z
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][0] = 0; 
	for ( int32_t q = 0; q < NUM_HIT_TYPES_USED; q++ ) lastHitBy[q][1] = 0; 
	for ( int32_t q = 0; q < wMax; q++ ) 
	{
		defence[q] = hero_defenses[q]; //we will need to have a Hero section in the quest load/save code! -Z Added 3/26/21 - Jman
	}
	
	clear_ice();
	script_ice_combo = 0;
	//Run script!
	if (( FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) && (game->get_hasplayed()) ) //if (!hasplayed) runs in game_loop()
	{
		ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_INIT); 
		FFCore.deallocateAllScriptOwned(ScriptType::Hero, SCRIPT_HERO_INIT);
		FFCore.initZScriptHeroScripts(); //Clear the stack and the refinfo data to be ready for Hero's active script. 
		set_respawn_point(); //screen entry at spawn; //This should be after the init script, so that Hero->X and Hero->Y set by the script
						//are properly set by the engine.
	}
	FFCore.nostepforward = 0;

	for (int i = 0; i < 4; i++)
		lastdir[i] = 0xFF;

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
		int screen = currentscroll;
		if(screen < 0)
		{
			screen = current_item_id(attack==wHammer ? (spins>1?itype_quakescroll2:itype_quakescroll)
				: (spins>5 || current_item_id(itype_spinscroll) < 0)
					? itype_spinscroll2 : itype_spinscroll);
		}
		power *= itemsbuf[screen].power;
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
	int32_t clock = (itemsbuf[itemid].flags&item_flag2 ? (NET_CLK_TOTAL-1)-attackclk : attackclk);
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
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & item_flag4 && charging==0)
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
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & item_flag4 && charging==0)
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
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & item_flag4 && charging==0)
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
        
        if(game->get_canslash() && w->id==wSword && itemsbuf[itemid].flags & item_flag4 && charging==0)
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
    
    if(game->get_canslash() && itemsbuf[itemid].flags & item_flag4 && attackclk<11)
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
            
            if(spins>0 || (itemsbuf[itemid].flags & item_flag8))
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
                
                if(spins>0 || (itemsbuf[itemid].flags & item_flag8))
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
	int32_t oxofs = xofs, oyofs = yofs;
	bool invisible=(dontdraw>0) || (hero_scr->flags3&fINVISHERO);
	
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
			yofs = oyofs-((!BSZ && isdungeon() && cur_screen<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
			
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
			
			cs = getFlashingCSet();
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
			int32_t itemid = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
						// TODO(crash): check that .add succeeds.
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
					
					if(((game->get_canslash() && (attack==wSword || attack==wWand || attack==wFire || attack==wCByrna)) && itemsbuf[itemid].flags&item_flag4 && (attackclk<7||FIXED_Z3_ANIMATION&&(attackclk < 16))))
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
						if ((dir == right && (itemsbuf[itemid].flags&item_flag2)) || (dir != right && !(itemsbuf[itemid].flags&item_flag2)))
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
					if (is_hitflickerframe())
					{
						int32_t temp_flicker_color = (game->get_life() > 0 || immortal) ? (flickercolor < 0 ? game->get_spriteflickercolor() : flickercolor) : 0;
						if((game->get_spriteflickercolor() || temp_flicker_color) && !superman)
						{
							sprite_flicker_transp_passes = (flickertransp < 0 ? game->get_spriteflickertransp() : flickertransp);
							sprite_flicker_color = temp_flicker_color;
							masked_draw(dest);
						}
					}
					else
						masked_draw(dest);

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
				
				if(BSZ || ((isdungeon() && cur_screen<128) && !get_qr(qr_HERODUNGEONPOSFIX)))
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
					sfx(((iswaterex_z3(MAPCOMBO(x+wx+8,y+wy), -1, x+wx+8, y+wy, true) || COMBOTYPE(x+wx+8,y+wy)==cSHALLOWWATER) && get_qr(qr_MORESOUNDS)) ? WAV_ZN1SPLASH : itemsbuf[itemid].usesound,pan(x));
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
			bool inwater = iswaterex_z3(MAPCOMBO(x+4,y+9), -1, x+4, y+9, true, false)  && iswaterex_z3(MAPCOMBO(x+4,y+15), -1, x+4, y+15, true, false) && iswaterex_z3(MAPCOMBO(x+11,y+9), -1, x+11, y+9, true, false) && iswaterex_z3(MAPCOMBO(x+11,y+15), -1, x+11, y+15, true, false);

			optional<uint32_t> jumping_frame;

			if (get_qr(qr_BETTER_PLAYER_JUMP_ANIM))
			{
				if (jumping && fall)
					jumping_frame = (jumping / 6) % 3;
			}
			else
			{
				auto val = int32_t(jumping * (get_grav_fall() / 16.0));
				if (val > 0 && val < 24)
					jumping_frame = val / 8;
			}
			
			bool noliftspr = get_qr(qr_NO_LIFT_SPRITE);
			bool advancetile = script_hero_sprite <= 0;
			switch(zinit.heroAnimationStyle)
			{
			case las_original:                                               //normal
				if(action==drowning)
				{
					if(inwater)
					{
						herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_drown, dir, zinit.heroAnimationStyle);
						if(advancetile) tile+=((frame>>3) & 1)*(extend==2?2:1);
					}
					else
					{
						goto herodraw_end;
					}
				}
				else if(action==lavadrowning)
				{
					herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_lavadrown, dir, zinit.heroAnimationStyle);
					if(advancetile) tile+=((frame>>3) & 1)*(extend==2?2:1);
				}
				else if(action==sidedrowning)
				{
					herotile(&tile, &flip, &extend, ls_sidedrown, down, zinit.heroAnimationStyle);
					if(advancetile) tile+=((frame>>3) & 1)*(extend==2?2:1);
				}
				else if (action == sideswimming || action == sideswimhit)
				{
					herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if(advancetile) ++flip;
						}
						else
						{
							if(advancetile) extend==2?tile+=2:++tile;
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
							if(advancetile) ++flip;
						}
						else
						{
							if(advancetile) extend==2?tile+=2:++tile;
						}
					}
					
					if(isDiving())
					{
						herotile(&tile, &flip, &extend, ls_dive, dir, zinit.heroAnimationStyle);
						if(advancetile) tile+=((frame>>3) & 1)*(extend==2?2:1);
					}
				}
				else if(charging > 0 && attack != wHammer)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimcharge:ls_charge, dir, zinit.heroAnimationStyle);
					
					if(lstep>=6)
					{
						if(dir==up)
						{
							if(advancetile) ++flip;
						}
						else
						{
							if(advancetile) extend==2?tile+=2:++tile;
						}
					}
				}
				else if ((z > 0 || fakez > 0 || isSideViewHero()) && jumping_frame && game->get_life() > 0 && action != rafting)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if (advancetile) tile += (*jumping_frame) * (extend == 2 ? 2 : 1);
				}
				else if(fallclk>0)
				{
					herotile(&tile, &flip, &extend, ls_falling, dir, zinit.heroAnimationStyle);
					if(advancetile) tile+=((PITFALL_FALL_FRAMES-fallclk)/10)*(extend==2?2:1);
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
					else
					{
						if(!noliftspr&&isLifting())
							herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
						else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					}
					
					if(dir>up)
					{
						useltm=true;
					}
					
					if(advancetile && lstep>=6)
					{
						if(dir==up)
						{
							++flip;
						}
						else
						{
							extend==2?tile+=2:++tile;
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
						if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
					else
					{
						goto herodraw_end;
					}
				}
				else if (action == sidedrowning)
				{
					herotile(&tile, &flip, &extend, ls_sidedrown, down, zinit.heroAnimationStyle);
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if(action==lavadrowning)
				{
					herotile(&tile, &flip, &extend, (drownclk > 60) ? ls_float : ls_lavadrown, dir, zinit.heroAnimationStyle);
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if (action == sideswimming || action == sideswimhit)
				{
					herotile(&tile, &flip, &extend, ls_sideswim, dir, zinit.heroAnimationStyle);
					
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
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
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					
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
						if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
					}
				}
				else if(charging > 0 && attack != wHammer)
				{
					herotile(&tile, &flip, &extend, (IsSideSwim())?ls_sideswimcharge:ls_charge, dir, zinit.heroAnimationStyle);
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
				}
				else if ((z > 0 || fakez > 0 || isSideViewHero()) && jumping_frame && game->get_life() > 0)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if (advancetile) tile += (*jumping_frame) * (extend == 2 ? 2 : 1);
				}
				else if(fallclk>0)
				{
					herotile(&tile, &flip, &extend, ls_falling, dir, zinit.heroAnimationStyle);
					if(advancetile) tile += ((PITFALL_FALL_FRAMES-fallclk)/10)*(extend==2?2:1);
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
					else
					{
						if(!noliftspr&&isLifting())
							herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
						else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					}
					
					if(dir > up)
					{
						useltm=true;
					}
					if(advancetile) tile += anim_3_4(lstep,7)*(extend==2?2:1);
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
				else if ((z > 0 || fakez > 0 || isSideViewHero()) && jumping_frame && game->get_life() > 0)
				{
					herotile(&tile, &flip, &extend, ls_jump, dir, zinit.heroAnimationStyle);
					if (script_hero_sprite <= 0) tile += (*jumping_frame) * (extend == 2 ? 2 : 1);
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
					else
					{
						if(!noliftspr&&isLifting())
							herotile(&tile, &flip, &extend, ls_liftwalk, dir, zinit.heroAnimationStyle);
						else herotile(&tile, &flip, &extend, ls_walk, dir, zinit.heroAnimationStyle);
					}
					
					if(advancetile && (action == walking || action == climbcoverbottom || action == climbcovertop))
					{
						tile += (extend == 2 ? 2 : 1);
					}
					
					if(dir>up)
					{
						useltm=true;
					}
					
					if(advancetile && (action == walking || action == hopping || action == climbcoverbottom || action == climbcovertop))
					{
						//tile+=(extend==2?2:1);
						//tile+=(((active_count>>2)%8)*(extend==2?2:1));
						int32_t l = hero_count / hero_animation_speed;
						l -= ((l > 3) ? 1 : 0) + ((l > 12) ? 1 : 0);
						tile += (l / 2) * (extend == 2 ? 2 : 1);
					}
				}
				
				break;
				
			default:
				break;
			}
		}
		
		yofs = oyofs-((!BSZ && isdungeon() && cur_screen<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
		
		if(action==won)
		{
			yofs=(get_qr(qr_OLD_DRAWOFFSET)?playing_field_offset:original_playing_field_offset) - 2;
		}
		
		if(action==landhold1 || action==landhold2)
		{
			useltm=(get_qr(qr_EXPANDEDLTM) != 0);
			yofs = oyofs-((!BSZ && isdungeon() && cur_screen<128 && !get_qr(qr_HERODUNGEONPOSFIX)) ? 2 : 0);
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
		
		if(is_hitflickerframe())
		{
			int32_t temp_flicker_color = (game->get_life() > 0 || immortal) ? (flickercolor < 0 ? game->get_spriteflickercolor() : flickercolor) : 0;
			if((game->get_spriteflickercolor() || temp_flicker_color) && !superman)
			{
				sprite_flicker_transp_passes = (flickertransp < 0 ? game->get_spriteflickertransp() : flickertransp);
				sprite_flicker_color = temp_flicker_color;
				masked_draw(dest);
			}
		}
		else
			masked_draw(dest);
		
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
	do_primitives(dest, SPLAYER_PLAYER_DRAW);
}

void HeroClass::masked_draw(BITMAP* dest)
{
	// The first sprite::draw in this function uses sprite_flicker_color
	// This is intended to be the player, handle this if this changes. -Moosh
	zfix lz, lfz;
	if(lift_wpn)
	{
		lz = lift_wpn->z;
		lfz = lift_wpn->fakez;
	}
	
	if(isdungeon() && cur_screen<128 && (x<16 || x>(world_w-32) || y<18 || y>(world_h-30)) && !get_qr(qr_FREEFORM))
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
	if(combobuf[prompt_combo].animflags & AF_EDITOR_ONLY) return;
	overcombo(dest, sx - viewport.x, sy - viewport.y, prompt_combo, prompt_cset);
	return;
}

void collectitem_script(int32_t id)
{
	if(itemsbuf[id].collect_script)
	{
		//clear item script stack. 
		FFCore.clear_ref(ScriptType::Item, -id);

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
	if(itemsbuf[id].script && ( (itemsbuf[id].flags&item_passive_script) && (get_qr(qr_ITEMSCRIPTSKEEPRUNNING)) ))
	{
		FFCore.reset_script_engine_data(ScriptType::Item, id);
		
		if(get_qr(qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST)
			&& current_item(itemsbuf[id].type) > itemsbuf[id].level)
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
	int32_t itemid = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
	itemid = vbound(itemid, 0, MAXITEMS-1);
	
	// The sword offsets aren't based on anything other than what felt about right
	// compared to the NES game and what mostly kept it from hitting things that
	// should clearly be out of range. They could probably still use more tweaking.
	// Don't use 2.10 for reference; it's pretty far off.
	// - Saf
	
	if(game->get_canslash() && (attack==wSword || attack==wWand) && itemsbuf[itemid].flags & item_flag4)
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
		if((guys.spr(i)->hit(wx,wy,wz,wxsz,wysz,wz>0?16:8) && ((attack!=wWand && attack!=wHammer && attack!=wCByrna) || !(itemsbuf[itemid].flags & item_flag3)))
				|| ((attack==wWand || attack==wCByrna) && guys.spr(i)->hit(wx,wy-8,z,16,24,z>8) && !(itemsbuf[itemid].flags & item_flag3))
				|| (attack==wHammer && guys.spr(i)->hit(wx,wy-8,z,16,24,z>0?16:8) && !(itemsbuf[itemid].flags & item_flag3)))
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
			if (h < 0)
			{ 
				e->hitby[HIT_BY_LWEAPON] = melee_weapon_index; 
				e->hitby[HIT_BY_LWEAPON_UID] = w->getUID();
				e->hitby[HIT_BY_LWEAPON_TYPE] = w->id;
				if (w->parentitem > -1) e->hitby[HIT_BY_LWEAPON_PARENT_FAMILY] = itemsbuf[w->parentitem].type; 
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
			
			if(abs(h)==2)
				break;
		}
	}
		
	if(attack == wBugNet
		|| (parentitem==-1&&!get_qr(qr_NOITEMMELEE))
		|| (parentitem>-1&&!(itemsbuf[parentitem].flags & item_flag7)))
	{
		int32_t bugnetid = attack != wBugNet ? -1 : (parentitem > -1 ? parentitem : current_item_id(itype_bugnet));
		for(int32_t j=0; j<items.Count(); j++)
		{
			item* ptr = (item*)items.spr(j);
			bool dofairy = (attack==wBugNet && itemsbuf[ptr->id].type == itype_fairy)
				&& (bugnetid > -1 && !(itemsbuf[bugnetid].flags & item_flag1));
			
			if((itemsbuf[ptr->id].type == itype_bottlefill || dofairy) && !game->canFillBottle())
				continue; //No picking these up unless you have a bottle to fill!
			if((ptr->pickup & ipCANGRAB) || (ptr->pickup & ipTIMER) || dofairy)
			{
				if(((ptr->pickup & ipCANGRAB) || ptr->clk2 >= game->get_item_spawn_flicker() || dofairy) && !ptr->fallclk && !ptr->drownclk)
				{
					if(ptr->hit(wx,wy,z,wxsz,wysz,1) || (attack==wWand && ptr->hit(x,y-8-fakez,z,wxsz,wysz,1))
							|| (attack==wHammer && ptr->hit(x,y-8-fakez,z,wxsz,wysz,1)))
					{
						int screen = get_screen_for_world_xy(wx, wy);
						mapscr* scr = get_scr_for_world_xy(wx, wy);
						int32_t pickup = ptr->pickup;
						int32_t id2 = ptr->id;
						int32_t pstr = ptr->pstring;
						int32_t pstr_flags = ptr->pickup_string_flags;
						int32_t pstr_screen = ptr->screen_spawned;
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
							setmapflag(scr, mITEM);
						else if(pickup&ipONETIME2) // set mSPECIALITEM flag for other one-time-only items
							setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
						
						if(ptr->pickupexstate > -1 && ptr->pickupexstate < 32)
							setxmapflag(screen, 1<<ptr->pickupexstate);
						if(pickup&ipSECRETS)								// Trigger secrets if this item has the secret pickup
						{
							if (scr->flags9&fITEMSECRETPERM) setmapflag(scr, mSECRET);
							trigger_secrets_for_screen(TriggerSource::ItemsSecret, scr, false);
						}
						//!DIMI
						
						if(dofairy)
						{
							game->fillBottle(itemsbuf[ptr->id].misc4);
						}
						else
						{
							collectitem_script(id2);
							
							getitem(id2, ptr->noSound, true);
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

			for_every_ffc([&](const ffc_handle_t& ffc_handle) {
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
			
			for_every_ffc([&](const ffc_handle_t& ffc_handle) {
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
		
		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
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
	    return;
    }
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
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
	if(combobuf[cid].only_gentrig)
		type = cNONE;

	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, layer);
	int screen = rpos_handle.screen;
	mapscr* s = rpos_handle.scr;
	int i = rpos_handle.pos;
        
    bool ignorescreen=false;

    if((get_bit(screengrid_layer[layer-1], i) != 0) || (!isCuttableType(type)))
		return;

    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].type==itype_sword) ? itemsbuf[directWpn].level : current_item(itype_sword);
	
	if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid_layer[layer-1],i,1);
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
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(screen, (screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)bx, (zfix)by,(zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(s->secretsfx);
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
				sfx(combobuf[cid].attribytes[2],pan(bx));
			}
		}
		else
		{
			if (combobuf[cid].usrflags&cflag3)
			{
				sfx(combobuf[cid].attribytes[2],pan(bx));
			}
			else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
		}
	}
	
	spawn_decoration_xy(combobuf[cid], fx, fy, bx+8, by+8);
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
	int32_t i = rpos_handle.pos;

	bool ignorescreen=false;
	bool ignoreffc=false;
	
	if(get_bit(screengrid, i) != 0)
	{
		ignorescreen = true;
	}
	else if(cmb.only_gentrig)
		ignorescreen = true;
	
	
	if (!current_ffc_handle || current_ffc_handle->ffc->recently_hit)
	{
		ignoreffc = true;
	}
	else if(cmb_ff.only_gentrig)
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
	
	mapscr *s = cur_screen >= 128 ? special_warp_return_scr : rpos_handle.scr;
	
	int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].type==itype_sword) ? itemsbuf[directWpn].level : current_item(itype_sword);
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
			sfx(s->secretsfx);
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
			current_ffc_handle->increment_data();
		}
		else
		{
			current_ffc_handle->set_data(s->undercombo);
			current_ffc_handle->ffc->cset = s->undercset;
		}
	}
	
	if(!ignorescreen)
	{
		if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid,i,1);
		
		if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
		{
			items.add(new item((zfix)bx, (zfix)by,(zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
			sfx(s->secretsfx);
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
					sfx(cmb.attribytes[2],pan(bx));
				}
			}
			else
			{
				if (cmb.usrflags&cflag3)
				{
					sfx(cmb.attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
			}
		}
		
		spawn_decoration_xy(cmb, fx, fy, bx+8, by+8);
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
			
			if(it!=-1 && itemsbuf[it].type != itype_misc) // Don't drop non-gameplay items
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
					sfx(cmb_ff.attribytes[2],pan(bx));
				}
			}
			else
			{
				if (cmb_ff.usrflags&cflag3)
				{
					sfx(cmb_ff.attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
			}
		}

		auto& ffc = *current_ffc_handle->ffc;
		spawn_decoration_xy(cmb_ff, fx, fy, ffc.x+(ffc.hit_width / 2), ffc.y+(ffc.hit_height / 2));
	}
}

void HeroClass::check_wpn_triggers(int32_t bx, int32_t by, weapon *w)
{
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h-1);
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
	    return;
    }
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
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
	if(combobuf[cid].only_gentrig)
		type = cNONE;
	
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, layer);
    int32_t i = rpos_handle.pos;
    
	// bool checked = w->rposes_checked.contains({rpos_handle.layer, rpos_handle.rpos});
    if((get_bit(w->wscreengrid_layer[layer-1], i) != 0) || (!isCuttableType(type)))
    {
	return; 
    }

	mapscr* s = rpos_handle.scr;
    
    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].type==itype_sword) ? itemsbuf[directWpn].level : current_item(itype_sword);
    
    {
	    if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(w->wscreengrid_layer[layer-1],i,1);
            if(isCuttableNextType(type) || isCuttableNextType(type))
            {
                s->data[i]++;
            }
            else
            {
                s->data[i] = s->undercombo;
                s->cset[i] = s->undercset;
                s->sflag[i] = 0;
            }
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(s->secretsfx);
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
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
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
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
    int32_t cid = MAPCOMBO(bx,by);
        
    //find out which combo row/column the coordinates are in
    bx = TRUNCATE_TILE(bx);
    by = TRUNCATE_TILE(by);
    
    int32_t type = COMBOTYPE(bx,by);
    int32_t type2 = FFCOMBOTYPE(fx,fy);
    int32_t flag = MAPFLAG(bx,by);
    int32_t flag2 = MAPCOMBOFLAG(bx,by);
    int32_t flag3 = MAPFFCOMBOFLAG(fx,fy);
	if(combobuf[cid].only_gentrig)
		type = cNONE;
    byte dontignore = 0;
    byte dontignoreffc = 0;
    
	    if (isCuttableType(type) && FindComboTriggerMatch(w, cid) > -1)
	    {
		dontignore = 1;
	    }
	if(w->useweapon != wSword && !dontignore) return;

    auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, 0);
    int32_t i = rpos_handle.pos;

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
    else if(combobuf[current_ffc_handle->data()].only_gentrig)
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
    
    mapscr *s = cur_screen >= 128 ? special_warp_return_scr : rpos_handle.scr;
    
    int32_t sworditem = (directWpn>-1 && itemsbuf[directWpn].type==itype_sword) ? itemsbuf[directWpn].level : current_item(itype_sword);
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
            sfx(s->secretsfx);
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
            current_ffc_handle->increment_data();
        }
        else
        {
            current_ffc_handle->set_data(s->undercombo);
            current_ffc_handle->ffc->cset = s->undercset;
        }
    }
    
    if(!ignorescreen || dontignore)
    {
        if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(w->wscreengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(s->secretsfx);
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
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
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
            
            if(it!=-1 && itemsbuf[it].type != itype_misc) // Don't drop non-gameplay items
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
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
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
	byte dontignore = 0;
	byte dontignoreffc = 0;
    
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
    int32_t cid = MAPCOMBO(bx,by);
   
    //Z_scripterrlog("check_wand_block2 MatchComboTrigger() returned: %d\n", );
    if(w->useweapon != wWand && FindComboTriggerMatch (w, cid) < 0) return;
    if ( FindComboTriggerMatch (w, cid) != -1 ) dontignore = 1;
    
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
    
    if((trigger_secrets_if_flag(bx,by,mfWAND,true)==false)&&(trigger_secrets_if_flag(bx,by,mfSTRIKE,true)==false))
    {
        if(flag3==mfWAND||flag3==mfSTRIKE)
        {
            trigger_secrets_if_flag(fx,fy,mfWAND,true);
            trigger_secrets_if_flag(fx,fy,mfSTRIKE,true);
        }
    }
    
    if(dontignore)
		trigger_secrets_if_flag(bx,by,mfWAND,true);
}

void HeroClass::check_slash_block(weapon *w)
{
	//first things 
	
	int32_t par_item = w->parentitem;
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].weap_data.imitate_weapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
    if(usewpn != wSword) return;
	
	
    int32_t bx = 0, by = 0;
	bx = ((int32_t)w->x) + (((int32_t)w->hit_width)/2);
	by = ((int32_t)w->y) + (((int32_t)w->hit_height)/2);
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
    
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
    int32_t i = rpos_handle.pos;
        
	if(combobuf[cid].only_gentrig)
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
    else if(combobuf[current_ffc_handle->data()].only_gentrig)
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
    
    mapscr *s = cur_screen >= 128 ? special_warp_return_scr : rpos_handle.scr;
    
    int32_t sworditem = (par_item >-1 ? itemsbuf[par_item].level : current_item(itype_sword)); //Get the level of the item, else the highest sword level in inventory.
    
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
            sfx(s->secretsfx);
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
            current_ffc_handle->increment_data();
        }
        else
        {
            current_ffc_handle->set_data(s->undercombo);
            current_ffc_handle->set_cset(s->undercset);
        }
    }
    
    if(!ignorescreen)
    {
        if(!isTouchyType(type) && !get_qr(qr_CONT_SWORD_TRIGGERS)) set_bit(screengrid,i,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by,(zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(s->secretsfx);
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
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
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
			
            if(it!=-1 && itemsbuf[it].type != itype_misc) // Don't drop non-gameplay items
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
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
			}
			else
			{
				if (combobuf[cid].usrflags&cflag3)
				{
					sfx(combobuf[cid].attribytes[2],pan(bx));
				}
				else sfx(QMisc.miscsfx[sfxBUSHGRASS],pan(bx));
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
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
    
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
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
    
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
        
    mapscr *s = cur_screen >= 128 ? special_warp_return_scr : get_scr_for_world_xy(bx, by);
    
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
            sfx(s->secretsfx);
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
            sfx(s->secretsfx);
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
            current_ffc_handle->increment_data();
        }
    }
    
    if(!ignorescreen)
    {
        if(pound)
            s->data[pos]+=1;
            
        set_bit(grid,pos,1);
        
        set_bit(screengrid,pos,1);
        
        if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
        {
            items.add(new item((zfix)bx, (zfix)by, (zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
            sfx(s->secretsfx);
        }
        
        if(type==cPOUND && get_qr(qr_MORESOUNDS))
            sfx(QMisc.miscsfx[sfxHAMMERPOUND],pan(bx));
        
        putcombo(scrollbuf, bx - viewport.x, by - viewport.y, s->data[pos], s->cset[pos]);
    }
    
    if(!ignoreffc)
    {
		current_ffc_handle->ffc->recently_hit = true;

        if(type2==cPOUND && get_qr(qr_MORESOUNDS))
            sfx(QMisc.miscsfx[sfxHAMMERPOUND],pan(bx));
    }
    
    return;
}

void HeroClass::check_pound_block_layer(int bx, int by, int lyr, weapon* w)
{
	if(lyr < 1 || lyr > 2) return; //sanity
	//keep things inside the screen boundaries
	bx=vbound(bx, 0, world_w-1);
	by=vbound(by, 0, world_h-1);
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
	auto rpos_handle = get_rpos_handle_for_world_xy(bx, by, lyr);
	int32_t i = rpos_handle.pos;
	
	bool pound=false;
	
	if(type!=cPOUND && flag!=mfHAMMER && flag!=mfSTRIKE && flag2!=mfHAMMER && flag2!=mfSTRIKE)
		return;
		
	if(get_bit(grid, i) != 0)
		return;
		
	mapscr *s = rpos_handle.scr;
	
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
		sfx(s->secretsfx);
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
		sfx(s->secretsfx);
	}
	else pound = true;
	
	if(pound)
		s->data[i]+=1;
		
	set_bit(grid,i,1);
	
	if((flag==mfARMOS_ITEM||flag2==mfARMOS_ITEM) && (!getmapflag(s, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (s->flags9&fBELOWRETURN)))
	{
		items.add(new item((zfix)bx, (zfix)by, (zfix)0, s->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((s->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
		sfx(s->secretsfx);
	}
	
	if(type==cPOUND && get_qr(qr_MORESOUNDS))
		sfx(QMisc.miscsfx[sfxHAMMERPOUND],pan(bx));
	

	putcombo(scrollbuf,bx-viewport.x,by-viewport.y,s->data[i],s->cset[i]);
}

void HeroClass::check_wand_block(weapon *w)
{
	
    int32_t par_item = w->parentitem;
	int32_t usewpn = -1;
	if ( par_item > -1 )
	{
		usewpn = itemsbuf[par_item].weap_data.imitate_weapon;
	}
	else if ( par_item == -1 && w->ScriptGenerated ) 
	{
		usewpn = w->useweapon;
	}
    if(usewpn != wWand) return;
	
	
    int32_t bx = 0, by = 0;
	bx = ((int32_t)w->x) + (((int32_t)w->hit_width)/2);
	by = ((int32_t)w->y) + (((int32_t)w->hit_height)/2);
	
    //keep things inside the screen boundaries
    bx=vbound(bx, 0, world_w-1);
    by=vbound(by, 0, world_h-1);
    int32_t fx=bx;
    int32_t fy=by;
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
				sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
			sfx(WAV_CHINK,pan(x));
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
				sfx(WAV_CHINK,pan(x));
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
			//sfx(WAV_HEAL,pan(x));
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
			//sfx(WAV_BREAKSHIELD,pan(x));
			if(itemsbuf[itemid].flags&item_edible)
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
	bool standard = !(shield.flags&item_flag9) || usingActiveShield();
	if(standard) //Use standard sides, either a passive shield, or a held active shield
	{
		if((cmpdir&CMPDIR_FRONT) && (shield.flags&item_flag1))
			return true;
		else if((cmpdir&CMPDIR_BACK) && (shield.flags&item_flag2))
			return true;
		else if((cmpdir&CMPDIR_LEFT) && (shield.flags&item_flag3))
			return true;
		else if((cmpdir&CMPDIR_RIGHT) && (shield.flags&item_flag4))
			return true;
	}
	else //Active Shield that is NOT held down
	{
		if((cmpdir&CMPDIR_FRONT) && (shield.flags&item_flag5))
			return true;
		else if((cmpdir&CMPDIR_BACK) && (shield.flags&item_flag6))
			return true;
		else if((cmpdir&CMPDIR_LEFT) && (shield.flags&item_flag7))
			return true;
		else if((cmpdir&CMPDIR_RIGHT) && (shield.flags&item_flag8))
			return true;
	}
	return false;
}

static bool sh_check(uint fl_block, uint fl_ref, int wty, bool& reflect, bool boss, bool defret)
{
	reflect = false;
	switch(wty)
	{
		case ewBrang:
			if(!(fl_block & sh_brang)) break;
			
			reflect = ((fl_ref & sh_brang) != 0);
			return true;
		case ewArrow:
		case wRefArrow:
			if(!(fl_block & sh_arrow)) break;
			
			reflect = ((fl_ref & sh_arrow) != 0);
			return true;
			
		case ewRock:
		case wRefRock:
			if(!(fl_block & sh_rock)) break;
			
			reflect = ((fl_ref & sh_rock) != 0);
			return true;
		case ewFlame:
		case wRefFire:
			if(!(fl_block & sh_flame)) break;
				
			reflect = ((fl_ref & sh_flame) != 0);
			return true;
		case ewFlame2:
		case wRefFire2:
			if(get_qr(qr_BROKEN_FLAME_ARROW_REFLECTING))
				return true;
			if(!(fl_block & sh_flame2)) break;
				
			reflect = ((fl_ref & sh_flame2) != 0);
			return true;
			
		case ewFireball2:
		case ewFireball:
		case wRefFireball:
		{
			int32_t mask = (boss ? sh_fireball2 : sh_fireball);
			
			if(!(fl_block & mask)) break;
			
			reflect = ((fl_ref & mask) != 0);
			return true;
		}
		
		case ewSword:
		case wRefBeam:
			if(!(fl_block & sh_sword)) break;
			
			reflect = ((fl_ref & sh_sword) != 0);
			return true;
			
		case wRefMagic:
		case ewMagic:
			if(!(fl_block & sh_magic)) break;
			
			reflect = ((fl_ref & sh_magic) != 0);
			return true;
			
		#define SCRIPT_SHBLOCK(scrfl) \
			if(!(fl_block & (sh_script|scrfl))) break; \
			reflect = ((fl_ref & (sh_script|scrfl)) != 0); \
			return true
		case wScript1: SCRIPT_SHBLOCK(sh_script1);
		case wScript2: SCRIPT_SHBLOCK(sh_script2);
		case wScript3: SCRIPT_SHBLOCK(sh_script3);
		case wScript4: SCRIPT_SHBLOCK(sh_script4);
		case wScript5: SCRIPT_SHBLOCK(sh_script5);
		case wScript6: SCRIPT_SHBLOCK(sh_script6);
		case wScript7: SCRIPT_SHBLOCK(sh_script7);
		case wScript8: SCRIPT_SHBLOCK(sh_script8);
		case wScript9: SCRIPT_SHBLOCK(sh_script9);
		case wScript10: SCRIPT_SHBLOCK(sh_script10);
		#undef SCRIPT_SHBLOCK
		
		case wWind:
			if(!(fl_block & sh_lw_wind)) break;
			
			reflect = ((fl_ref & sh_lw_wind) != 0);
			return true;
		case ewWind:
			if(!(fl_block & sh_ew_wind)) break;
			
			reflect = ((fl_ref & sh_ew_wind) != 0);
			return true;
		
		case ewLitBomb:
		case ewLitSBomb:
			return true;
		
		default:
			return defret;
	}
	return false;
}

bool HeroClass::check_ewpn_collide(weapon* w)
{
	if(w->ignoreHero || w->fallclk|| w->drownclk)
		return false;
	if(!w->hit(x+7,y+7-fakez,z,2,2,1))
		return false;
	
	int32_t stompid = current_item_id(itype_stompboots);
	
	if(current_item(itype_stompboots) && checkbunny(stompid) && checkmagiccost(stompid) && (stomping ||
	((z+fakez) > (w->z+(w->fakez))) ||
	((isSideViewHero() && (y+16)-(w->y)<=14) && falling_oldy<y)))
	{
		itemdata const& stomp = itemsbuf[stompid];
		bool reflect = false; //unused, always false
		if(!sh_check(stomp.misc2, 0, w->id, reflect, w->level&1, true))
		{
			w->onhit(false);
			sfx(WAV_CHINK,pan(x));
			return false;
		}
	}

	int32_t defresult = defend(w);
	if ( defresult == -1 ) return false; //The weapon did something special, but it is otherwise ignored, possibly killed by defend(). 
		
	if(w->id==ewWind)
	{
		xofs=1000;
		action=freeze; FFCore.setHeroAction(freeze);
		w->misc=999;                                         // in enemy wind
		attackclk=0;
		return false;
	}
	
	switch(w->id)
	{
		case ewLitBomb:
		case ewBomb:
		case ewLitSBomb:
		case ewSBomb:
			return true;
	}
	
	int32_t itemid = getCurrentShield(false);
	if(itemid<0 || !(checkbunny(itemid) && checkmagiccost(itemid)))
		return true;
	itemdata const& shield = itemsbuf[itemid];
	bool allow_inactive = (shield.flags & item_flag9);
	auto cmpdir = compareDir(w->dir);
	bool hitshield = compareShield(cmpdir, shield);
	
	if(!allow_inactive && ((lift_wpn && (liftflags & LIFTFL_DIS_SHIELD)) || (action==attacking||action==sideswimattacking) || action==swimming || action == sideswimming || action == sideswimattacking || charging > 0 || spins > 0 || hopclk==0xFF))
		return true;
	
	if(!hitshield)
		return true;
	
	paymagiccost(itemid);
	
	bool reflect = false;
	
	if(!sh_check(shield.misc1, shield.misc2, w->id, reflect, w->level&1, true))
		return true;
	
	if(reflect && (w->unblockable&WPNUNB_REFL))
		reflect = false;
	if(!reflect && (w->unblockable&WPNUNB_SHLD))
		return true;
	
	int32_t oldid = w->id;
	w->onhit(false, reflect ? 2 : 1, dir);
	
	sfx(shield.usesound,pan(x));
	return true;
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
				bool reflect = false; //unused, always false
				if(!sh_check(stomp.misc2, 0, ew->id, reflect, ew->level&1, true))
				{
					ew->onhit(false);
					sfx(WAV_CHINK,pan(x));
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
			bool allow_inactive = (shield.flags & item_flag9);
			auto cmpdir = compareDir(ew->dir);
			bool hitshield = compareShield(cmpdir, shield);
			
			if(!allow_inactive && ((lift_wpn && (liftflags & LIFTFL_DIS_SHIELD)) || (action==attacking||action==sideswimattacking) || action==swimming || action == sideswimming || action == sideswimattacking || charging > 0 || spins > 0 || hopclk==0xFF))
				return i;
			
			if(!hitshield)
				return i;
			
			paymagiccost(itemid);
			
			bool reflect = false;
			
			if(!sh_check(shield.misc1, shield.misc2, ew->id, reflect, ew->level&1, true))
				return i;
			
			if(reflect && (ew->unblockable&WPNUNB_REFL))
				reflect = false;
			if(!reflect && (ew->unblockable&WPNUNB_SHLD))
				return i;
			
			int32_t oldid = ew->id;
			ew->onhit(false, reflect ? 2 : 1, dir);
			
			sfx(shield.usesound,pan(x));
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
			
			switch(lw->id)
			{
				case wRefFireball: case wRefMagic: case wRefBeam:
				case wRefRock: case wRefArrow: case wRefFire:
				case wRefFire2:
				case wScript1: case wScript2: case wScript3: case wScript4: case wScript5:
				case wScript6: case wScript7: case wScript8: case wScript9: case wScript10:
					break;
				default:
					return -1;
			}
			int32_t itemid = getCurrentShield(false);
			if(itemid<0 || !(checkbunny(itemid) && checkmagiccost(itemid)))
				return i;
			itemdata const& shield = itemsbuf[itemid];
			auto cmpdir = compareDir(lw->dir);
			bool hitshield = compareShield(cmpdir, shield);
			bool reflect = false;
			
			if(!hitshield)
				return i;
			
			bool allow_inactive = (shield.flags & item_flag9);
			if(!allow_inactive && ((lift_wpn && (liftflags & LIFTFL_DIS_SHIELD)) || (action==attacking||action==sideswimattacking) || action==swimming || action == sideswimming || action == sideswimattacking || charging > 0 || spins > 0 || hopclk==0xFF))
				return i;
			
			if(!sh_check(shield.misc1, shield.misc2, lw->id, reflect, lw->level&1, true))
				return i;
			
			paymagiccost(itemid);
			
			lw->onhit(false, reflect ? 2 : 1, dir);
			sfx(shield.usesound,pan(x));
		}
		
	return -1;
}

bool HeroClass::try_lwpn_hit(weapon* w)
{
	int32_t itemid = w->parentitem;
	int indx = Lwpns.find(w);
	//if ( itemdbuf[parentitem].flags&item_flags3 ) //can damage Hero
	//if ( itemsbuf[parentitem].misc1 > 0 ) //damages Hero by this amount. 
	if((!(itemid==-1&&get_qr(qr_FIREPROOFHERO)||((itemid>-1&&itemsbuf[itemid].type==itype_candle||itemsbuf[itemid].type==itype_book)&&(itemsbuf[itemid].flags & item_flag3)))) && scriptcoldet && !fallclk && (!superman || !get_qr(qr_FIREPROOFHERO2)))
	{
		if(w->id==wFire && (superman ? ((diagonalMovement||NO_GRIDLOCK)?w->hit(x+4,y+4-fakez,z,7,7,1):w->hit(x+7,y+7-fakez,z,2,2,1)) : w->hit(this))&&
					(itemid < 0 || itemsbuf[itemid].type!=itype_divinefire))
		{
			std::vector<int32_t> &ev = FFCore.eventData;
			ev.clear();
			ev.push_back(lwpn_dp(indx)*10000);
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
			
			if(nullhit) {ev.clear(); return true;}
			
			//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
			ev[0] = ringpower(dmg)*10000;
			
			throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
			dmg = ev[0]/10000;
			int32_t hdir = ev[1]/10000;
			nullhit = ev[2] != 0;
			bool divineprot = ev[3] != 0;
			int32_t iframes = ev[4] / 10000;
			ev.clear();
			if(nullhit) return true;
			if (Lwpns.spr(indx) != w)
			{
				auto hit = Lwpns.find(w);
				if (hit < 0)
					w = nullptr;
				else
				{
					w = (weapon*)Lwpns.spr(hit);
					indx = hit;
				}
			}
			if(!divineprot)
			{
				game->set_life(zc_max(game->get_life()-dmg,0));
				if (!get_qr(qr_BROKENHITBY) && w)
				{
					sethitHeroUID(HIT_BY_LWEAPON,(indx+1));
					sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
					sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
					sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
					if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
					else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
					sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].type);
				}
			}
			
			doHit(hdir, iframes);
			return true;
		}
	}
	
	//   check enemy weapons true, 1, -1
	//
	if((itemsbuf[itemid].flags & item_flag6))
	{
		if(w->id==wBrang || (w->id==wHookshot&&!pull_hero))
		{
			int32_t itemid = w->parentitem>-1 ? w->parentitem :
						 directWpn>-1 ? directWpn : current_item_id(w->id==wHookshot ? (w->family_class == itype_switchhook ? itype_switchhook : itype_hookshot) : itype_brang);
			itemid = vbound(itemid, 0, MAXITEMS-1);
			
			for(int32_t j=0; j<Ewpns.Count(); j++)
			{
				sprite *t = Ewpns.spr(j);
				
				if(w->hit(t->x+7,t->y+7-t->fakez,t->z,2,2,1))
				{
					bool reflect = false;
					// sethitHeroUID(HIT_BY_EWEAPON,j); //set that Hero was hit by a specific eweapon index. 
					if(sh_check(itemsbuf[itemid].misc3, itemsbuf[itemid].misc4, t->id, reflect, ((weapon*)t)->level&1, false))
					{
						w->dead=1;
						weapon *ew = ((weapon*)t);
						int32_t oldid = ew->id;
						ew->onhit(true, reflect ? 2 : 1, w->dir);
					}
					
					break;
				}
			}
		}
	}
	
	if((itemsbuf[itemid].flags & item_flag2)||(itemid==-1&&get_qr(qr_OUCHBOMBS)))
	{
		if(((w->id==wBomb)||(w->id==wSBomb)) && !superman && scriptcoldet && !fallclk)
		{
			bool didhit = w->hit(this);
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
				
				if(nullhit) {ev.clear(); return true;}
				
				//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
				ev[0] = ringpower(dmg)*10000;
				
				throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
				dmg = ev[0]/10000;
				int32_t hdir = ev[1]/10000;
				nullhit = ev[2] != 0;
				bool divineprot = ev[3] != 0;
				int32_t iframes = ev[4] / 10000;
				ev.clear();
				if(nullhit) return true;
				if (Lwpns.spr(indx) != w)
				{
					auto hit = Lwpns.find(w);
					if (hit < 0)
						w = nullptr;
					else
					{
						w = (weapon*)Lwpns.spr(hit);
						indx = hit;
					}
				}
				if(!divineprot)
				{
					game->set_life(zc_min(game->get_maxlife(), zc_max(game->get_life()-dmg,0)));
					if (!get_qr(qr_BROKENHITBY) && w)
					{
						sethitHeroUID(HIT_BY_LWEAPON,(indx+1));
						sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
						sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
						sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
						if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
						else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
						sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].type);
					}
				}
				
				doHit(hdir, iframes);
				return true;
			}
		}
	}
	
	if(hclk==0 && w->id==wWind && w->hit(x+7,y+7-fakez,z,2,2,1) && !fairyclk)
	{
		std::vector<int32_t> &ev = FFCore.eventData;
		ev.clear();
		ev.push_back(0);
		ev.push_back(w->dir*10000);
		ev.push_back(0);
		ev.push_back(0);
		ev.push_back(ZSD_LWPN*10000);
		ev.push_back(w->getUID());
		ev.push_back(ZSD_NONE*10000);
		ev.push_back(0);
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
		bool nullhit = ev[2] != 0;
		if(nullhit) {ev.clear(); return true;}
		
		throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
		int32_t hdir = ev[1]/10000;
		nullhit = ev[2] != 0;
		ev.clear();
		if(nullhit) return true;
		if (Lwpns.spr(indx) != w)
		{
			auto hit = Lwpns.find(w);
			if (hit < 0)
				w = nullptr;
			else
			{
				w = (weapon*)Lwpns.spr(hit);
				indx = hit;
			}
		}
		
		reset_hookshot();
		xofs=1000;
		action=inwind; FFCore.setHeroAction(inwind);
		dir = hdir;
		if(w) w->dir = hdir;
		spins = charging = attackclk = 0;
		
		// In case Hero used two whistles in a row, summoning two whirlwinds,
		// check which whistle's whirlwind picked him up so the correct
		// warp ring will be used
		int32_t whistle = w ? w->parentitem : -1;
		
		if(whistle>-1 && itemsbuf[whistle].type==itype_whistle)
			whistleitem=whistle;
			
		return true;
	}
	return false;
}

bool HeroClass::try_ewpn_hit(weapon* w, bool force)
{
	if(!force)
	{
		if(!check_ewpn_collide(w))
			return false;
		if(w->ignoreHero || w->fallclk|| w->drownclk)
			return false;
	}
	auto indx = Ewpns.find(w);
	std::vector<int32_t> &ev = FFCore.eventData;
	ev.clear();
	ev.push_back((ewpn_dp(indx)*10000));
	ev.push_back(w->hitdir(x,y,16,16,dir)*10000);
	ev.push_back(0);
	ev.push_back(DivineProtectionShieldClk>0?10000:0);
	ev.push_back(48*10000);
	ev.push_back(ZSD_EWPN*10000);
	ev.push_back(w->getUID());
	ev.push_back(ZSD_NONE*10000);
	ev.push_back(0);
	
	throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_1);
	int32_t dmg = ev[0]/10000;
	bool nullhit = ev[2] != 0;
	
	if(nullhit) {ev.clear(); return false;}
	
	//Args: 'damage (post-ring)','hitdir','nullifyhit','type:npc','npc uid'
	ev[0] = ringpower(dmg)*10000;
	
	throwGenScriptEvent(GENSCR_EVENT_HERO_HIT_2);
	dmg = ev[0]/10000;
	int32_t hdir = ev[1]/10000;
	nullhit = ev[2] != 0;
	bool divineprot = ev[3] != 0;
	int32_t iframes = ev[4] / 10000;
	ev.clear();
	if (nullhit) return false;
	if (Ewpns.spr(indx) != w)
	{
		indx = Ewpns.find(w);
		if (indx < 0)
			w = nullptr;
		else w = (weapon*)Ewpns.spr(indx);
	}
	if (!divineprot)
	{
		game->set_life(zc_max(game->get_life() - dmg, 0));
		if (w)
		{
			sethitHeroUID(HIT_BY_EWEAPON, (indx + 1));
			sethitHeroUID(HIT_BY_EWEAPON_UID, w->getUID());
			sethitHeroUID(HIT_BY_EWEAPON_ENGINE_UID, w->getUID());
			sethitHeroUID(HIT_BY_EWEAPON_TYPE, w->id);
		}
	}
	
	if(w)
		w->hit_pierce();
	
	doHit(hdir, iframes);
	return true;
}

void HeroClass::checkhit()
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
	
	if(hclk>0 || inlikelike == 1 || is_autowalking() || action==inwind || action==drowning || action==lavadrowning || action==sidedrowning || inwallm || isDiving() || (action==hopping && hopclk<255))
	{
		return;
	}
	
	for(int32_t i=0; i<Lwpns.Count(); i++)
	{
		sprite *s = Lwpns.spr(i);
		int32_t itemid = ((weapon*)(Lwpns.spr(i)))->parentitem;
		//if ( itemdbuf[parentitem].flags&item_flags3 ) //can damage Hero
		//if ( itemsbuf[parentitem].misc1 > 0 ) //damages Hero by this amount. 
		if((!(itemid==-1&&get_qr(qr_FIREPROOFHERO)||((itemid>-1&&itemsbuf[itemid].type==itype_candle||itemsbuf[itemid].type==itype_book)&&(itemsbuf[itemid].flags & item_flag3)))) && scriptcoldet && !fallclk && (!superman || !get_qr(qr_FIREPROOFHERO2)))
		{
			if(s->id==wFire && (superman ? ((diagonalMovement||NO_GRIDLOCK)?s->hit(x+4,y+4-fakez,z,7,7,1):s->hit(x+7,y+7-fakez,z,2,2,1)) : s->hit(this))&&
						(itemid < 0 || itemsbuf[itemid].type!=itype_divinefire))
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
						sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
						sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
						sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
						if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
						else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
						sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].type);
					}
				}
				
				doHit(hdir, iframes);
				return;
			}
		}
		
		//   check enemy weapons true, 1, -1
		//
		if((itemsbuf[itemid].flags & item_flag6))
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
						
						if(sh_check(itemsbuf[itemid].misc3, itemsbuf[itemid].misc4, t->id, reflect, ((weapon*)t)->level&1, false))
						{
							((weapon*)s)->dead=1;
							weapon *ew = ((weapon*)t);
							int32_t oldid = ew->id;
							ew->onhit(true, reflect ? 2 : 1, s->dir);
						}
						break;
					}
				}
			}
		}
		
		if((itemsbuf[itemid].flags & item_flag2)||(itemid==-1&&get_qr(qr_OUCHBOMBS)))
		{
			if(((s->id==wBomb)||(s->id==wSBomb)) && !superman && scriptcoldet && !fallclk)
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
							sethitHeroUID(HIT_BY_LWEAPON_UID,w->getUID());
							sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID,w->getUID());
							sethitHeroUID(HIT_BY_LWEAPON_TYPE, w->id);
							if (w->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, w->parentitem);
							else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
							sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[w->parentitem].type);
						}
					}
					
					doHit(hdir, iframes);
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
			
			if(whistle>-1 && itemsbuf[whistle].type==itype_whistle)
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
		hit2 = (diagonalMovement||NO_GRIDLOCK) ? GuyHitFrom(hit2+1,x+4,y+4-fakez,z,8,8,hzsz)
			: GuyHitFrom(hit2+1,x+7,y+7-fakez,z,2,2,hzsz);
		
		if(hit2!=-1)
		{
			if (hithero(hit2) == 0) return;
		}
	} while (hit2 != -1);
	if (superman || !scriptcoldet || fallclk) return;
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
				sethitHeroUID(HIT_BY_LWEAPON_UID, lwpnspr->getUID());
				sethitHeroUID(HIT_BY_LWEAPON_ENGINE_UID, lwpnspr->getUID());
				sethitHeroUID(HIT_BY_LWEAPON_TYPE, lwpnspr->id);
				if (lwpnspr->parentitem > -1) sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, lwpnspr->parentitem);
				else sethitHeroUID(HIT_BY_LWEAPON_PARENT_ID, -1);
				sethitHeroUID(HIT_BY_LWEAPON_PARENT_FAMILY, itemsbuf[lwpnspr->parentitem].type);
			}
		}
		
		if(lwpnspr)
			lwpnspr->hit_pierce();
		
		doHit(hdir, iframes);
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
				sethitHeroUID(HIT_BY_EWEAPON_UID, ewpnspr->getUID());
				sethitHeroUID(HIT_BY_EWEAPON_ENGINE_UID, ewpnspr->getUID());
				sethitHeroUID(HIT_BY_EWEAPON_TYPE, ewpnspr->id);
			}
		}
		
		if(ewpnspr)
			ewpnspr->hit_pierce();
		
		doHit(hdir, iframes);
		return;
	}
	
	// The rest of this method deals with damage combos, which can be jumped over.
	if((z>0 || fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) return;
	
	int32_t dx1 = (int32_t)x+8-(hero_scr->csensitive);
	int32_t dx2 = (int32_t)x+8+(hero_scr->csensitive-1);
	int32_t dy1 = (int32_t)y+(bigHitbox?8:12)-(bigHitbox?hero_scr->csensitive:(hero_scr->csensitive+1)/2);
	int32_t dy2 = (int32_t)y+(bigHitbox?8:12)+(bigHitbox?hero_scr->csensitive-1:((hero_scr->csensitive+1)/2)-1);
	
	for(int32_t i=get_qr(qr_DMGCOMBOLAYERFIX) ? 1 : -1; i>=-1; i--)  // Layers 0, 1 and 2!!
		checkdamagecombos(dx1,dx2,dy1,dy2,i);
}

bool HeroClass::checkdamagecombos(int32_t dx, int32_t dy)
{
    return checkdamagecombos(dx,dx,dy,dy);
}

void HeroClass::doHit(int32_t hdir, int iframes)
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
		
	hclk = iframes;
	check_on_hit();
	
	if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
	{
		spins = charging = attackclk = 0;
		attack=none;
		tapping = false;
	}
	
	sfx(getHurtSFX(),pan(x));
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
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
	rpos_t rposes[] = {COMBOPOS_REGION_B(dx1,dy1),COMBOPOS_REGION_B(dx1,dy2),COMBOPOS_REGION_B(dx2,dy1),COMBOPOS_REGION_B(dx2,dy2)};
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
		if (rposes[i] == rpos_t::None)
			continue;

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

	int ffc_ids[] = {-1, -1, -1, -1};
	{
		auto ffc_handle = getFFCAt(dx1,dy1);
		ffc_ids[0] = ffc_handle ? ffc_handle->id : -1;
		cid[4] = ffc_handle ? ffc_handle->data() : 0;
		newcombo& cmb = combobuf[cid[4]];
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[0] = cmb.attributes[0]/-10000L;
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[1] = cmb.attributes[0]/-10000L;
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[2] = cmb.attributes[0]/-10000L;
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
		if ( !(cmb.only_gentrig) && combo_class_buf[cmb.type].modify_hp_amount)
		{
			if(cmb.usrflags&cflag1 )
				hp_mod[3] = cmb.attributes[0]/-10000L;
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
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && !_walkflag_layer(dx1,dy1,i)) {hp_mod[0] = 0; hasKB &= ~(1<<4);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && !_walkflag_layer(dx1,dy2,i)) {hp_mod[1] = 0; hasKB &= ~(1<<5);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && !_walkflag_layer(dx2,dy1,i)) {hp_mod[2] = 0; hasKB &= ~(1<<6);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && !_walkflag_layer(dx2,dy2,i)) {hp_mod[3] = 0; hasKB &= ~(1<<7);}
		}
		else
		{
			if (combobuf[MAPCOMBO2(i,dx1,dy1)].type == cBRIDGE && _effectflag_layer(dx1,dy1,i)) {hp_mod[0] = 0; hasKB &= ~(1<<4);}
			if (combobuf[MAPCOMBO2(i,dx1,dy2)].type == cBRIDGE && _effectflag_layer(dx1,dy2,i)) {hp_mod[1] = 0; hasKB &= ~(1<<5);}
			if (combobuf[MAPCOMBO2(i,dx2,dy1)].type == cBRIDGE && _effectflag_layer(dx2,dy1,i)) {hp_mod[2] = 0; hasKB &= ~(1<<6);}
			if (combobuf[MAPCOMBO2(i,dx2,dy2)].type == cBRIDGE && _effectflag_layer(dx2,dy2,i)) {hp_mod[3] = 0; hasKB &= ~(1<<7);}
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

	mapscr* damage_scr;
	int32_t hp_modmin = zc_min(hp_modtotal, hp_modtotalffc);
	int best_type = 0;
	if(hp_modtotalffc < hp_modtotal)
	{
		damage_scr = get_ffc_handle(best_ffcid).scr;
		bestcid = bestffccid;
		best_type = 1;
	}
	else
	{
		damage_scr = get_scr_for_rpos(best_rpos);
	}
	
	bool global_defring = ((itemsbuf[current_item_id(itype_ring)].flags & item_flag1));
	bool global_perilring = ((itemsbuf[current_item_id(itype_perilring)].flags & item_flag1));

	bool current_ring = ((hero_scr->flags6&fTOGGLERINGDAMAGE) != 0);
	if(current_ring)
	{
		global_defring = !global_defring;
		global_perilring = !global_perilring;
	}
	int32_t itemid = current_item_id(itype_boots);
	
	bool bootsnosolid = itemid >= 0 && 0 != (itemsbuf[itemid].flags & item_flag1);
	bool ignoreBoots = itemid >= 0 && (itemsbuf[itemid].flags & item_flag3);
	
	if(hp_modmin<0)
	{
		if((itemid<0) || ignoreBoots || (damage_scr->flags5&fDAMAGEWITHBOOTS) || (4<<current_item_power(itype_boots)<(abs(hp_modmin))) || (solid && bootsnosolid) || !(checkbunny(itemid) && checkmagiccost(itemid)))
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
			doHit(hitdir, iframes);
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
		
		if(itemsbuf[stompid].flags & item_flag1)
		{
			fall = -(itemsbuf[stompid].misc1);
		}
		
		if(itemsbuf[stompid].flags & item_downgrade)
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
	else if(superman || !scriptcoldet || fallclk)
		return 0;
	//!TODO SOLIDPUSH Enemy flag to make them not deal contact damage
	//!Add a flag check to this if:
	else if (!(enemyptr->stunclk==0 && enemyptr->frozenclock==0 && (!get_qr(qr_SAFEENEMYFADE) || enemyptr->fading != fade_flicker)
			&& (enemyptr->d->type != eeGUY || enemyptr->dmisc1)))
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
		sethitHeroUID(HIT_BY_NPC_UID,enemyptr->getUID());
		sethitHeroUID(HIT_BY_NPC_ENGINE_UID,enemyptr->getUID());
		sethitHeroUID(HIT_BY_NPC_ID, enemyptr->id);
		sethitHeroUID(HIT_BY_NPC_TYPE, enemyptr->type);
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
	check_on_hit();
	sfx(getHurtSFX(),pan(x));
	
	if(charging > 0 || spins > 0 || attack == wSword || attack == wHammer)
	{
		spins = charging = attackclk = 0;
		attack=none;
		tapping = false;
	}
	
	enemy_scored(hit2);
	int32_t dm7 = enemyptr->dmisc7;
	int32_t dm8 = enemyptr->dmisc8;
	
	switch(enemyptr->type)
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
			int32_t shielddivisor = ((itemid > -1 && itemsbuf[itemid].misc1 & 4) ? itemsbuf[itemid].power : 1);
			switch(dm7)
			{
				case e7tTEMPJINX:
					if(dm8&e8tSWORD)
						if(swordclk>=0 && !(sworddivisor==0))
							swordclk=int32_t(150/sworddivisor);

					if(dm8&e8tITEM)
						if(itemclk>=0 && !(itemdivisor==0))
							itemclk=int32_t(150/itemdivisor);

					if(dm8&e8tSHIELD)
						if (shieldjinxclk >= 0 && !(shielddivisor==0))
							shieldjinxclk=int32_t(150/shielddivisor);
							
					break;
				
				case e7tPERMJINX:
					if (dm8&e8tSWORD)
						if(sworddivisor) swordclk=(itemid >-1 && itemsbuf[itemid].flags & item_flag1)? int32_t(150/sworddivisor) : -1;
						
					if (dm8&e8tITEM)
						if(itemdivisor) itemclk=(itemid >-1 && itemsbuf[itemid].flags & item_flag1)? int32_t(150/itemdivisor) : -1;
					
					if (dm8&e8tSHIELD)
						if(shielddivisor) shieldjinxclk=(itemid >-1 && itemsbuf[itemid].flags & item_flag1)? int32_t(150/shielddivisor) : -1;

					break;
				
				case e7tUNJINX:
					if (dm8&e8tSWORD)
						swordclk=0;
						
					if (dm8&e8tITEM)
						itemclk=0;

					if (dm8&e8tSHIELD)
						shieldjinxclk=0;

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
        
    int32_t itemtype = itemsbuf[itemid].type;
    
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
        
        if(w->id == wPhantom && w->level == type1)
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

//Waitframe handler for refilling operations
static void do_refill_waitframe()
{
	put_passive_subscr(framebuf,0,0,game->should_show_time(),sspUP);
	if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN))
	{
		script_drawing_commands.Clear();
		if(DMaps[cur_dmap].passive_sub_script != 0)
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
		
		if (FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[cur_dmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}	
		do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
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
		if(itemsbuf[q].type == itype_bottle && itemsbuf[q].misc1 == slot)
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
			int32_t sourcescr = cur_screen, sourcedmap = cur_dmap;
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
	update_current_screen();

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
						lift_wpn->moveflags &= ~move_obeys_grav;
						drop_liftwpn();
						goto heroanimate_skip_liftwpn;
					}
					++lift_wpn->clk;
				}
				break;
		}
		if(lift_wpn->dead>0)
			--lift_wpn->dead;
		
		if(get_qr(qr_LIFTED_WEAPONS_RUN_SCRIPTS))
			lift_wpn->run_script(MODE_NORMAL);
		
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
		setpit();
		dowarp(hero_scr, 3, 0);
		cheats_execute_goto=false;
		solid_update(false);
		return false;
	}
	
	if(cheats_execute_light)
	{
		toggle_lights(pal_litOVERRIDE); //Forcibly set permLit, overriding its current setting
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
		int maxlyr = get_qr(qr_BUSHESONLAYERS1AND2) ? 2 : 0;
		if (get_qr(qr_GRASS_SENSITIVE))
		{
			bool g1 = false, g2 = false, g3 = false, g4 = false;
			for(int q = maxlyr; q >= 0; --q)
			{
				g1 = g1 || isGrassType(combobuf[MAPCOMBO2(q-1,x+4,y+15)].type);
				g2 = g2 || isGrassType(combobuf[MAPCOMBO2(q-1,x+11,y+15)].type);
				g3 = g3 || isGrassType(combobuf[MAPCOMBO2(q-1,x+4,y+9)].type);
				g4 = g4 || isGrassType(combobuf[MAPCOMBO2(q-1,x+11,y+9)].type);
			}
			if(g1 && g2 && g3 && g4)
			{
				int grasscid = 0;
				for(int q = maxlyr; q >= 0; --q)
				{
					int cid = MAPCOMBO2(q-1,x+8,y+12);
					if(cid > 0 && isGrassType(combobuf[cid].type))
					{
						grasscid = cid;
						break;
					}
				}
				if(grasscid)
				{
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
		}
		else
		{
			bool g1 = false, g2 = false;
			for(int q = maxlyr; q >= 0; --q)
			{
				g1 = g1 || isGrassType(combobuf[MAPCOMBO2(q-1,x,y+15)].type);
				g2 = g2 || isGrassType(combobuf[MAPCOMBO2(q-1,x+15,y+15)].type);
			}
			if(g1 && g2)
			{
				int grasscid = 0;
				for(int q = maxlyr; q >= 0; --q)
				{
					int cid = MAPCOMBO2(q-1,x+8,y+15);
					if(cid > 0 && isGrassType(combobuf[cid].type))
					{
						grasscid = cid;
						break;
					}
				}
				if(grasscid)
				{
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
	}
	if(z==0 && fakez==0) //Shallow Water / Custom Walk SFX
	{
		if (get_qr(qr_SHALLOW_SENSITIVE))
		{
			if (action != swimming && action != isdiving && action != drowning && action!=lavadrowning && action!=sidedrowning && action!=rafting && action != falling && !IsSideSwim() && !(ladderx+laddery) && !pull_hero && !walk_through_walls)
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
						waterid = iswaterex_z3(waterid, -1, watercheck_x,watercheck_y, false, false, true, true);
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
											check_on_hit();
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
		
		for_each_rpos_stood_on([&](const rpos_handle_t& handle)
			{
				auto& cmb = handle.combo();
				byte csfx = action == walking ? cmb.sfx_walking : cmb.sfx_standing;
				byte cspr = action == walking ? cmb.spr_walking : cmb.spr_standing;
				if(csfx)
					sfx_no_repeat(csfx, pan(x));
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
			});
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
	
	if(action!=inwind && action!=drowning && action!=lavadrowning && action!= sidedrowning && !is_autowalking())
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
		checksigns();
		checkgenpush();
	}
	
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
		if(!(itemsbuf[hoverid].flags & item_flag1))
		{
			if(hoverclk < 0) hoverclk = 0;
			hoverflags &= ~HOV_OUT;
		}
	}
	bool platformfell2 = false;
	last_rocs_id = getRocsPressed(); // reset the cached roc's feather ID
	if (current_rocs_jump_id > -1 && current_rocs_jump_id != last_rocs_id)
		released_jump_button = true;
	if (current_rocs_jump_id > -1 && released_jump_button)
	{
		itemdata const& last_rocs = itemsbuf[current_rocs_jump_id];
		if (fall < 0)
		{
			if (last_rocs.flags & item_flag6)
			{
				int32_t jump_loss = last_rocs.misc6 / 100;
				fall = zc_min(0_zf, fall + jump_loss);
			}
			if (fall >= 0)
				try_hover();
		}
		if (fall >= 0)
		{
			current_rocs_jump_id = -1;
			released_jump_button = false;
		}
	}
	last_grav_boots_id = -1; // clear grav boots, so that they are re-checked in get_gravity()/get_terminalv() if needed
	int32_t gravity3 = get_grav_fall();
	int32_t termv = get_terminalv_fall();
	bool used_grav_or_termv = false;

	if (handle_termv())
		used_grav_or_termv = true;
	else if (fall > termv || fakefall > termv)
	{
		if ((last_grav_boots_id != -1) ||
			(last_rocs_id != -1 && (itemsbuf[last_rocs_id].flags & item_flag5)))
		{
			if (fall > termv)
				fall = termv;
			if (fakefall > termv)
				fakefall = termv;
			used_grav_or_termv = true;
		}
	}
	bool nograv = (sideview_mode() && is_autowalking());
	if(nograv)
		;
	else if(sideview_mode())  // Sideview gravity
	{
		//Handle falling through a platform
		bool platformfell = false;
		if (on_sideview_solid_oldpos(this,true,3) && !on_sideview_solid_oldpos(this,false,3))
		{
			if (!(!on_sideview_slope(Hero.x, Hero.y, Hero.old_x, Hero.old_y) && (on_sideview_slope(Hero.x, Hero.y + 1, Hero.old_x, Hero.old_y) || on_sideview_slope(Hero.x, Hero.y + 2, Hero.old_x, Hero.old_y)) && getInput(btnDown, INPUT_HERO_ACTION))) platformfell = true;
			y+=1; //Fall down a pixel instantly, through the platform.
			if(fall < 0) fall = 0;
			if(jumping < 0) jumping = 0;
			platformfell2 = true;
		}
		//Unless using old collision, run this check BEFORE moving Hero, to prevent clipping into the ceiling.
		if(!get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON))
		{
			if(fall < 0 && (_walkflag(x+4,y+((bigHitbox||!diagonalMovement)?(fall/100):(fall/100)+8),1,get_standing_z_state()) || _walkflag(x+12,y+((bigHitbox||!diagonalMovement)?(fall/100):(fall/100)+8),1,get_standing_z_state())
				|| ((y+(fall/100)<=0) &&
				// Extra checks if Smart Screen Scrolling is enabled
				 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(hero_scr->flags&fMAZE)) &&
											   !(hero_scr->flags2&wfUP)) && (nextcombo_solid(up)))))))
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
		if (!(walk_through_walls && getInput(btnUp, INPUT_HERO_ACTION)) && !drownclk && action != rafting && !IsSideSwim() && !pull_hero && !((ladderx || laddery) && fall > 0) && !getOnSideviewLadder())
		{
			int32_t ydiff = fall/(spins && fall<0 ? 200:100);
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
						if(on_sideview_solid_oldpos(this, false, 0, 0, q))
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
			if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !autostep)
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
		if((on_sideview_solid_oldpos(this) || getOnSideviewLadder())  && !(pull_hero && dir==down) && action!=rafting && !platformfell2)
		{
			stop_item_sfx(itype_hoverboots);
			auto oldfall = fall;
			fall = hoverclk = jumping = 0;
			if(get_qr(qr_OLD_SIDEVIEW_LANDING_CODE))
			{
				if(!getOnSideviewLadder() && (oldfall > 0 || get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON)))
				{
					y.doFloor();
					y-=(int32_t)y%8; //fix position
				}
				if(oldfall > 0)
					land_on_ground();
			}
			else
			{
				snap_platform();
				if(oldfall > 0)
					land_on_ground();
			}
			inair = false;
			hoverflags = 0;
			
			if(y>=world_h-16 && cur_screen>=0x70 && !(get_scr_for_world_xy(x, y)->flags2&wfDOWN))  // Landed on the bottommost screen.
				y = world_h-16;
		}
		// Stop hovering if you press down.
		else if ((hoverclk > 0 || ladderx || laddery) && getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			stop_item_sfx(itype_hoverboots);
			hoverclk = -hoverclk;
			reset_ladder();
			fall = gravity3;
			used_grav_or_termv = true;
			inair = false;
		}
		else if (hoverclk < 1 && !inair && fall == 0 && !IsSideSwim() && justmoved <= 0)
		{
			needFall = true;
			if (replay_version_check(0, 23))
			{
				zfix my = y + 4;
				for (zfix ty = y + 1; ty < my; ++ty)
				{
					if (on_sideview_solid_oldpos(this, false, 0, 0, ty-y))
					{
						y = ty;
						if (check_new_slope(x, ty + 1, 16, 16, old_x, old_y, false) < 0)
						{
							if (!slopeid)
								slopeid = get_new_slope(x, ty + 1, 16, 16, old_x, old_y).get_info().slope();
							onplatid = 1;
						}
						needFall = false;
						break;
					}
				}
			}
			else
			{
				zfix dy = 0;
				bool landed = false;
				dy = binary_search_zfix(0, 4, [&](zfix val, zfix& retval)
					{
						if (on_sideview_solid_oldpos(this, false, 0, 0, val))
						{
							retval = val;
							landed = true;
							return BSEARCH_CONTINUE_TOWARD0;
						}
						else return BSEARCH_CONTINUE_AWAY0;
					});
				if (dy)
				{
					push_move(0, dy);
				}
				if (landed)
				{
					if (check_new_slope(x, y + 0.0001_zf, 16, 16, old_x, old_y, false, true) < 0)
					{
						if (!slopeid)
							slopeid = get_new_slope(x, y + 0.0001_zf, 16, 16, old_x, old_y).get_info().slope();
						onplatid = 1;
					}
					needFall = false;
				}
			}
		}
		else
		{
			needFall = true;
		}
		// Continue falling.
		
		if(fall <= termv && needFall)
		{
			inair = true;
			if (fall != 0 || hoverclk > 0)
				jumping++;
				
			// Bump head if: hit a solid combo from beneath, or hit a solid combo in the screen above this one.
			if(get_qr(qr_OLD_SIDEVIEW_CEILING_COLLISON))
			{
				if((_walkflag(x+4,y-(bigHitbox?9:1),0,get_standing_z_state())
					|| (y<=(bigHitbox?9:1) &&
					// Extra checks if Smart Screen Scrolling is enabled
					 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(hero_scr->flags&fMAZE)) &&
												   !(hero_scr->flags2&wfUP)) && (nextcombo_solid(up))))))
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
				if((_walkflag(x+4,y+((bigHitbox||!diagonalMovement)?-1:7),1,get_standing_z_state()) || _walkflag(x+12,y+((bigHitbox||!diagonalMovement)?-1:7),1,get_standing_z_state())
					|| ((y<=0) &&
					// Extra checks if Smart Screen Scrolling is enabled
					 (nextcombo_wf(up) || ((get_qr(qr_SMARTSCREENSCROLL)&&(!(hero_scr->flags&fMAZE)) &&
												   !(hero_scr->flags2&wfUP)) && (nextcombo_solid(up))))))
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
				if(tick_hover() && !ladderx && !laddery)
				{
					fall += gravity3;
					used_grav_or_termv = true;
				}
			}
			else if(fall+gravity3 > 0 && fall<=0
				&& try_hover())
				used_grav_or_termv = true;
			else if(!ladderx && !laddery && !getOnSideviewLadder() && !IsSideSwim())
			{
				fall += gravity3;
				used_grav_or_termv = true;
			}
		}
		else if(needFall)
		{
			if (get_qr(qr_BETTER_PLAYER_JUMP_ANIM) && (fall != 0 || hoverclk > 0))
				jumping++;
			used_grav_or_termv = true;
		}
	}
	else // Topdown gravity
	{
		if (!(moveflags & move_no_fake_z)) fakez-=fakefall/(spins && fakefall>0 ? 200:100);
		if (!(moveflags & move_no_real_z)) z-=fall/(spins && fall>0 ? 200:100);
		if(z>0||fakez>0)
		{
			switch(action)
			{
				case swimming:
				{
					set_dive(0);
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
		
		if(z<=0&&!(moveflags & move_no_real_z))
		{
			auto oldfall = fall;
			z = fall = 0;
			if (fakez <= 0 || (moveflags & move_no_fake_z)) 
			{
				if(oldfall > 0)
				{
					land_on_ground();
					
					stomping = true;
				}
			}
			if (fakez <= 0 || (moveflags & move_no_fake_z)) 
			{
				jumping = 0;
				if(check_pitslide(true) == -1)
				{
					hoverclk = 0;
					hoverflags = 0;
				}
				else tick_hover();
			}
		}
		else if(fakez<=0&&!(moveflags & move_no_fake_z))
		{
			auto oldfakefall = fakefall;
			fakez = fakefall = 0;
			if (z <= 0 || (moveflags & move_no_real_z))
			{
				if(oldfakefall > 0)
				{
					land_on_ground();
						
					stomping = true;
				}
			}
			if (z <= 0 || (moveflags & move_no_real_z)) 
			{
				jumping = 0;
				if(check_pitslide(true) == -1)
				{
					hoverclk = 0;
					hoverflags = 0;
				}
				else tick_hover();
			}
		}
		if(fall <= termv && !(moveflags & move_no_real_z) && z>0 || fakefall <= termv && !(moveflags & move_no_fake_z) && fakez > 0)
		{
			if(fall != 0 || fakefall != 0 || hoverclk>0)
				jumping++;
				
			if(hoverclk > 0)
			{
				if(tick_hover())
				{
					if (!(moveflags & move_no_real_z) && z > 0)
					{
						if (fall <= termv) fall += gravity3;
						used_grav_or_termv = true;
					}
					if (!(moveflags & move_no_fake_z) && fakez > 0)
					{
						if (fakefall <= termv) fakefall += gravity3;
						used_grav_or_termv = true;
					}
				}
			}
			else if (((fall + gravity3 > 0 && fall <= 0 && !(moveflags & move_no_real_z) && z > 0) || (fakefall + gravity3 > 0 && fakefall <= 0 && !(moveflags & move_no_fake_z) && fakez > 0))
				&& try_hover())
				used_grav_or_termv = true;
			else 
			{
				if (!(moveflags & move_no_real_z) && z > 0)
				{
					if (fall <= termv) fall += gravity3;
					used_grav_or_termv = true;
				}
				if (!(moveflags & move_no_fake_z) && fakez > 0)
				{
					if (fakefall <= termv) fakefall += gravity3;
					used_grav_or_termv = true;
				}
			}
		}
		if(fall > termv && !(moveflags & move_no_real_z) && z>0 || fakefall > termv && !(moveflags & move_no_fake_z) && fakez > 0)
		{
			if (get_qr(qr_BETTER_PLAYER_JUMP_ANIM) && (fall != 0 || fakefall != 0 || hoverclk > 0))
				jumping++;
			used_grav_or_termv = true;
		}
		if (fakez<0) fakez = 0;
		if (z<0) z = 0;
	}

	if (handle_termv())
		used_grav_or_termv = true;

	if (used_grav_or_termv && last_grav_boots_id > -1) // if the gravity boots affected the player's falling, charge their cost
		paymagiccost(last_grav_boots_id);
	
	for (int q = 0; q < MAXITEMS; ++q)
		if (item_cooldown[q] > 0)
			--item_cooldown[q];
	
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
	}
	
	if( lbunnyclock > 0 )
	{
		--lbunnyclock;
	}
	if(DMaps[cur_dmap].flags&dmfBUNNYIFNOPEARL)
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

	bool is_broken_behavior = replay_is_active() && replay_get_meta_str("sav") == "link_to_the_zelda_2_of_3.sav";
	bool fall_check = (fall == 0 || z > 0) && (fakefall == 0 || fakez > 0);
	if (is_broken_behavior)
		fall_check = (fall == 0 || z > 0) || (fakefall == 0 || fakez > 0);

	if(!is_on_conveyor && !(diagonalMovement||NO_GRIDLOCK) && fall_check && charging==0 && spins<=5
			&& action != gothit && !is_autowalking())
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
				int screen = it.first;
				int id = it.second;
				addenemy(screen,0,0,id,0);
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
									rpos_t targrpos = hooked_comborpos, plrpos = COMBOPOS_REGION_B(x+8,y+8);
									if (hooked_layerbits && is_valid_rpos(targrpos) && is_valid_rpos(plrpos))
									{
										int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
										for(int q = max_layer; q > -1; --q)
										{
											if(!(hooked_layerbits & (1<<q)))
												continue; //non-switching layer
											
											auto target_rpos_handle = get_rpos_handle(targrpos, q);
											auto player_rpos_handle = get_rpos_handle(plrpos, q);

											mapscr* target_scr = target_rpos_handle.scr;

											auto& cmb = target_rpos_handle.combo();
											int32_t srcfl = target_rpos_handle.sflag();
											auto& cmb2 = player_rpos_handle.combo();
											int32_t c = player_rpos_handle.data(),
													cs = player_rpos_handle.cset(),
													fl = player_rpos_handle.sflag();
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
														cmb, target_rpos_handle.cset(), it, thedropset, cmb.attribytes[2],
														cmb.attribytes[1] ? -1 : 0, cmb.attribytes[1], switchhookclk);
													br->switch_hooked = true;
													decorations.add(br);
													hooked_layerbits &= ~(0x101<<q); //this swap completed entirely
													hooked_undercombos[q] = -1;
													
													if(cmb.usrflags&cflag6)
													{
														target_rpos_handle.increment_data();
													}
													else
													{
														target_rpos_handle.set_data(target_scr->undercombo);
														target_rpos_handle.set_cset(target_scr->undercset);
														if(cmb.usrflags&cflag2)
															target_rpos_handle.set_sflag(0);
													}
												}
												else if(isPush)
												{
													//Simulate a block clicking into place
													movingblock mtemp;
													mtemp.clear();

													auto [mx, my] = COMBOXY_REGION(plrpos);

													mtemp.set(mx,my,target_rpos_handle.data(),target_rpos_handle.cset(),q,target_rpos_handle.sflag());
													mtemp.dir = getPushDir(target_rpos_handle.sflag());
													if(mtemp.dir < 0)
														mtemp.dir = getPushDir(cmb.flag);
													mtemp.clk = 1;
													mtemp.force_many = isFakePush;
													mtemp.no_icy = true;
													mtemp.animate(0);
													if((mtemp.bhole || mtemp.trigger)
														&& (fl == mfBLOCKTRIGGER || fl == mfBLOCKHOLE
															|| cmb2.flag == mfBLOCKTRIGGER
															|| cmb2.flag == mfBLOCKHOLE))
													{
														target_rpos_handle.set_data(target_scr->undercombo);
														target_rpos_handle.set_cset(target_scr->undercset);
														target_rpos_handle.set_sflag(0);
													}
													else
													{
														target_rpos_handle.set_data(c);
														target_rpos_handle.set_cset(cs);
														if(cmb.usrflags&cflag2)
															target_rpos_handle.set_sflag(fl);
														else
															target_rpos_handle.set_sflag(0);
													}
												}
												else
												{
													player_rpos_handle.set_data(target_rpos_handle.data());
													player_rpos_handle.set_cset(target_rpos_handle.cset());
													if(cmb.usrflags&cflag2)
														player_rpos_handle.set_sflag(target_rpos_handle.sflag());

													target_rpos_handle.set_data(c);
													target_rpos_handle.set_cset(cs);
													if(cmb.usrflags&cflag2)
														target_rpos_handle.set_sflag(fl);
												}
											}
											else if(isCuttableType(cmb.type)) //Break and drop effects
											{
												int32_t breakcs = target_rpos_handle.cset();
												if(isCuttableNextType(cmb.type)) //next instead of undercmb
												{
													target_rpos_handle.increment_data();
												}
												else
												{
													target_rpos_handle.set_data(target_scr->undercombo);
													target_rpos_handle.set_cset(target_scr->undercset);
													target_rpos_handle.set_sflag(0);
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

													auto [mx, my] = COMBOXY_REGION(plrpos);

													mtemp.set(mx,my,target_rpos_handle.data(),target_rpos_handle.cset(),q,target_rpos_handle.sflag());
													mtemp.dir = getPushDir(target_rpos_handle.sflag());
													if(mtemp.dir < 0)
														mtemp.dir = getPushDir(cmb.flag);
													mtemp.clk = 1;
													mtemp.animate(0);
													if(mtemp.bhole || mtemp.trigger)
													{
														target_rpos_handle.set_data(target_scr->undercombo);
														target_rpos_handle.set_cset(target_scr->undercset);
														target_rpos_handle.set_sflag(0);
													}
													else
													{
														target_rpos_handle.set_data(c);
														target_rpos_handle.set_cset(cs);
														target_rpos_handle.set_sflag(0);
													}
												}
												else
												{
													player_rpos_handle.set_data(target_rpos_handle.data());
													player_rpos_handle.set_cset(target_rpos_handle.cset());
													target_rpos_handle.set_data(c);
													target_rpos_handle.set_cset(cs);
												}
											}
										}
										if(switchhook_cost_item > -1)
											paymagiccost(switchhook_cost_item);
										zfix tx = x, ty = y;

										//Position the player at the combo
										std::tie(x, y) = COMBOXY_REGION(targrpos);

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
										if(itemsbuf[it->id].type == itype_fairy && itemsbuf[it->id].misc3)
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

	if (!get_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP))
	{
		if (getInput(btnL, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
			selectNextBWpn(SEL_LEFT);
		else if (getInput(btnR, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
			selectNextBWpn(SEL_RIGHT);
	}
	if (get_qr(qr_SELECTAWPN) && get_qr(qr_USE_EX1_EX2_INVENTORYSWAP))
	{
		if (getInput(btnEx3, INPUT_PRESS | INPUT_HERO_ACTION))
			selectNextAWpn(SEL_LEFT);
		else if (getInput(btnEx4, INPUT_PRESS | INPUT_HERO_ACTION))
			selectNextAWpn(SEL_RIGHT);
	}

	if (getInput(btnP, INPUT_PRESS | INPUT_HERO_ACTION))
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
			for (int32_t i = 0; i < Lwpns.Count(); i++)
			{
				weapon* w = ((weapon*)Lwpns.spr(i));

				if (w->id == wCByrna && !w->weapon_dying_frame)
					w->dead = 1;
			}
			//kill the sound effect for the orbits -Z 14FEB2019
			stop_sfx(itemsbuf[itemid].usesound);
		}
		else paymagiccost(itemid);
	}
	
	if(z == 0 && fakez == 0)
	{
		if (standing_z_offset)
		{
			standing_z_offset = false;
			yofs += 8;
		}
		if (standing_on_z > 0)
			standing_on_z = 0;
	}
	if (!isDiving() && standing_on_z < 0)
		standing_on_z = 0;
	
	if(!isSideViewHero())
	{
		int32_t tx = x.getInt()+8,
		    ty = y.getInt()+8;//(bigHitbox?8:12);
		if (unsigned(ty) < world_h && unsigned(tx) < world_w)
		{
			rpos_t rpos = COMBOPOS_REGION_B(tx, ty);
			for(int32_t q = 0; q < 3; ++q)
			{
				if (rpos == rpos_t::None) break;

				auto rpos_handle = get_rpos_handle(rpos, q);
				if (!rpos_handle.scr->is_valid()) continue;

				auto& cmb = rpos_handle.combo();
				
				int32_t b = 1;
				if(tx&8) b <<= 2;
				if(ty&8) b <<= 1;
				b |= (b<<4); //check equivalent effect flag too
				if((cmb.walk & b) != b) //not both solid and effecting
					continue;
				
				bool standing_switchblock = cmb.type == cCSWITCHBLOCK && (cmb.usrflags & cflag9);
				bool standing_combo = (cmb.genflags & cflag3) && !standing_switchblock;
				
				if (cmb.dive_under_level && isDiving())
				{
					// keep you from surfacing
					// also sets the 'standing_on_z' value, based on flippers
					set_dive(zc_max(diveclk, 2));
					if (standing_on_z >= -cmb.dive_under_level) // push you down further if the combo appeared on you or etc
						standing_on_z = -cmb.dive_under_level;
				}
				else if (standing_switchblock || standing_combo)
				{
					bool should_z_offs = standing_switchblock ? (cmb.usrflags&cflag10) : (cmb.genflags & cflag4);
					zfix cmb_z = standing_switchblock ? zslongToFix(cmb.attributes[2]) : cmb.z_height;
					if (!cmb_z) cmb_z = STANDING_Z_MAX;
					
					if (standing_on_z >= 0)
					{
						if (z == 0 && fakez == 0)
							standing_on_z = zc_max(standing_on_z, cmb_z);
						else if(get_standing_z_state() < cmb_z)
							standing_on_z += cmb_z - get_standing_z_state();
					}
					if(z == 0 && fakez == 0 && should_z_offs && !standing_z_offset)
					{
						standing_z_offset = true;
						yofs -= 8;
					}
				}
				else continue;
				
				break;
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
					if(bt->flags & BTFLAG_CURESHJINX)
						shieldjinxclk = 0;
					if(word max = std::max(toFill[0], std::max(toFill[1], toFill[2])))
					{
						int32_t itemid = find_bottle_for_slot(slot,true);
						stop_sfx(QMisc.miscsfx[sfxLOWHEART]); //stop heart beep!
						if(itemid > -1)
							sfx(itemsbuf[itemid].usesound,pan(x));
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
					FFCore.deallocateAllScriptOwned(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
					ALLOFF(true,true);
					if(lift_wpn)
					{
						delete lift_wpn;
						lift_wpn = nullptr;
					}
					GameFlags |= GAMEFLAG_NO_F6;
					if(!debug_enabled)
					{
						Paused=false;
					}
					if(!get_qr(qr_ONDEATH_RUNS_AFTER_DEATH_ANIM))
					{
						FFCore.runOnDeathEngine();
						FFCore.deallocateAllScriptOwned(ScriptType::Hero, SCRIPT_HERO_DEATH);
					}
					Playing = false;
					heroDeathAnimation();
					if(get_qr(qr_ONDEATH_RUNS_AFTER_DEATH_ANIM))
					{
						Playing = true;
						FFCore.runOnDeathEngine();
						FFCore.deallocateAllScriptOwned(ScriptType::Hero, SCRIPT_HERO_DEATH);
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
	
	if(fallclk)
	{
		action=falling; FFCore.setHeroAction(falling);
	}
	else if(hopclk)
	{
		action=hopping; FFCore.setHeroAction(hopping);
	}
	
	if(isSwimming() && (liftflags&LIFTFL_DIS_SWIMMING) && !get_qr(qr_BROKEN_LIFTSWIM))
		drop_liftwpn();
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
	if (active_shield_id > -1 && !shield_active && on_cooldown(active_shield_id))
		active_shield_id = -1;
	bool sh = active_shield_id > -1;
	itemdata const& shield = itemsbuf[active_shield_id];
	//Handle direction forcing. This runs every frame so that scripts can interact with dir still.
	shield_forcedir = -1;
	if(sh && action != rafting && (shield.flags & item_flag11)) //Lock Dir
	{
		shield_forcedir = dir;
	}
	if(sh != shield_active) //Toggle active shield on/off
	{
		shield_active = sh;
		if(sh) //Toggle active shield on
		{
			sfx(shield.usesound2); //'Activate' sfx
			start_cooldown(active_shield_id);
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
			optional<combined_handle_t> comb_handle;
			int32_t water = iswaterex_z3(MAPCOMBO(x.getInt()+7.5,y.getInt()+12), -1, x.getInt()+7.5,y.getInt()+12, true, false, true, false, true, &comb_handle);
			if(drownCombo && water != drownCombo)
			{
				comb_handle = nullopt;
				water = drownCombo;
			}
			
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
			if(comb_handle)
				do_trigger_ctype_causes(*comb_handle);
			go_respawn_point();
			hclk=48;
			check_on_hit();
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
		set_dive(0);
		
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
			set_dive(0);
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
			// When the wind weapon goes away, check if the player has been brought to the edge of the screen
			// by the wind. If so, push them one more pixel to trigger the screen scrolling code.
			else if (dir==up && y<=viewport.top()) y=viewport.top()-1;
			else if (dir==down && y>=viewport.bottom()-16) y=viewport.bottom()-16+1;
			else if (dir==left && x<=viewport.left()) x=viewport.left()-1;
			else if (dir==right && x>=viewport.right()-16) x=viewport.right()-16+1;
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
		else
		{
			auto wind = Lwpns.spr(i);
			x = wind->x;
			y = wind->y;
			dir = wind->dir;
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
		
		if (_walkflag(x+7,y+(bigHitbox?6:11),1,get_standing_z_state())
                || _walkflag(x+7,y+(bigHitbox?9:12),1,get_standing_z_state())
		|| _walkflag(x+8,y+(bigHitbox?6:11),1,get_standing_z_state())
                || _walkflag(x+8,y+(bigHitbox?9:12),1,get_standing_z_state())) isthissolid = true;
		if ((get_qr(qr_NO_HOPPING) || CanSideSwim()) && !isthissolid) //Since hopping won't be set with this on, something needs to kick Hero out of water...
		{
			if(!iswaterex_z3(MAPCOMBO(x.getInt()+4,y.getInt()+9), -1, x.getInt()+4,y.getInt()+9, true, false)||!iswaterex_z3(MAPCOMBO(x.getInt()+4,y.getInt()+15), -1, x.getInt()+4,y.getInt()+15, true, false)
			|| !iswaterex_z3(MAPCOMBO(x.getInt()+11,y.getInt()+9), -1, x.getInt()+11,y.getInt()+9, true, false)||!iswaterex_z3(MAPCOMBO(x.getInt()+11,y.getInt()+15), -1, x.getInt()+11,y.getInt()+15, true, false))
			{
				hopclk=0;
				set_dive(0);
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
							check_on_hit();
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
		else
		{
			sliding = 0;
			moveheroOld();
		}
	}
	if (isSideViewHero() && slopeid && hoverclk < 1 && !inair && fall == 0 && !IsSideSwim() && justmoved <= 0 && !is_autowalking())
	{
		if (!on_sideview_solid_oldpos(this, false, 0))
		{
			zfix dy = 0;
			for (zfix q = 0; q <= 4; ++q)
			{
				if (on_sideview_solid_oldpos(this, false, 0, 0, q))
				{
					dy = q;
					break;
				}
			}
			dy = binary_search_zfix(0, dy, [&](zfix val, zfix& retval)
			{
				if (on_sideview_solid_oldpos(this, false, 0, 0, val))
				{
					retval = val;
					return BSEARCH_CONTINUE_TOWARD0;
				}
				else return BSEARCH_CONTINUE_AWAY0;
			});
			if (dy) push_move(0, dy);
		}
	}
	
	if(shield_forcedir > -1 && action != rafting)
		dir = shield_forcedir;
	
	
	// check for ladder removal
	if(diagonalMovement||NO_GRIDLOCK)
	{
		if(ladderx+laddery)
		{
			if(ladderdir<=down)
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
		if(hero_scr->room==rGANON)
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
		if(!is_autowalking())
		{
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
		}
		
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
	
	if((!activated_timed_warp) && (origin_scr->timedwarptics>0))
	{
		origin_scr->timedwarptics--;
		
		if(origin_scr->timedwarptics==0)
		{
			activated_timed_warp=true;
			
			if(origin_scr->flags4 & fTIMEDDIRECT)
			{
				setpit();
			}
			
			int32_t index2 = 0;
			
			if(origin_scr->flags5 & fRANDOMTIMEDWARP) index2=zc_oldrand()%4;
			
			sdir = dir;
			dowarp(origin_scr, 1, index2);
		}
	}
	
	// Global Combo Effects (AUTO STUFF)
	bool awarp = false;
	for_some_rpos([&](const rpos_handle_t& rpos_handle) {
		if (!get_qr(qr_AUTOCOMBO_ANY_LAYER))
		{
			if (rpos_handle.layer > 2) return false;
			if (rpos_handle.layer == 1 && !get_qr(qr_AUTOCOMBO_LAYER_1)) return true;
			if (rpos_handle.layer == 2 && !get_qr(qr_AUTOCOMBO_LAYER_2)) return true;
		}
		int32_t ind=0;
		
		//AUTOMATIC TRIGGER CODE
		trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
			return trig.trigger_flags.get(TRIGFLAG_AUTOMATIC);
		});
		newcombo const& cmb = rpos_handle.combo();
		
		//AUTO WARP CODE
		if (!(cmb.only_gentrig))
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
		if (awarp)
		{
			if (rpos_handle.scr->flags5 & fDIRECTAWARP)
			{
				setpit();
			}
			
			sdir = dir;
			dowarp(rpos_handle.scr, 1, ind);
			return false;
		}

		return true;
	});
	
	awarp=false;
	
	for_some_ffcs([&](const ffc_handle_t& ffc_handle) {
		int32_t ind=0;
		
		trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
			return trig.trigger_flags.get(TRIGFLAG_AUTOMATIC);
		});
		auto& cmb = ffc_handle.combo();
		
		if(!(cmb.only_gentrig))
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
			if(ffc_handle.scr->flags5&fDIRECTAWARP)
			{
				setpit();
			}
			
			sdir = dir;
			dowarp(ffc_handle.scr, 1, ind, 0);
			return false;
		}

		return true;
	});

	zfix dx, dy;
	if (sideview_mode() && !on_sideview_solid_oldpos(this, false, 1) && on_sideview_solid_oldpos(this, false, 2) && !walk_through_walls && !is_autowalking())
	{
		if (slide_slope(this, dx, dy, slopeid))
		{
			onplatid = 1;
			if (dx || dy) push_move(dx, dy);
		}
	}
	if (onplatid <= 0) slopeid = 0;
	else --onplatid;
	bool onplatform = (on_sideview_solid_oldpos(this, false, 1) && !getInput(btnUp, INPUT_HERO_ACTION));
	if(!walk_through_walls && !is_autowalking())
	{
		for (auto q = 0; check_slope(this, true) && q < 2; ++q)
		{
			dx = 0;
			dy = 0;
			if (check_slope(this, true))
			{
				slope_info const& s = get_slope(this, true).get_info();
				bool staircheck = false;
				if (s.slope() != slopeid && slopeid) staircheck = true;
				if (onplatform) staircheck = true;
				slope_push_int(s, this, dx, dy, staircheck, platformfell2);
				
				if (dx || dy) 
				{
					reset_hookshot();
					int32_t pushret = push_move(dx,dy);
					onplatform = (on_sideview_solid_oldpos(this, false, 1) && !getInput(btnUp, INPUT_HERO_ACTION));
					if (s.slope() != slopeid && slopeid) staircheck = true;
					if (onplatform) staircheck = true;
					if(sideview_mode() && slopeid)
						onplatid = 0;
					if (pushret == 1)
					{
						dx = -1;
						dy = 0;
						slope_push_int(s, this, dx, dy, staircheck);
						push_move(0,dy);
					}
					if (pushret == 2)
					{
						dx = 0;
						dy = -1;
						slope_push_int(s, this, dx, dy, staircheck);
						push_move(dx,0);
					}
				}
			}
		}
	}
	
	if(ffwarp)
	{
		if(ffpit)
		{
			ffpit=false;
			setpit();
		}
		
		ffwarp=false;
		dowarp(hero_scr,1,0);
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
		
		if(isdungeon() && action!=freeze && action != sideswimfreeze && loaded_guys && !inlikelike && !diveclk && action!=rafting && !lstunclock && !is_conveyor_stunned && !is_autowalking())
		{
			int x0 = x.getInt() % 256;
			int y0 = y.getInt() % 176;

			if (((dtype == dBOMBED) ? getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) : dir == up) && ((diagonalMovement || NO_GRIDLOCK) ? x0 > 112 && x0 < 128:x0 == 120) && y0 <= 32 && hero_scr->door[0] == dtype)
			{
				walk=true;
				dir=up;
			}
			
			if (((dtype == dBOMBED) ? getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) : dir == down) && ((diagonalMovement || NO_GRIDLOCK) ? x0 > 112 && x0 < 128:x0 == 120) && y0 >= 128 && hero_scr->door[1] == dtype)
			{
				walk=true;
				dir=down;
			}
			
			if (((dtype == dBOMBED) ? getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) : dir == left) && x0 <= 32 && ((diagonalMovement || NO_GRIDLOCK) ? y0 > 72 && y0 < 88:y0 == 80) && hero_scr->door[2] == dtype)
			{
				walk=true;
				dir=left;
			}
			
			if (((dtype == dBOMBED) ? getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) : dir == right) && x0 >= 208 && ((diagonalMovement || NO_GRIDLOCK) ? y0 > 72 && y0 < 88:y0 == 80) && hero_scr->door[3] == dtype)
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
				sfx(hero_scr->secretsfx);
				if(!get_qr(qr_WALKTHROUGHWALL_NO_DOORSTATE))
					set_doorstate(current_screen, dir);
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

	if (getInput(btnS, INPUT_PRESS | INPUT_HERO_ACTION))
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
				trigger_save(combobuf[last_savepoint_id], hero_scr);
			else save_game((hero_scr->flags4&fSAVEROOM) != 0, save_type); //sanity? 
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
	bool actual = set && !walk_through_walls; //not solid when noclipping
	bool ret = solid_object::setSolid(actual);
	solid = set;
	return ret;
}

void HeroClass::solid_push(solid_object* obj)
{
	if(obj == this) return; //can't push self
	
	zfix dx, dy;
	int32_t hdir = -1;
	solid_push_int(obj,dx,dy,hdir,!on_ffc_platform());
	
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
    if ( get_qr(qr_NEVERDISABLEAMMOONSUBSCREEN) || itemsbuf[game->forced_awpn].type == itype_bomb || itemsbuf[game->forced_bwpn].type == itype_bomb || itemsbuf[game->forced_xwpn].type == itype_bomb || itemsbuf[game->forced_ywpn].type == itype_bomb) return;
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

int HeroClass::onWater(bool drownonly)
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
		rpos_handle_t rpos_handles[4];
		rpos_handles[0] = get_rpos_handle_for_world_xy(x1, y1, 0);
		rpos_handles[1] = get_rpos_handle_for_world_xy(x1, y2, 0);
		rpos_handles[2] = get_rpos_handle_for_world_xy(x2, y1, 0);
		rpos_handles[3] = get_rpos_handle_for_world_xy(x2, y2, 0);
		
		types[0] = rpos_handles[0].ctype();
		if (auto ffc_handle = getFFCAt(x1, y1))
			types[0] = ffc_handle->ctype();
		
		types[1] = rpos_handles[1].ctype();
		if (auto ffc_handle = getFFCAt(x1, y2))
			types[1] = ffc_handle->ctype();

		types[2] = rpos_handles[2].ctype();
		if (auto ffc_handle = getFFCAt(x2, y1))
			types[2] = ffc_handle->ctype();

		types[3] = rpos_handles[3].ctype();
		if (auto ffc_handle = getFFCAt(x2, y2))
			types[3] = ffc_handle->ctype();
		
		auto rpos_handle = get_rpos_handle_for_world_xy((x2+x1)/2, (y2+y1)/2, 0);
		auto typec = rpos_handle.ctype();
		auto cid = rpos_handle.data();
		if (auto ffc_handle = getFFCAt((x2+x1)/2, (y2+y1)/2))
		{
			typec = ffc_handle->ctype();
			cid = ffc_handle->data();
		}
		
		if(combo_class_buf[types[0]].water && combo_class_buf[types[1]].water &&
				combo_class_buf[types[2]].water && combo_class_buf[types[3]].water && combo_class_buf[typec].water)
			water = cid;
	}
	if(water > 0)
	{
		if(!drownonly) return water;
		if(current_item(itype_flippers) <= 0 || current_item(itype_flippers) < combobuf[water].attribytes[0] || ((combobuf[water].usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))) 
		{
			return water;
		}
	}
	return 0;
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
	if (on_cooldown(mirrorid)) return;
	
	if((hero_scr->flags9&fDISABLE_MIRROR) || !(checkbunny(mirrorid) && checkmagiccost(mirrorid)))
	{
		item_error();
		return;
	}
	static const int32_t sens = 4; //sensitivity of 'No Mirror' combos (0 most, 8 least)
	rpos_t rposes[] = {COMBOPOS_REGION_B(x+sens,y+sens), COMBOPOS_REGION_B(x+sens,y+15-sens),
		COMBOPOS_REGION_B(x+15-sens,y+sens), COMBOPOS_REGION_B(x+15-sens,y+15-sens)};
	for(auto rpos : rposes)
	{
		if (rpos == rpos_t::None)
			continue;

		if(HASFLAG_ANY(mfNOMIRROR, rpos)) //"No Mirror" flag touching the player
		{
			item_error();
			return;
		}
	}
	
	itemdata const& mirror = itemsbuf[mirrorid];
	if(DMaps[cur_dmap].flags & dmfMIRRORCONTINUE)
	{
		paymagiccost(mirrorid);
		start_cooldown(mirrorid);
		if(mirror.usesound2) sfx(mirror.usesound2);
		
		doWarpEffect(mirror.misc2, true);
		if(mirror.flags & item_flag2) //Act as F6->Continue
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
		int32_t destdmap = DMaps[cur_dmap].mirrorDMap;
		int32_t offscr = cur_screen - DMaps[cur_dmap].xoff;
		if(destdmap < 0)
			return;
		int32_t destscr = DMaps[destdmap].xoff + offscr;
		if((offscr%16 != destscr%16) || unsigned(destscr) >= 0x80)
			return;
		paymagiccost(mirrorid);
		start_cooldown(mirrorid);
		
		//Store some values to restore if 'warp fails'
		int32_t tLastEntrance = lastentrance,
				tLastEntranceDMap = lastentrance_dmap,
				tContScr = game->get_continue_scrn(),
				tContDMap = game->get_continue_dmap(),
				tPortalDMap = game->saved_mirror_portal.srcdmap;
		int32_t sourcescr = cur_screen, sourcedmap = cur_dmap;
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
		else if(mirror.flags & item_flag1) //Place portal!
		{
			//Place the portal
			game->set_portal(sourcedmap, destdmap, offscr, x.getZLong(), y.getZLong(), mirror.usesound, mirror.misc1, mirror.wpn);
			//Since it was placed after loading this screen, load the portal object now
			game->load_portal();
			//Don't immediately trigger the warp back
			mirror_portal.prox_active = false;
			
			//Set continue point
			if(cur_dmap != game->get_continue_dmap())
			{
				game->set_continue_scrn(DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff);
			}
			game->set_continue_dmap(cur_dmap);
			lastentrance_dmap = cur_dmap;
			lastentrance = game->get_continue_scrn();
		}
	}
}

void HeroClass::handle_passive_buttons()
{
	if(no_control()) return;
	do_liftglove(-1,true);
	do_jump(-1,true);
}

void HeroClass::for_each_rpos_stood_on(std::function<void(const rpos_handle_t&)> proc)
{
	auto rpos = COMBOPOS_REGION_B(x+8, y+(sideview_mode()?16:12));
	auto rpos2 = sideview_mode() ? COMBOPOS_REGION_B(x+8, y+12) : rpos_t::None;
	if (rpos != rpos_t::None || rpos2 != rpos_t::None)
	{
		for (int layer = 0; layer < 7; ++layer)
		{
			if (rpos != rpos_t::None)
			{
				auto handle = get_rpos_handle(rpos, layer);
				if (!sideview_mode() || !(handle.combo().genflags & cflag5))
					proc(handle);
			}
			if (rpos2 != rpos_t::None)
			{
				auto handle = get_rpos_handle(rpos2, layer);
				if (handle.combo().genflags & cflag5) // sideview_mode() already checked above
					proc(handle);
			}
		}
	}
}

void HeroClass::land_on_ground()
{
	if (fakez<0) fakez = 0;
	if (z<0) z = 0;
	
	bool played_land_sfx = false;
	if(get_qr(qr_OLD_LANDING_SFX))
	{
		if(!sideview_mode() && ((iswaterex_z3(MAPCOMBO(x,y+8), -1, x, y+8, true, false) && ladderx<=0 && laddery<=0) || COMBOTYPE(x,y+8)==cSHALLOWWATER))
			sfx(WAV_ZN1SPLASH,pan(x));
		played_land_sfx = true;
	}

	for_each_rpos_stood_on([&](const rpos_handle_t& handle)
		{
			auto& cmb = handle.combo();
			byte csfx = cmb.sfx_landing;
			if (csfx && !get_qr(qr_OLD_LANDING_SFX))
			{
				sfx(csfx, pan(x));
				played_land_sfx = true;
			}
			trig_each_combo_trigger(handle, [&](combo_trigger const& trig){
				return trig.trigger_flags.get(TRIGFLAG_PLAYERLANDHERE);
			});
		});
	
	if(!played_land_sfx && QMisc.miscsfx[sfxHERO_LANDS])
		sfx(QMisc.miscsfx[sfxHERO_LANDS], pan(x));
	
	
	for_some_rpos([&](const rpos_handle_t& rpos_handle) {
		trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
			return trig.trigger_flags.get(TRIGFLAG_PLAYERLANDANYWHERE);
		});
		
		return true;
	});
	for_some_ffcs([&](const ffc_handle_t& ffc_handle) {
		trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
			return trig.trigger_flags.get(TRIGFLAG_PLAYERLANDANYWHERE);
		});
		
		return true;
	});
}

static bool did_passive_jump = false;
bool HeroClass::do_jump(int32_t jumpid, bool passive)
{
	if(passive) did_passive_jump = false;
	else if(did_passive_jump) return false; //don't jump twice in the same frame
	
	if (on_cooldown(jumpid)) return false;
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
		if(!getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK | INPUT_PEEK))
			return false; //not pressed
	}
	
	paymagiccost(jumpid);
	start_cooldown(jumpid);
	
	if (itm.flags & item_flag6) // delayed effect on release, store the jump id
	{
		current_rocs_jump_id = jumpid;
		released_jump_button = false;
	}
	
	if(!standing)
	{
		++extra_jump_count;
		fall = 0;
		fakefall = 0;
		if(hoverclk > 0)
			hoverclk = -hoverclk;
	}
	if(itm.flags & item_flag1)
		setFall(fall - itm.power);
	else setFall(fall - (FEATHERJUMP*(itm.power+2)));
	coyotetime = 65535; //jumped, so no coyotetime
	setOnSideviewLadder(false);
	
	// Reset the ladder, unless on an unwalkable combo
	if((ladderx || laddery) && !(_walkflag(ladderx,laddery,0,get_standing_z_state())))
		reset_ladder();
	
	if(itm.usesound)
		sfx(itm.usesound,pan(x));
	
	if(passive)
	{
		did_passive_jump = true;
		getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK); //eat buttons
	}
	return true;
}
void HeroClass::drop_liftwpn()
{
	if(!lift_wpn) return;
	
	handle_lift(false); //sets position properly, accounting for large weapons
	int liftid = last_lift_id ? *last_lift_id : current_item_id(itype_liftglove,true,true);
	itemdata const& glove = itemsbuf[liftid];
	if(isSideViewGravity())
	{
		lift_wpn->moveflags |= move_no_fake_z;
	}
	else
	{
		auto lheight = liftheight+z+fakez;
		if(glove.flags & item_flag1)
		{
			lift_wpn->z = 0;
			lift_wpn->fakez = lheight;
			lift_wpn->moveflags |= move_no_real_z;
		}
		else
		{
			lift_wpn->z = lheight;
			lift_wpn->moveflags |= move_no_fake_z;
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
	if(!lift_wpn)
		last_lift_id.reset();
	if(liftid < 0)
	{
		if(last_lift_id && can_lift(*last_lift_id))
			liftid = *last_lift_id;
		else liftid = current_item_id(itype_liftglove,true,true);
	}
	if(!can_lift(liftid)) return;
	if (on_cooldown(liftid)) return;
	itemdata const& glove = itemsbuf[liftid];
	byte intbtn = byte(glove.misc1&0xFF);
	if(passive)
	{
		if(!getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK | INPUT_PEEK))
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
				getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK); //eat buttons
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
			if(lift_wpn->angular)
				lift_wpn->angle = WrapAngle(DirToRadians(dir));
			
			lift_wpn->doAutoRotate(false, true);
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
						lift_wpn->fall = zc_min(basestep,lift_wpn->get_terminalv_fall());
						lift_wpn->step = 0;
						break;
				}
				lift_wpn->moveflags |= move_no_fake_z;
			}
			else
			{
				auto lheight = liftheight+z+fakez;
				if(glove.flags & item_flag1)
				{
					lift_wpn->z = 0;
					lift_wpn->fakez = lheight;
					lift_wpn->fakefall = -glove.misc3;
					lift_wpn->moveflags |= move_no_real_z;
				}
				else
				{
					lift_wpn->z = lheight;
					lift_wpn->fall = -glove.misc3;
					lift_wpn->moveflags |= move_no_fake_z;
				}
			}
			Lwpns.add(lift_wpn);
			lift_wpn = nullptr;
			last_lift_id.reset();
			if(glove.usesound2)
				sfx(glove.usesound2,pan(x));
		}
		else last_lift_id = liftid;
		
		if(passive)
		{
			getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK); //eat buttons
		}
		start_cooldown(liftid);
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
			if((w->lift_level && w->lift_level <= glove.level))
			{
				auto tmpflags = w->misc_wflags;
				w->misc_wflags &= ~WFLAG_NO_COLL_WHEN_STILL;
				bool hit = w->hit(hx,hy,0,hw,hh,1);
				w->misc_wflags = tmpflags;
				if(!hit)
					continue;
				if(glove.usesound)
					sfx(glove.usesound,pan(x));
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
		rpos_t rpos = COMBOPOS_REGION_B(bx, by);
		rpos_t rpos2 = COMBOPOS_REGION_B(bx2, by2);
		
		for(auto lyr = 6; lyr >= 0; --lyr)
		{
			if(rpos != rpos_t::None)
			{
				auto rpos_handle = get_rpos_handle(rpos, lyr);
				auto& cmb = rpos_handle.combo();	
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
	
	if(!lifted)
	{
		last_lift_id.reset();
		return;
	}
	last_lift_id = liftid;
	
	if(!paidmagic)
	{
		paidmagic = true;
		paymagiccost(liftid);
	}
	start_cooldown(liftid);
	set_liftflags(liftid);
	if(passive)
		getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK); //eat buttons
	return;
}
void HeroClass::handle_lift(bool dec)
{
	if(lift_wpn)
	{
		lift_wpn->fakez = 0;
		lift_wpn->dir = dir;
	}
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
			if(glove.flags & item_flag2)
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

void HeroClass::check_on_hit() // Called when the player is hit
{
	if(lift_wpn && (liftflags & LIFTFL_DROP_ON_HIT))
		drop_liftwpn();
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
	rpos_t plrpos = COMBOPOS_REGION_B(x+8, y+8);
	if(hooked_comborpos != rpos_t::None && plrpos != rpos_t::None)
	{
		int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
		hooked_layerbits = 0;
		for(auto q = 0; q < 7; ++q)
			hooked_undercombos[q] = -1;
		
		int target_pos = RPOS_TO_POS(hooked_comborpos);
		int player_pos = RPOS_TO_POS(plrpos);
		
		for(auto q = max_layer; q > -1; --q)
		{
			auto target_pos_handle = get_rpos_handle(hooked_comborpos, q);
			auto player_pos_handle = get_rpos_handle(plrpos, q);
			
			mapscr* player_scr = player_pos_handle.scr;
			mapscr* target_scr = target_pos_handle.scr;

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
					if(zc_max(1,itemsbuf[(w && w->parentitem>-1) ? w->parentitem : current_item_id(itype_switchhook)].level) < cmb.attribytes[0])
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
					&& hasMainGuy(target_scr->screen))
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
				auto [decx, decy] = COMBOXY_REGION(hooked_comborpos);
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
	if (on_cooldown(itemid)) return false;
	itemdata const& itm = itemsbuf[itemid];
	if(((dir==up && y<24) || (dir==down && y>world_h-48) ||
			(dir==left && x<32) || (dir==right && x>world_w-48)) && !(get_qr(qr_ITEMSONEDGES) || inlikelike))
		return false;
	
	bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
	if(liftonly)
	{
		dowpn = -1;
		switch(itm.type)
		{
			case itype_bomb:
			case itype_sbomb:
				if(itm.flags & item_flag4)
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
	if (IsSideSwim() && (itm.flags & item_sideswim_disabled)) return false;
	
	switch(itm.type)
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
			start_cooldown(itemid);
			
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
				start_cooldown(itemid);

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
				bool check_jinxes = true;
				if(get_qr(qr_NO_BOTTLE_IF_ANY_COUNTER_FULL))
				{
					for(int q = 0; q < 3; ++q)
					{
						if(bt->counter[q] > -1)
						{
							check_jinxes = false;
							if(!toFill[q])
							{
								run = false;
								break;
							}
						}
					}
				}
				if(check_jinxes)
				{
					if((bt->flags & BTFLAG_CURESWJINX) && swordclk)
						run = true;
					else if((bt->flags & BTFLAG_CUREITJINX) && itemclk)
						run = true;
					else if((bt->flags & BTFLAG_CURESHJINX) && shieldjinxclk)
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
					if(bt->flags & BTFLAG_CURESHJINX)
						shieldjinxclk = 0;
					if(!paidmagic)
					{
						paymagiccost(itemid);
						start_cooldown(itemid);
					}
					stop_sfx(QMisc.miscsfx[sfxLOWHEART]); //stop heart beep!
					sfx(itm.usesound,pan(x));
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
			if(!msg_active)
			{
				if(play_combo_string(itm.misc1, current_screen))
				{
					sfx(itm.usesound);
					paymagiccost(itemid);
					start_cooldown(itemid);
				}
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
					(cur_screen >= 128 ? special_warp_return_scr : hero_scr)->room==rP_SHOP &&
					(cur_screen >= 128 ? special_warp_return_scr : hero_scr)->guy &&
					((cur_screen<128&&!(DMaps[cur_dmap].flags&dmfGUYCAVES))
						||(cur_screen>=128&&DMaps[cur_dmap].flags&dmfGUYCAVES)) &&
					checkbunny(itemid)
				)
			{
				int32_t usedid = getItemID(itemsbuf, itype_letter,i_letter+1);
				
				if(usedid != -1)
					getitem(usedid, true, true);
					
				sfx((cur_screen >= 128 ? special_warp_return_scr : hero_scr)->secretsfx);
				setupscreen();
				action=none; FFCore.setHeroAction(none);
				start_cooldown(itemid);
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
			start_cooldown(itemid);
			sfx(itm.usesound);
			
			if(dir==up || dir==right)
				++blowcnt;
			else
				--blowcnt;

			uint32_t frames_to_wait = 0;
			if (replay_is_active())
			{
				// How long an sfx takes to play (`sfx_allocated`) is not deterministic,
				// use a fixed number of frames in replay mode.
				// This has changed over time.
				if (replay_version_check(0, 26))
				{
					frames_to_wait = 180;
				}
				else if (replay_version_check(27, 31))
				{
					frames_to_wait = 0;
				}
				else
				{
					SAMPLE* sample = sfx_get_sample(itm.usesound);
					if (sample && sample->freq)
						frames_to_wait = 60 * sample->len / sample->freq;
				}
			}

			int sfx_count = 0;
			while ((!replay_is_active() && sfx_allocated(itm.usesound)) || (replay_is_active() && sfx_count < frames_to_wait))
			{
				sfx_count += 1;
				advanceframe(true);
				
				if(Quit)
					return false;
			}
			
			Lwpns.add(new weapon(x,y-fakez,z,wWhistle,0,0,dir,itemid,getUID(),false,0,1,0));
			
			if((whistleflag=trigger_secrets_if_flag(x,y,mfWHISTLE,get_qr(qr_PERMANENT_WHISTLE_SECRETS))))
				didstuff |= did_whistle;
				
			if((didstuff&did_whistle && itm.flags&item_flag1) || cur_screen>=128)
				return false;
				
			if(itm.flags&item_flag1) didstuff |= did_whistle;
			
			if((hero_scr->flags&fWHISTLE) || (hero_scr->flags7 & fWHISTLEWATER)
					|| (hero_scr->flags7&fWHISTLEPAL))
			{
				whistleclk=0;                                       // signal to start drying lake or doing other stuff
			}
			else
			{
				int32_t where = itm.misc1;
				
				if(where>right) where=dir^1;
				
				if(((DMaps[cur_dmap].flags&dmfWHIRLWIND && TriforceCount()) || DMaps[cur_dmap].flags&dmfWHIRLWINDRET) &&
						itm.misc2 >= 0 && itm.misc2 <= 8 && !whistleflag)
				{
					zfix windx = where == left ? (zfix)(viewport.right()-16) : where == right ? (zfix)viewport.left() : x;
					zfix windy = where == down ? (zfix)viewport.top() : where == up ? zfix(viewport.bottom()-16) : y;
					Lwpns.add(new weapon(
						windx,
						windy,
						(zfix)0,
						wWind,
						0, //type
						0,
						where,
						itemid,getUID(),false,false,true,0)); //last arg is byte special, used to override type for wWind for now. -Z 18JULY2020
				}
										 
				whistleitem=itemid;
			}
			
			ret = false;
		}
		break;
		
		case itype_bomb: case itype_sbomb:
		{
			bool sbomb = (itm.type == itype_sbomb);
			auto lit_ty = sbomb ? wLitSBomb : wLitBomb;
			auto boom_ty = sbomb ? wSBomb : wBomb;
			//Remote detonation
			if(Lwpns.idCount(lit_ty) >= zc_max(itm.misc2,1))
			{
				weapon *ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(lit_ty)));
				
				while(Lwpns.idCount(lit_ty) && ew->misc == 0)
				{
					//If this ever needs a version check, in the future. -z
					if (boom_ty == wBomb && (FFCore.getQuestHeaderInfo(vZelda) > 0x250 || ( FFCore.getQuestHeaderInfo(vZelda) == 0x250 && FFCore.getQuestHeaderInfo(vBuild) > 31)))
					{
						if ( ew->power > 1 ) //Don't reduce 1 to 0. -Z
							ew->power *= 0.5; //Remote bombs were dealing double damage. -Z
					}
					ew->misc=50;
					ew->clk=ew->misc-3;
					ew->id=boom_ty;
					ew = (weapon*)(Lwpns.spr(Lwpns.idFirst(lit_ty)));
				}
				
				deselectbombs(sbomb);
				return false;
			}
			
			if((itm.flags & item_flag4) && lift_wpn)
			{
				do_liftglove(-1,false); //Throw the already-held weapon
				start_cooldown(itemid);
				return false;
			}
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			start_cooldown(itemid);
				
			if(itm.misc1>0) // If not remote bombs
				deselectbombs(sbomb);
				
			if(isdungeon())
			{
				wy=zc_max(wy,16);
			}
			
			weapon* wpn = new weapon((zfix)wx,(zfix)wy,(zfix)wz,lit_ty,itm.level,
				itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true);
			bool lifted = false;
			if(itm.flags & item_flag4)
			{
				auto liftid = current_item_id(itype_liftglove);
				if(liftid > -1 && (itm.weap_data.lift_level <= itemsbuf[liftid].level))
				{
					lift(wpn, itm.weap_data.lift_time, itm.weap_data.lift_height);
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
			if (bookid > -1 && on_cooldown(bookid))
				bookid = -1;
			bool paybook = (bookid>-1 && checkbunny(bookid) && checkmagiccost(bookid));
			
			if(!(itm.flags&item_flag1) && !paybook)  //Can the wand shoot without the book?
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
				type = bookid != -1 ? current_item(itype_book) : itm.level;
				pow = (bookid != -1 ? current_item_power(itype_book) : itm.power)*game->get_hero_dmgmult();
			}
			else
			{
				type = (bookid != -1 && paybook) ? current_item(itype_book) : itm.level;
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
			{
				paymagiccost(bookid);
				start_cooldown(bookid);
			}
			
			start_cooldown(itemid);
				
			if(bookid != -1)
			{
				if (( itemsbuf[bookid].flags & item_flag4 ))
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
			
			start_cooldown(itemid);

			int temppower;
			if(itm.flags & item_flag2)
			{
				temppower=game->get_hero_dmgmult()*itm.power;
				temppower=temppower*itm.misc2;
				temppower=temppower/100;
			}
			else
			{
				temppower = game->get_hero_dmgmult()*itm.misc2;
			}

			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBeam,itm.level,temppower,dir,itemid,getUID(),false,false,true));
			sfx(itm.usesound2, pan(wx));
		}
		break;
		
		case itype_candle:
		{
			int32_t countid = itemid;
			if(get_qr(qr_CANDLES_SHARED_LIMIT))
				countid = -itype_candle;
			if(itm.flags&item_flag1 && usecounts[countid] >= zc_max(1, itm.misc3))
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
			start_cooldown(itemid);
			
			if(itm.flags&item_flag1) ++usecounts[countid];
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wFire,
								 //(itm.fam_type > 1), //To do with combo flags ... Needs to be changed to fix ->Level for wFire
								 (itm.level), //To do with combo flags ... Needs to be changed to fix ->Level for wFire
								 itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
			sfx(itm.usesound,pan(wx));
			attack=wFire;
		}
		break;
		
		case itype_script1: case itype_script2: case itype_script3: case itype_script4: case itype_script5:
		case itype_script6: case itype_script7: case itype_script8: case itype_script9: case itype_script10:
		{
			int32_t wtype = wScript1 + (itm.type-itype_script1);
			if(Lwpns.idCount(wtype))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
			
			if(!get_qr(qr_CUSTOMWEAPON_IGNORE_COST))
				paymagiccost(itemid);
			start_cooldown(itemid);
			
			zfix wpnstep = zfix(itm.misc1)/100;
			if(replay_version_check(0,30))
				wpnstep = itm.misc1/100;
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wtype,itm.level,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
			if(!(itm.weap_data.flags & wdata_set_step))
				((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = wpnstep;
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
			start_cooldown(itemid);
			
			zfix wpnstep = zfix(itm.misc1)/100;
			if(replay_version_check(0,30))
				wpnstep = itm.misc1/100;
		
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wIce,itm.level,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
			if(!(itm.weap_data.flags & wdata_set_step))
				((weapon*)Lwpns.spr(Lwpns.Count()-1))->step = wpnstep;
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
			start_cooldown(itemid);
			
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wArrow,itm.level,game->get_hero_dmgmult()*itm.power,dir,itemid,getUID(),false,false,true));
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
			
			bool grumble = (hero_scr->room==rGRUMBLE && (!getmapflag(hero_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (hero_scr->flags9&fBELOWRETURN)));
			bool checkcost = grumble || !(itm.flags & item_flag4);
			bool paycost = grumble || !(itm.flags & (item_flag4|item_flag5));
			
			if(!grumble && (itm.flags & item_flag2))
				return item_error(); //Only usable for grumble rooms
			
			if(checkcost && !checkmagiccost(itemid))
				return item_error();
				
			if(paycost)
				paymagiccost(itemid);
			start_cooldown(itemid);
			sfx(itm.usesound,pan(wx));
			
			if(grumble)
			{
				items.add(new item((zfix)wx,(zfix)wy,(zfix)0,itemid,ipDUMMY+ipFADE,0));
				fadeclk=66;
				dismissmsg();
				clear_bitmap(pricesdisplaybuf);
				set_clip_state(pricesdisplaybuf, 1);
				setmapflag(hero_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
				if(!(itm.flags & item_flag3)) //"Don't remove when feeding" flag
				{
					removeItemsOfFamily(game,itemsbuf,itype_bait);
					verifyBothWeapons();
				}
				sfx(hero_scr->secretsfx);
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
			start_cooldown(itemid);
			
			current_item_power(itype_brang);
			Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wBrang,itm.level,(itm.power*game->get_hero_dmgmult()),dir,itemid,getUID(),false,false,true));
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
			bool sw = itm.type == itype_switchhook;
			
			if(sw && (itm.flags&item_flag8))
				switchhook_cost_item = itemid;
			else paymagiccost(itemid);
			
			start_cooldown(itemid);
			
			bool use_hookshot=true;
			bool hit_hs = false, hit_solid = false, insta_switch = false;
			int32_t max_layer = get_qr(qr_HOOKSHOTALLLAYER) ? 6 : (get_qr(qr_HOOKSHOTLAYERFIX) ? 2 : 0);
			rpos_t rpos = rpos_t::None;
			ffcdata* ffc = nullptr;
			for(int32_t i=0; i<=max_layer && !hit_hs; ++i)
			{
				if(dir==up)
				{
					if(check_hshot(i,x+2,y-7,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==down)
				{
					if(check_hshot(i,x+12,y+23,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==left)
				{
					if(check_hshot(i,x-7,y+12,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==right)
				{
					if(check_hshot(i,x+23,y+12,sw, &rpos, &ffc))
						hit_hs = true;
				}
				//Diagonal Hookshot (6)
				else if(dir==r_down)
				{
					if(check_hshot(i,x+9,y+13,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==l_down)
				{
					if(check_hshot(i,x+6,y+13,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==r_up)
				{
					if(check_hshot(i,x+9,y+13,sw, &rpos, &ffc))
						hit_hs = true;
				}
				else if(dir==l_up)
				{
					if(check_hshot(i,x+6,y+13,sw, &rpos, &ffc))
						hit_hs = true;
				}
			}
			if(dir==up && _walkflag(x+2,y+4,1,get_standing_z_state()) && !ishookshottable(x.getInt(),int32_t(y+4)))
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
				int32_t hookitem = itm.level;
				int32_t hookpower = itm.power;
				byte allow_diagonal = (itm.flags & item_flag2) ? 1 : 0; 
			
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy-4,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)wx,(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						hs_startx=wx-4;
						hs_starty=wy;
					}
					break;
				
					case right:
					{
						hookshot_used=true;
						hs_switcher = sw;
						// TODO(crash): check that .add succeeds.
						Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wHSHandle,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx+4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy+offset,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
						Lwpns.add(new weapon((zfix)(wx-4),(zfix)wy,(zfix)wz,wHookshot,hookitem,
											 hookpower*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
						((weapon*)Lwpns.spr(Lwpns.Count()-1))->family_class = itm.type;
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
				if (rpos != rpos_t::None) hooked_comborpos = rpos;
				if (ffc)
				{
					ffc->switch_hooked = true;
					switching_object = ffc;
				}
				w->misc=2;
				w->step=0;
				doSwitchHook(itm.misc5);
				if(itm.usesound2)
					sfx(itm.usesound2,pan(x));
				else if(QMisc.miscsfx[sfxSWITCHED])
					sfx(QMisc.miscsfx[sfxSWITCHED],pan(x));
				stop_sfx(itm.usesound);
			}
		}
		break;
			
		case itype_divinefire:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(this) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			start_cooldown(itemid);
			
			if (IsSideSwim()) {action=sideswimcasting; FFCore.setHeroAction(sideswimcasting);}
			else {action=casting; FFCore.setHeroAction(casting);}
			magicitem=itemid;
			break;
			
		case itype_divineescape:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(this) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			start_cooldown(itemid);
			
			if (IsSideSwim()) {action=sideswimcasting; FFCore.setHeroAction(sideswimcasting);}
			else {action=casting; FFCore.setHeroAction(casting);}
			magicitem=itemid;
			break;
			
		case itype_divineprotection:
			if(z!=0 || fakez!=0 || (isSideViewHero() && !(on_sideview_solid_oldpos(this) || getOnSideviewLadder() || IsSideSwim())))
				return false;
				
			if(!(checkbunny(itemid) && checkmagiccost(itemid)))
			{
				return item_error();
			}
				
			paymagiccost(itemid);
			start_cooldown(itemid);
			
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
			start_cooldown(itemid);
			
			last_cane_of_byrna_item_id = itemid; 
			for(int32_t i=0; i<itm.misc3; i++)
			{
				//byrna weapons are added here
				//space them apart
				//the iterator isn passed to 'type'. weapons.cpp converts thisd to
				//'quantity_iterator' pn construction; and this is used for orbit initial spacing.
				Lwpns.add(new weapon((zfix)wx,(zfix)wy,(zfix)wz,wCByrna,i,itm.power*game->get_hero_dmgmult(),dir,itemid,getUID(),false,false,true));
			}
			if(!(Lwpns.idCount(wCByrna)))
				stop_sfx(itm.usesound); //If we can't create the beams, kill the sound. 
		}
		break;
		
		case itype_clock:
		{
			ret = false;
			if(!(itm.flags & item_flag1))
				break; //Passive clock, don't use
			if((itm.flags & item_flag2) && watch) //"Can't activate while clock active"
				break;
			if(!(checkbunny(itemid) && checkmagiccost(itemid))) //cost/bunny check
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			start_cooldown(itemid);
			
			setClock(watch=true);
			
			clock_zoras.clear();
				
			clockclk=itm.misc1;
			sfx(itm.usesound);
			break;
		}
		case itype_killem:
		{
			ret = false;
			if(!(itm.flags & item_flag1))
				break; //Passive killemall, don't use
			
			if(!(checkbunny(itemid) && checkmagiccost(itemid))
				|| !can_kill_em_all()) //No enemies onscreen
			{
				return item_error();
			}
			
			paymagiccost(itemid);
			start_cooldown(itemid);
			
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
			
			if(itm.flags & item_flag1) //Cure sword jinx
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
				bool gradual = itm.flags & item_flag2;
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
			start_cooldown(itemid);
			sfx(itm.usesound);
			ret = false;
			break;
		}
		
		case itype_lens:
			// don't start cooldown here, it's activated elsewhere
			ret = false;
			break;
		case itype_shield:
			if (!(itm.flags & item_flag9)) // don't start cooldown here for active shields
				start_cooldown(itemid);
			ret = false;
			break;
		default:
			start_cooldown(itemid);
			ret = false;
			break;
	}
	
	if(itm.flags & item_downgrade)
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
bool HeroClass::on_cooldown(int32_t itemid)
{
	if (itemid < 0 || itemid >= MAXITEMS) return false;
	return item_cooldown[itemid];
}
void HeroClass::start_cooldown(int32_t itemid)
{
	if (itemid < 0 || itemid >= MAXITEMS) return;
	if (item_cooldown[itemid] < 0) return; // set to perma-cooldown (via script, presumably)
	auto cd_data = calc_item_cooldown(itemid);
	if(cd_data.max_cooldown <= 0) return;
	if(cd_data.max_cooldown > item_cooldown[itemid])
	{
		item_cooldown[itemid] = cd_data.max_cooldown;
		paymagiccost(cd_data.cooldown_ring_id);
	}
	else if(cd_data.base_cooldown > item_cooldown[itemid]) // cd ring still *did something*, so still pay
		paymagiccost(cd_data.cooldown_ring_id);
}

bool HeroClass::doattack()
{
	if(lift_wpn && (liftflags & LIFTFL_DIS_ITEMS))
		return false;
	//int32_t s = BSZ ? 0 : 11;
	int32_t s = (zinit.heroAnimationStyle==las_bszelda) ? 0 : 11;
	
	int32_t bugnetid = (directWpn>-1 && itemsbuf[directWpn].type==itype_bugnet) ? directWpn : current_item_id(itype_bugnet);
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
	
	int32_t candleid = (directWpn>-1 && itemsbuf[directWpn].type==itype_candle) ? directWpn : current_item_id(itype_candle);
	int32_t byrnaid = (directWpn>-1 && itemsbuf[directWpn].type==itype_cbyrna) ? directWpn : current_item_id(itype_cbyrna);
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
		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
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
		normalcharge = (itemsbuf[itemid].misc1>0)?itemsbuf[itemid].misc1:64;
		magiccharge = (itemsbuf[itemid].misc2>normalcharge)?itemsbuf[itemid].misc2:192;
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
			if(!(itemsbuf[scrollid].flags&item_flag1))
				paymagiccost(scrollid);
			sfx(itemsbuf[scrollid].usesound2,pan(x));
		}
		else if(charging==magiccharge)
		{
			if(scroll2id>-1 && checkbunny(scroll2id) && checkmagiccost(scroll2id))
			{
				if(!(itemsbuf[scroll2id].flags&item_flag1))
					paymagiccost(scroll2id);
				charging++; // charging>magiccharge signifies a successful supercharge.
				sfx(itemsbuf[scroll2id].usesound2,pan(x));
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
				bool paid = !(spinscroll.flags&item_flag1);
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
					sfx(spinscroll.usesound,pan(x));
					if(spinscroll.flags&item_flag1)
						paymagiccost(id);
				}
			}
			else if(attack==wHammer && sideviewhammerpound())
			{
				bool super = charging>magiccharge && scroll2id > -1;
				int id = super ? scroll2id : scrollid;
				itemdata const& quakescroll = itemsbuf[id];
				bool paid = !(quakescroll.flags&item_flag1);
				if(!paid && checkbunny(id) && checkmagiccost(id))
				{
					paid = true;
					paymagiccost(id);
				}
				if(paid)
				{
					currentscroll = id;
					spins = super ? 2 : 1;
					sfx(quakescroll.usesound,pan(x));
					quakeclk=quakescroll.misc1;
					
					// general area stun
					for(int32_t i=0; i<GuyCount(); i++)
					{
						if(!isflier(GuyID(i)))
						{
							StunGuy(i,quakescroll.misc2-distance(x,y,GuyX(i),GuyY(i)));
						}
					}
					
					int hmrid = (directWpn>-1 && itemsbuf[directWpn].type==itype_hammer) ? directWpn : current_item_id(itype_hammer);
					int hmrlvl = hmrid < 0 ? 1 : itemsbuf[hmrid].level;
					if(hmrlvl < 1) hmrlvl = 1;
					int rad = quakescroll.misc2;
					for_every_combo([&](const auto& handle) {
						auto [cx, cy] = handle.xy();
						if (distance(x, y, cx, cy) > rad)
							return;

						vector<int> trigflags = {TRIGFLAG_QUAKESTUN};
						if(super) trigflags.push_back(TRIGFLAG_SQUAKESTUN);
						trig_each_combo_trigger(handle, [&](combo_trigger const& trig){
							if(!trig.trigger_flags.any(trigflags)) return false;
							return (trig.trigger_flags.get(TRIGFLAG_INVERTMINMAX)
								? hmrlvl <= trig.triggerlevel : hmrlvl >= trig.triggerlevel);
						});
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
	
		int32_t wpnid = (directWpn>-1 && itemsbuf[directWpn].type==itype_sword) ? directWpn : current_item_id(itype_sword);
		int64_t templife = wpnid>=0? itemsbuf[wpnid].misc1 : 0;
		
		if(wpnid>=0 && itemsbuf[wpnid].flags & item_flag1)
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
						  && ((itemsbuf[wpnid].flags & item_flag1)
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
	if(lift_wpn && (liftflags & LIFTFL_DIS_ITEMS))
		return false;
	int32_t currentSwordOrWand = (itemsbuf[dowpn].type == itype_wand || itemsbuf[dowpn].type == itype_sword)?dowpn:-1;
    if(action==hopping || action==swimming || action==freeze || action==sideswimfreeze
		|| lstunclock > 0 || is_conveyor_stunned || spins>0 || usingActiveShield()
		|| ((action==attacking||action==sideswimattacking)
			&& ((attack!=wSword && attack!=wWand) || !(itemsbuf[currentSwordOrWand].flags & item_flag5))
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
	bool enabled = l_id >= 0 && (itemsbuf[l_id].flags & item_flag6);
	auto& combo_cache = combo_caches::lens;
	for_every_combo([&](const auto& handle) {
		auto cid = handle.data();
		auto& mini_cmb = combo_cache.minis[cid];
		if (!(enabled ? mini_cmb.on : mini_cmb.off))
			return;
		trig_each_combo_trigger(handle, [&](combo_trigger const& trig){
			return trig.trigger_flags.get(enabled ? TRIGFLAG_LENSON : TRIGFLAG_LENSOFF);
		});
	});
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
	if (lensid < 0 && Hero.on_cooldown(itemid))
		itemid = -1;
	if(itemid >= 0)
	{
		if(isWpnPressed(itype_lens) && checkitem_jinx(itemid) && !lensclk && checkbunny(itemid) && checkmagiccost(itemid))
		{
			if(lensid<0)
			{
				lensid = itemid;
				if(itemsbuf[itemid].type == itype_lens)
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
				
				if(lensid > -1)
				{
					Hero.start_cooldown(lensid);
					lensid = -1;
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

void HeroClass::tick_diving()
{
	if (diveclk && isSideViewHero() && get_qr(qr_SIDESWIM))
		set_dive(0);
	int32_t flippers_id = current_item_id(itype_flippers);
	itemdata const& itm = itemsbuf[flippers_id];
	byte intbtn = byte(itm.misc3&0xFF);
	bool dive_pressed = getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK | INPUT_PEEK);
	bool eatdive = false;
	if (diveclk > 0)
	{
		set_dive(diveclk - 1);
		bool cool_down = (diveclk == 0);
		if (!cool_down && (flippers_id > -1 && (itm.flags & item_flag2) && dive_pressed)) // cancel dive
		{
			cool_down = true;
			eatdive = true;
		}
		if (cool_down)
			set_dive(flippers_id < 0 ? -30 : -itemsbuf[flippers_id].misc2); // start cooldown
	}
	else if (diveclk < 0)
	{
		set_dive(diveclk + 1);
	}
	else if ((action == swimming || action == hopping) && dive_pressed)
	{
		bool global_diving = (flippers_id > -1 && (itm.flags & item_flag1));
		bool screen_diving = (hero_scr->flags5 & fTOGGLEDIVING) != 0;
		
		if (global_diving == screen_diving)
		{
			diveclk = (flippers_id < 0 ? 50 : itm.misc1);
			eatdive = true;
		}
	}
	if(eatdive)
		getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK);
}

void HeroClass::do_hopping()
{
    do_lens();
    
    if(hopclk==0xFF) //|| (diagonalMovement && hopclk >= 0xFF) ))                                         // swimming
			//Possible fix for exiting water in diagonal movement. -Z
    {
		tick_diving();
        
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                        set_dive(0);
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
                set_dive(0);
                
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

	if(walk_through_walls)
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
			if (dir != down && getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && (isRaftFlag(nextflag(x, y, up, false)) || isRaftFlag(nextflag(x, y, up, true))))
			{
				dir = up;
				goto skip;
			}

			if (dir != up && getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && (isRaftFlag(nextflag(x, y, down, false)) || isRaftFlag(nextflag(x, y, down, true))))
			{
				dir = down;
				goto skip;
			}

			if (dir != right && getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && (isRaftFlag(nextflag(x, y, left, false)) || isRaftFlag(nextflag(x, y, left, true))))
			{
				dir = left;
				goto skip;
			}

			if (dir != left && getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && (isRaftFlag(nextflag(x, y, right, false)) || isRaftFlag(nextflag(x, y, right, true))))
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

bool HeroClass::tick_hover()
{
	if(hoverclk > 0)
	{
		if(hoverflags&HOV_INF) return false;
		if(is_autowalking()) return false;
		if(!--hoverclk)
		{
			hoverflags |= HOV_OUT | HOV_PITFALL_OUT;
			return true;
		}
	}
	return false;
}

bool HeroClass::try_hover()
{
	if(hoverclk > 0) return false;
	if(!(is_autowalking() && check_pitslide() != -1))
	{
		if(!can_use_item(itype_hoverboots,i_hoverboots)) return false;
		if(ladderx || laddery) return false;
		if(hoverflags & HOV_OUT) return false;
	}
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
		
		sfx(itemsbuf[itemid].usesound,pan(x));
	}
	if(itemsbuf[itemid].wpn)
		decorations.add(new dHover(x, y, dHOVER, 0));
	return true;
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
		bool can_diag = (diagonalMovement || NO_GRIDLOCK);
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
						if (!ispitul_75 && (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
						return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
					}
				}
				else if(ispitul && ispitur && ispitbr) //UR_3
				{
					if(ispitur_50)
					{
						if (!ispitur_75 && (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
						return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
					}
				}
				else if(ispitul && ispitbl && ispitbr) //BL_3
				{
					if(ispitbl_50)
					{
						if (!ispitbl_75 && (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
						return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
					}
				}
				else if(ispitbl && ispitur && ispitbr) //BR_3
				{
					if(ispitbr_50)
					{
						if (!ispitbr_75 && (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
						return (can_diag ? r_down : right) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
					}
				}
				break;
			}
			case 2:
			{
				if(ispitul && ispitur) //Up
				{
					if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
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
							if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitul_75) return -1;
							return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitur_50)
						{
							if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitur_75) return -1;
							return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
						}
					}
				}
				else if(ispitbl && ispitbr) //Down
				{
					if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION))
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
							if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitbl_75) return -1;
							return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
						}
						else if(ispitbr_50)
						{
							if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitbr_75) return -1;
							return (can_diag ? r_down : right) | (ispitbr_75 ? flag_pit_irresistable : 0) | (ispitbr << 8);
						}
					}
				}
				else if(ispitbl && ispitul) //Left
				{
					if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
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
							if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitul_75) return -1;
							return (can_diag ? l_up : up) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
						}
						else if(ispitbl_50)
						{
							if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitbl_75) return -1;
							return (can_diag ? l_down : down) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
						}
					}
				}
				else if(ispitbr && ispitur) //Right
				{
					if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))
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
							if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitur_75) return -1;
							return (can_diag ? r_up : up) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
						}
						else if(ispitbr_50)
						{
							if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !ispitbr_75) return -1;
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
					if (!ispitul_75 && (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
					return (can_diag ? l_up : left) | (ispitul_75 ? flag_pit_irresistable : 0) | (ispitul << 8);
				}
				if(ispitur && ispitur_50) //UR_1
				{
					if (!ispitur_75 && (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
					return (can_diag ? r_up : right) | (ispitur_75 ? flag_pit_irresistable : 0) | (ispitur << 8);
				}
				if(ispitbl && ispitbl_50) //BL_1
				{
					if (!ispitbl_75 && (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
					return (can_diag ? l_down : left) | (ispitbl_75 ? flag_pit_irresistable : 0) | (ispitbl << 8);
				}
				if(ispitbr && ispitbr_50) //BR_1
				{
					if (!ispitbr_75 && (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))) return -1;
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
		if(fallclk == PITFALL_FALL_FRAMES && fallCombo) sfx(combobuf[fallCombo].attribytes[0], pan(x));
		//Handle falling
		if(!--fallclk)
		{
			optional<combined_handle_t> comb_handle;
			if((comb_handle = get_pitfall_handle(x+8, y+(bigHitbox?8:12))) && comb_handle->data() == fallCombo) /*nil*/;
			else if((comb_handle = get_pitfall_handle(x+8, y+(bigHitbox?0:8))) && comb_handle->data() == fallCombo) /*nil*/;
			else if((comb_handle = get_pitfall_handle(x+15, y+(bigHitbox?0:8))) && comb_handle->data() == fallCombo) /*nil*/;
			else if((comb_handle = get_pitfall_handle(x, y+15)) && comb_handle->data() == fallCombo) /*nil*/;
			else if((comb_handle = get_pitfall_handle(x+15, y+15)) && comb_handle->data() == fallCombo) /*nil*/;
			else comb_handle = nullopt; // nothing found, no `CType Causes->`
			
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
				if(dmg > 0)
				{
					hclk=48; //IFrames only if damaged, not if healed
					check_on_hit();
				}
				game->set_life(vbound(int32_t(dmg_perc ? game->get_life() - ((vbound(dmg,-100,100)/100.0)*game->get_maxlife()) : (game->get_life()-int64_t(dmg))),0,game->get_maxlife()));
			}
			
			if(comb_handle)
				do_trigger_ctype_causes(*comb_handle);
			
			if(warp) //Warp
			{
				sdir = dir;
				if(cmb->usrflags&cflag2) //Direct Warp
				{
					setpit();
				}
				dowarp(hero_scr, 0, vbound(cmb->attribytes[1],0,3), 0);
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

void HeroClass::handle_slide(newcombo const& icecmb, zfix& dx, zfix& dy)
{
	bool is_inair = z || fakez;
	zfix odx = dx, ody = dy;
	if(sliding == 2 && !is_inair) //landed from air-sliding
	{
		if((ice_vx+odx).sign() != ice_vx.sign())
			ice_vx = 0;
		if((ice_vy+ody).sign() != ice_vy.sign())
			ice_vy = 0;
		sliding = 1;
	}
	if(!sliding) //just hit the ice
	{
		if(is_inair)
			return;
		sliding = 1;
		zfix start_perc = icecmb.attribytes[0] / 100_zf;
		ice_vx = dx * start_perc;
		ice_vy = dy * start_perc;
		ice_entry_count = ice_entry_mcount = icecmb.attribytes[1];
	}
	else //not the first frame sliding
	{
		if(is_inair)
			sliding = 2;
		zfix accel = zslongToFix(zc_max(1,icecmb.attributes[0]));
		zfix decel = zslongToFix(zc_max(1,icecmb.attributes[1]));
		
		if(ice_entry_count)
		{
			if(--ice_entry_count)
			{
				zfix perc = zfix(ice_entry_count)/ice_entry_mcount;
				perc *= perc; //square the portion, for a better transition
				zfix normal_rate = zfix(steprate)/100/2;
				decel = (perc*normal_rate)+((1-perc)*decel);
			}
		}
		//!TODO Traction Boots can be added here, with a multiplier on accel/decel
		//Accelerate in the pushed direction
		if(is_inair)
		{
			static const int air_accel = 100;
			accel = abs(odx)/air_accel;
			if(accel)
			{
				if(odx < 0)
				{
					if(ice_vx - accel < odx)
					{
						if(ice_vx > odx)
							ice_vx = odx;
						else odx = 0; //allow decel
					}
					else ice_vx -= accel;
				}
				else //if(odx > 0)
				{
					if(ice_vx + accel > odx)
					{
						if(ice_vx < odx)
							ice_vx = odx;
						else odx = 0; //allow decel
					}
					else ice_vx += accel;
				}
			}
			accel = abs(ody)/air_accel;
			if(accel)
			{
				if(ody < 0)
				{
					if(ice_vy - accel < ody)
					{
						if(ice_vy > ody)
							ice_vy = ody;
						else ody = 0; //allow decel
					}
					else ice_vy -= accel;
				}
				else //if(ody > 0)
				{
					if(ice_vy + accel > ody)
					{
						if(ice_vy < ody)
							ice_vy = ody;
						else ody = 0; //allow decel
					}
					else ice_vy += accel;
				}
			}
		}
		else if(accel)
		{
			if(odx > 0)
				ice_vx += accel;
			else if(odx < 0)
				ice_vx -= accel;
			if(ody > 0)
				ice_vy += accel;
			else if(ody < 0)
				ice_vy -= accel;
		}
		//Decelerate in non-pushed direction
		if(is_inair)
			decel = zinit.air_drag;
		if(decel)
		{
			if(ice_vx)
			{
				if(!odx && abs(ice_vx) < decel)
					ice_vx = 0;
				else if(ice_vx > 0 && odx <= 0)
					ice_vx -= decel;
				else if(ice_vx < 0 && odx >= 0)
					ice_vx += decel;
			}
			if(ice_vy)
			{
				if(!ody && abs(ice_vy) < decel)
					ice_vy = 0;
				else if(ice_vy > 0 && ody <= 0)
					ice_vy -= decel;
				else if(ice_vy < 0 && ody >= 0)
					ice_vy += decel;
			}
		}
	}
	zfix cap = zslongToFix(zc_max(1,abs(icecmb.attributes[2])));
	dx = ice_vx = vbound(ice_vx, -cap, cap);
	dy = ice_vy = vbound(ice_vy, -cap, cap);
	if(!dx && !dy)
		ice_entry_count = 0;
}
void HeroClass::mod_steps(std::vector<zfix*>& v)
{
	bool can_combo = ((z==0 && fakez==0) || hero_scr->flags2&fAIRCOMBOS);
	bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1,-1) && can_combo) ||
					 (isSideViewHero() && (on_sideview_solid_oldpos(this)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1,-1));
			 //!DIMITODO: add QR for slow combos under hero
	if(slowcombo) for (int32_t i = 1; i <= 2; ++i)
	{
		if (get_scr_layer_valid(current_screen, i))
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i-1, x+7, y+8)].type == cBRIDGE && !_walkflag_layer(x+7, y+8, i))
				{
					slowcombo = false;
					break;
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(i-1, x+7, y+8)].type == cBRIDGE && _effectflag_layer(x+7, y+8, i))
				{
					slowcombo = false;
					break;
				}
			}
		}
	}
	bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & item_flag10);
	bool is_swimming = (action == swimming);
	int32_t shieldid = getCurrentActiveShield();
	if(shieldid > -1)
	{
		itemdata const& shield = itemsbuf[shieldid];
		if(shield.flags & item_flag10) //Change Speed flag
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
		rpos_t slow_rpos = COMBOPOS_REGION_B(x+7, y+8);
		for (int q = 6; q >= 0; --q)
		{
			if (slow_rpos == rpos_t::None) break;
			auto& cmb = get_rpos_handle(slow_rpos, q).combo();

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
	
	tick_diving();
	
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
	int32_t currentSwordOrWand = (itemsbuf[dowpn].type == itype_wand || itemsbuf[dowpn].type == itype_sword)?dowpn:-1;
	if((!attackclk && action!=attacking && action != sideswimattacking) || ((attack==wSword || attack==wWand) && (itemsbuf[currentSwordOrWand].flags & item_flag5)))
	{
		if (getInput(btnB, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Bwpn);
			dowpn = NEG_OR_MASK(Bwpn,0xFFF);
			directWpn = directItemB;
		}
		else if (getInput(btnA, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Awpn);
			dowpn = NEG_OR_MASK(Awpn,0xFFF);
			directWpn = directItemA;
		}
		else if (get_qr(qr_SET_XBUTTON_ITEMS) && getInput(btnEx1, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Xwpn);
			dowpn = NEG_OR_MASK(Xwpn,0xFFF);
			directWpn = directItemX;
		}
		else if (get_qr(qr_SET_YBUTTON_ITEMS) && getInput(btnEx2, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Ywpn);
			dowpn = NEG_OR_MASK(Ywpn,0xFFF);
			directWpn = directItemY;
		}
		
		auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
		if (on_cooldown(itmid))
		{
			directWpn = olddirectwpn;
			btnwpn = -1;
			dowpn = -1;
			did_scriptb = false;
		}
		else
		{
			if(directWpn >= MAXITEMS) directWpn = -1;
			
			// The Quick Sword only allows repeated sword or wand swings.
			if((action==attacking||action==sideswimattacking) && ((attack==wSword && btnwpn!=itype_sword) || (attack==wWand && btnwpn!=itype_wand)))
				btnwpn=-1;
		}
	}
	
	auto swordid = (directWpn>-1 ? directWpn : current_item_id(itype_sword));
	if(can_attack() && (swordid > -1 && itemsbuf[swordid].type==itype_sword) && checkitem_jinx(swordid) && btnwpn==itype_sword && charging==0)
	{
		attackid=directWpn>-1 ? directWpn : current_item_id(itype_sword);
		if(checkbunny(attackid) && (checkmagiccost(attackid) || !(itemsbuf[attackid].flags & item_flag6)))
		{
			if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_SWORD_COST))
			{
				paymagiccost(attackid,true);
				misc_internal_hero_flags |= LF_PAID_SWORD_COST;
			}
			SetAttack();
			attack=wSword;
			
			attackclk=0;
			sfx(itemsbuf[directWpn>-1 ? directWpn : current_item_id(itype_sword)].usesound, pan(x));
			
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
		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && canSideviewLadder())
		{
			setOnSideviewLadder(true);
		}
		else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && canSideviewLadder(true))
		{
			y+=1;
			old_y += 1;
			setOnSideviewLadder(true);
		}
	}
	
	int32_t wx=x;
	int32_t wy=y;
	if((action==none || action==walking) && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
	{
		if((xoff==0)||diagonalMovement)
		{
			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;
			if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;
		}
		
		if((yoff==0)||diagonalMovement)
		{
			if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;
			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
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
	bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
	auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
	if(liftonly)
	{
		if(replay_version_check(38) && btnwpn > -1)
		{
			no_jinx = checkitem_jinx(itmid);
			if(no_jinx)
				startwpn(itmid);
			directWpn = olddirectwpn;
		}
	}
	else if(can_attack() && btnwpn>itype_sword && charging==0 && btnwpn!=itype_rupee) // This depends on item 0 being a rupee...
	{
		bool paidmagic = false;
		if(btnwpn==itype_wand && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_wand : false) : current_item(itype_wand)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_wand);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & item_flag6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_WAND_COST)){
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
		else if((btnwpn==itype_hammer)&&!((action==attacking||action==sideswimattacking) && attack==wHammer)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_hammer : false) : current_item(itype_hammer)))
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
		else if((btnwpn==itype_candle)&&!((action==attacking||action==sideswimattacking) && attack==wFire)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_candle : false) : current_item(itype_candle)))
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
		else if((btnwpn==itype_cbyrna)&&!((action==attacking||action==sideswimattacking) && attack==wCByrna)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_cbyrna : false) : current_item(itype_cbyrna)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_cbyrna);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & item_flag6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_CBYRNA_COST)){
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
		else if((btnwpn==itype_bugnet)&&!((action==attacking||action==sideswimattacking) && attack==wBugNet)
				&& (directWpn>-1 ? (!item_disabled(directWpn) && itemsbuf[directWpn].type==itype_bugnet) : current_item(itype_bugnet)))
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
				if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;
				if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;
			}
			
			if((yoff==0)||diagonalMovement)
			{
				if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;
				if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
			}
		}
		
		bool attacked = doattack();
		
		// This section below interferes with script-setting Hero->Dir, so it comes after doattack
		if(!inlikelike && attackclk>4 && (attackclk&3)==0 && charging==0 && spins==0 && action!=sideswimattacking)
		{
			if((xoff==0)||diagonalMovement)
			{
				if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;

				if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;
			}
			
			if((yoff==0)||diagonalMovement)
			{
				if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;

				if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
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
					sfx(itemsbuf[id].usesound,pan(x));
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
		if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !autostep)
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
			if (!getInput(btnUp, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case down:
			if (!getInput(btnDown, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case left:
			if (!getInput(btnLeft, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case right:
			if (!getInput(btnRight, INPUT_HERO_ACTION))
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
			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == up))
			{
				if(isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
					{
						dir=up;
					}
					
					holddir=up;

					if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left)
					{
						shiftdir=right;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
					}
					else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right)
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
					if(isSideViewHero() && !walk_through_walls && !((can_deploy_ladder() && get_qr(qr_OLD_LADDER_ITEM_SIDEVIEW)) || (ladderx && laddery && ladderdir==up)) && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						if ((get_qr(qr_OLD_LADDER_ITEM_SIDEVIEW) && can_deploy_ladder()) || (ladderx && laddery && ladderdir==up)) justmoved = 2; //Prevent the slope-snap code from running
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
					
					if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM))
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
							if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
									!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
									_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else
							{
								if(_walkflag(x,   y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
										!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
										!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
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

			if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == down))
			{
				if(isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
					{
						dir=down;
					}
					
					holddir=down;

					if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left)
					{
						shiftdir=right;
						if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
						if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
					}
					else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right)
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
					if(isSideViewHero() && !walk_through_walls && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
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
					
					if(isdungeon() && (y<=26 || y>=world_h - 42) && !get_qr(qr_FREEFORM))
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
							if(!_walkflag(x,   y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+8, y+15+1,1,get_standing_z_state())&&
									_walkflag(x+15,y+15+1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else if(_walkflag(x,   y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+7, y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+15,y+15+1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
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

			if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == left))
			{
				if(isdungeon() && (y<=26 || y>=world_h - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
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

					if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down)
					{
						shiftdir=up;
					}
					else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up)
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
					
					if((isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
							
							if(!_walkflag(x-1,y+v1,1,get_standing_z_state())&&
									!_walkflag(x-1,y+v2,1,get_standing_z_state())&&
									_walkflag(x-1,y+15,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x-1,y+v1,  1,get_standing_z_state())&&
									!_walkflag(x-1,y+v2-1,1,get_standing_z_state())&&
									!_walkflag(x-1,y+15,  1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == right))
			{
				if(isdungeon() && (y<=26 || y>=world_h - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
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

					if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down)
					{
						shiftdir=up;
					}
					else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up)
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
					
					if((isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
								   
							if(!_walkflag(x+16,y+v1,1,get_standing_z_state())&&
								   !_walkflag(x+16,y+v2,1,get_standing_z_state())&&
								   _walkflag(x+16,y+15,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x+16,y+v1,1,get_standing_z_state())&&
									   !_walkflag(x+16,y+v2-1,1,get_standing_z_state())&&
									   !_walkflag(x+16,y+15,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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
			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == up))
			{
				if(isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0)
					{
						dir=up;
					}
					
					holddir=up;

					if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left)
					{
						shiftdir=right;
					}
					else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right)
					{
						shiftdir=left;
					}
					else
					{
						shiftdir=-1;
					}
					
					//walkable if Ladder can be placed or is already placed vertically
					if(isSideViewHero() && !walk_through_walls && !(((replay_version_check(0, 23) || get_qr(qr_OLD_LADDER_ITEM_SIDEVIEW)) && can_deploy_ladder()) || (ladderx && laddery && ladderdir==up)) && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
					{
						walkable=false;
					}
					else
					{
						if ((can_deploy_ladder() && get_qr(qr_OLD_LADDER_ITEM_SIDEVIEW)) || (ladderx && laddery && ladderdir==up)) justmoved = 2;
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
					
					if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM))
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
							if(!_walkflag(x,   y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
									!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
									_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else
							{
								if(_walkflag(x,   y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
										!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
										!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
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

			if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == down))
			{
				if(isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0)
					{
						dir=down;
					}
					
					holddir=down;

					if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left)
					{
						shiftdir=right;
					}
					else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right)
					{
						shiftdir=left;
					}
					else
					{
						shiftdir=-1;
					}
					
					//bool walkable;
					if(isSideViewHero() && !walk_through_walls && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
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
					
					if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM))
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
							if(!_walkflag(x,   y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+8, y+15+1,1,get_standing_z_state())&&
									_walkflag(x+15,y+15+1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
									sprite::move((zfix)-1,(zfix)0);
							}
							else if(_walkflag(x,   y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+7, y+15+1,1,get_standing_z_state())&&
									!_walkflag(x+15,y+15+1,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
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

			if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == left))
			{
				if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0)
					{
						dir=left;
					}
					
					holddir=left;

					if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down)
					{
						shiftdir=up;
					}
					else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up)
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
					
					if((isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
							
							if(!_walkflag(x-1,y+v1,1,get_standing_z_state())&&
									!_walkflag(x-1,y+v2,1,get_standing_z_state())&&
									_walkflag(x-1,y+15,1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else if(_walkflag(x-1,y+v1,  1,get_standing_z_state())&&
									!_walkflag(x-1,y+v2-1,1,get_standing_z_state())&&
									!_walkflag(x-1,y+15,  1,get_standing_z_state()))
							{
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == right))
			{
				if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
				{
				}
				else
				{
					if(charging==0 && spins==0)
					{
						dir=right;
					}
					
					holddir=right;

					if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down)
					{
						shiftdir=up;
					}
					else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up)
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
					
					if((isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM)) || (isSideViewHero() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking))
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
								if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
									sprite::move((zfix)0,(zfix)-1);
							}
							else
							{
								info = walkflag(x+16,y+v1,  1,right)&&
									   !walkflag(x+16,y+v2-1,1,right)&&
									   !walkflag(x+16,y+15,  1,right);
									   
								if(info.isUnwalkable())
								{
									if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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
		
		if(can_use_item(itype_flippers,i_flippers)&&current_item(itype_flippers) >= combobuf[wtrc].attribytes[0]&&(!(combobuf[wtrc].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))&&!(ladderx+laddery)&&z==0&&fakez==0)
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
	
	if(isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
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
		if (isdungeon() && getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && (temp_x == 32 && temp_y == 80))
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

		if (isdungeon() && getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && temp_x == 208 && temp_y == 80)
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

		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION))
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
						
						if(_walkflag(temp_x+15, temp_y+(bigHitbox?0:8)-temp_step, 1,get_standing_z_state()) &&
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

				if (!getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
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

		if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
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
						
						if(_walkflag(temp_x+15, temp_y+15+temp_step, 1,get_standing_z_state()) &&
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

				if (!getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						if(!_walkflag(x,   y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+8, y+15+1,1,get_standing_z_state())&&
								_walkflag(x+15,y+15+1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,   y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+7, y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+15,y+15+1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
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
		if(isdungeon() && (temp_y<=26 || temp_y>=134) && !get_qr(qr_FREEFORM) && !walk_through_walls)
		{
			return;
		}

		if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))
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

				if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
						
						if(!_walkflag(x-1,y+v1,1,get_standing_z_state())&&
								!_walkflag(x-1,y+v2,1,get_standing_z_state())&&
								_walkflag(x-1,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x-1,y+v1,1,get_standing_z_state())&&
								!_walkflag(x-1,y+v2-1,1,get_standing_z_state())&&
								!_walkflag(x-1,y+15,  1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

		if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
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

				if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						x = x.getInt();
						y = y.getInt();
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
							   
						if(!_walkflag(x+16,y+v1,1,get_standing_z_state())&&
							   !_walkflag(x+16,y+v2,1,get_standing_z_state())&&
							   _walkflag(x+16,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x+16,y+v1,1,get_standing_z_state())&&
								   !_walkflag(x+16,y+v2-1,1,get_standing_z_state())&&
								   !_walkflag(x+16,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

		if (isdungeon() && getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !info.isUnwalkable() && (x == 32 && y == 80))
		{
			//ONLY process the side-effects of the above walkflag if Hero will actually move
			//sigh sigh sigh... walkflag is a horrible mess :-/ -DD
			execute(info);
			moveOld2(left);
			return;
		}
		
		info = walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+(bigHitbox?0:8),1,right) ||
			   walkflag(x+15+int32_t(lsteps[x.getInt()&7]),y+8,1,right);

		if (isdungeon() && getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !info.isUnwalkable() && x == 208 && y == 80)
		{
			execute(info);
			moveOld2(right);
			return;
		}
		
		ladderx = oldladderx;
		laddery = oldladdery;

		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION))
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
					
					if(_walkflag(x+15, y+(bigHitbox?0:8)-int32_t(lsteps[y.getInt()&7]), 1,get_standing_z_state()) &&
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

				if (!getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						if(!_walkflag(x,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+8, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+(bigHitbox?0:8)-1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+7, y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
								!_walkflag(x+15,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+(bigHitbox?0:8)-1))
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

		if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
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
					
					if(_walkflag(x+15, y+15+int32_t(lsteps[y.getInt()&7]), 1,get_standing_z_state()) &&
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

				if (!getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						if(!_walkflag(x,   y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+8, y+15+1,1,get_standing_z_state())&&
								_walkflag(x+15,y+15+1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),y+15+1))
								sprite::move((zfix)-1,(zfix)0);
						}
						else if(_walkflag(x,   y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+7, y+15+1,1,get_standing_z_state())&&
								!_walkflag(x+15,y+15+1,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),y+15+1))
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

		if(isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM) && !walk_through_walls)
		{
			return;
		}

		if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))
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

				if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
						
						if(!_walkflag(x-1,y+v1,1,get_standing_z_state())&&
								!_walkflag(x-1,y+v2,1,get_standing_z_state())&&
								_walkflag(x-1,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x-1,y+v1,  1,get_standing_z_state())&&
								!_walkflag(x-1,y+v2-1,1,get_standing_z_state())&&
								!_walkflag(x-1,y+15,  1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x-1,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

		if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
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

				if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
				{
					if(NO_GRIDLOCK)
					{
						int32_t v1=bigHitbox?0:8;
						int32_t v2=bigHitbox?8:12;
							   
						if(!_walkflag(x+16,y+v1,1,get_standing_z_state())&&
							   !_walkflag(x+16,y+v2,1,get_standing_z_state())&&
							   _walkflag(x+16,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11)))
								sprite::move((zfix)0,(zfix)-1);
						}
						else if(_walkflag(x+16,y+v1,1,get_standing_z_state())&&
								   !_walkflag(x+16,y+v2-1,1,get_standing_z_state())&&
								   !_walkflag(x+16,y+15,1,get_standing_z_state()))
						{
							if(hclk || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS)) || !checkdamagecombos(x+16,y+v1+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
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

bool HeroClass::scr_walkflag(zfix_round zdx,zfix_round zdy,int d2,bool kb, int* canladder)
{
	if(walk_through_walls) return false;
	int dx = zdx.getRound(), dy = zdy.getRound();
	
	bool solid = false;
	
	if(!is_autowalking())
	{
		if(blockpath && dy<80) //Blocked top parts of rooms
			return true;
		
		if(mblock2.active() && mblock2.hit(dx,dy,0,1,1,1))
			return true;
		//collide_object handled in scr_canmove
		
		if(isdungeon() && cur_screen<128 && dy<40
			&& ((x<=112||x>=128) || _walkflag(120,24,2,get_standing_z_state()))
			&& !get_qr(qr_FREEFORM))
			return true; //Old NES dungeon stuff
		
		solid = _walkflag(zdx,zdy,1,get_standing_z_state());
		
		if(isdungeon() && cur_screen<128 && !get_qr(qr_FREEFORM))
		{
			if(dx>=112&&dx<120&&dy<40&&dy>=32)
				solid=true;
			
			if(dx>=136&&dx<144&&dy<40&&dy>=32)
				solid=true;
		}
	}
	
	if(action==swimming || IsSideSwim())
	{
		if(!solid)
		{
			int ls = 22;
			if((get_qr(qr_DROWN) && isSwimming()) || (!diagonalMovement) || get_qr(qr_NO_HOPPING))
				ls = 1;
			if(landswim < ls)
			{
				if(dx<0||dy<0);
				else if(dx>=256);
				else if(dy>=176);
				else if(get_qr(qr_DROWN) && !ilswim);
				else if(iswaterex_z3(MAPCOMBO(dx,dy), -1, dx,dy)) //!DIMI: weird duplicate function here before. Was water bugged this whole time, or was it just an unneccessary duplicate?
					solid = false;
				else
					solid = true;
			}
		}
		else
		{
			int32_t wtrx  = iswaterex_z3(MAPCOMBO(dx,dy), -1, dx,dy);
			
			if(wtrx)
				solid = false;
		}
	}
	else if(ladderx+laddery)                                  // ladder is being used
	{
		if (canladder)
		{
			*canladder = 0;
		}
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
		bool unwalkablex  = _walkflag(zdx,zdy,1,get_standing_z_state()); //will be used later for the ladder -DD
		
		if(get_qr(qr_DROWN))
		{
			// Drowning changes the following attributes:
			// * Dangerous water is also walkable, so ignore the previous
			// definitions of unwalkablex.
			// * Instead, prevent the ladder from being used in the
			// one frame where Hero has landed on water before drowning.
			unwalkablex = !iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11);
		}
		
		// check if he can swim
		if(current_item(itype_flippers) && z==0 && fakez==0)
		{
			int32_t wtrx  = iswaterex_z3(MAPCOMBO(dx,dy), -1, dx,dy);
			if (current_item(itype_flippers) >= combobuf[wtrx].attribytes[0] && (!(combobuf[wtrx].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))) //Don't swim if the water's required level is too high! -Dimi
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
			bool wtrx = (iswaterex_z3(MAPCOMBO(dx,dy), -1, dx,dy) != 0);
			int32_t ldrid = current_item_id(itype_ladder);
			bool ladderpits = ldrid > -1 && (itemsbuf[ldrid].flags&item_flag1);
			
			if(wtrx)
			{
				if(isSideViewHero())
				{
					wtrx  = !_walkflag(zdx, zdy+8, 1,get_standing_z_state()) && !_walkflag(dx, dy, 1,get_standing_z_state()) && dir!=down;
				}
				// * walk on half-water using the ladder instead of using flippers.
				// * otherwise, walk on ladder(+hookshot) combos.
				else if((isstepable(MAPCOMBO(dx, dy)) || wtrx==true))
				{
					if(!get_qr(qr_OLD_210_WATER) && current_item(itype_flippers))
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
			
			for (int32_t i = 1; i <= 2; ++i)
			{
				if (get_scr_layer_valid(current_screen, i))
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[MAPCOMBO2(i-1,dx,dy)].type == cBRIDGE && !_walkflag_layer(dx,dy)) wtrx = false;
					}
					else
					{
						if (combobuf[MAPCOMBO2(i-1,dx,dy)].type == cBRIDGE && _effectflag_layer(dx,dy)) wtrx = false;
					}
				}
			}
			bool walkwater = (get_qr(qr_DROWN) && !iswaterex_z3(MAPCOMBO(dx,dy), -1, dx,dy));
			
			if (!wtrx && solid)
			{
				if (canladder)
				{
					*canladder = 0;
				}
			}
			
			if(d2==dir)
			{
				int32_t c = walkwater ? 0:8;
				int32_t b = walkwater ? 8:0;
				
				if(d2>=left)
				{
					if (wtrx)
					{
						//If the difference between dy and y is small enough
						//this is apparently a really crappy corner shove check?
						if((replay_version_check(26)) || (abs((dy)-(int32_t(y+c)))<=(b)))
						{
							// Don't activate the ladder if it would be entirely
							// over water and Hero has the flippers. This isn't
							// a good way to do this, but it's too risky
							// to make big changes to this stuff.
							bool deployLadder=true;
							int32_t lx=dx&0xF0;
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
								if (replay_version_check(26))
								{
									if (canladder)
									{
										if (*canladder == -1) *canladder = 1;
									}
								}
								else
								{
									ladderx = dx&0xF0;
									laddery = y;
									ladderdir = left;
									ladderstart = d2;
									solid = laddery!=y.getInt();
								}
							}
						}
					}
				}
				else if(d2<=down)
				{
					if (wtrx)
					{
						//Unsure if this actually needs a replay check but better safe than sorry?
						if (replay_version_check(26))
						{
							if (canladder)
							{
								if (*canladder == -1) *canladder = 1;
							}
						}
						// If the difference between dx and x is small enough
						if(!replay_version_check(26) && (abs((dx)-(int32_t(x+c)))<=(b)))
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
	}
	
	if (replay_version_check(26))
	{
		if (canladder)
		{
			if (solid && *canladder == 1) *canladder = 2;
		}
	}
	
	return solid && !is_autowalking();
}

bool HeroClass::scr_canmove(zfix dx, zfix dy, bool kb, bool ign_sv, int* canladder)
{
	if(walk_through_walls) return true;
	if(!(dx || dy)) return true;
	zfix bx = x, by = y+(bigHitbox?0:8); //left/top
	zfix rx = x+15.9999_zf, ry = y+15.9999_zf; //right/bottom
	zfix wid = 16, hei = bigHitbox ? 16 : 8;
	if(!ign_sv && dy < 0 && sideview_mode() && !getOnSideviewLadder() && action != sideswimming && action != sideswimhit && action != sideswimattacking)
		return false;
	
	bool nosolid = true;

	bool ret = true;
	
	if(dx && !dy)
	{
		if(dx < 0)
		{
			zfix mx = bx+dx;
			for(zfix ty = 0; by+ty < ry; ty += 8)
			{
				if(scr_walkflag(mx, by+ty, left, kb, canladder))
				{
					if (canladder) ret = false;
					else
					{
						return false;
					}
				}
			}
			if(scr_walkflag(mx, ry, left, kb, canladder))
				return false;
			if (!ret) return false;
			if(nosolid && collide_object(bx+dx,by,-dx,hei,this))
				return false;
		}
		else
		{
			zfix mx = rx+dx;
			int lx = mx-hit_width+1;
			for(zfix ty = 0; by+ty < ry; ty += 8)
			{
				if(scr_walkflag(mx, by+ty, right, kb, canladder))
				{
					if (canladder) ret = false;
					else
					{
						return false;
					}
				}
			}
			if(scr_walkflag(mx, ry, right, kb, canladder))
				return false;
			if (!ret) return false;
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
				if(scr_walkflag(bx+tx, my, up, kb, canladder))
				{
					if (canladder) ret = false;
					else
					{
						return false;
					}
				}
			}
			if(scr_walkflag(rx, my, up, kb, canladder))
				return false;
			if (!ret) return false;
			if(nosolid && collide_object(bx,by+dy,wid,-dy,this))
				return false;
		}
		else
		{
			zfix my = ry+dy;
			int ly = my-hit_height+1;
			for(zfix tx = 0; bx+tx < rx; tx += 8)
			{
				if(scr_walkflag(bx+tx, my, down, kb, canladder))
				{
					if (canladder) ret = false;
					else
					{
						return false;
					}
				}
			}
			if(scr_walkflag(rx, my, down, kb, canladder))
				return false;
			if (!ret) return false;
			if(nosolid && collide_object(bx,by+hei,wid,dy,this))
				return false;
		}
	}
	else //! Untested, and currently unused.
	{
		return scr_canmove(dx, 0, kb, ign_sv) && scr_canmove(dy, 0, kb, ign_sv);
	}
	return ret;
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

optional<zfix> HeroClass::get_solid_coord(zfix tx, zfix ty, byte dir, byte mdir, bool kb, zfix earlyterm, bool doladder)
{
	zfix tmp;
	switch(dir)
	{
		default:
		case up:
			tmp = binary_search_zfix(ty, earlyterm, [&](zfix ty, zfix& retval)
				{
					int laddercheck = -1;
					if(!scr_walkflag(tx,ty,mdir,kb, &laddercheck))
					{
						retval = ty;
						return BSEARCH_CONTINUE_UP;
					}
					else if(doladder && laddercheck > 0)
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
					int laddercheck = -1;
					if(!scr_walkflag(tx,ty,mdir,kb, &laddercheck))
					{
						retval = ty;
						return BSEARCH_CONTINUE_DOWN;
					}
					else if(doladder && laddercheck > 0)
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
					int laddercheck = -1;
					if(!scr_walkflag(tx,ty,mdir,kb, &laddercheck))
					{
						retval = tx;
						return BSEARCH_CONTINUE_UP;
					}
					else if(doladder && laddercheck > 0)
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
					int laddercheck = -1;
					if(!scr_walkflag(tx,ty,mdir,kb, &laddercheck))
					{
						retval = tx;
						return BSEARCH_CONTINUE_DOWN;
					}
					else if(doladder && laddercheck > 0)
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
		int laddercheck = -1;
		bool valid = scr_canmove(dx, dy, kb, true, &laddercheck);
		if (laddercheck > 0) valid = true;
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
	int ladderstuff = -1;
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
	
	bool skipdmg = earlyret || get_qr(qr_LENIENT_SOLID_DAMAGE) || get_qr(qr_NOSOLIDDAMAGECOMBOS) || hclk || ((z>0||fakez>0) && !(hero_scr->flags2&fAIRCOMBOS));
	if(dx)
	{
		if(scr_canmove(dx, 0, kb, ign_sv, &ladderstuff))
		{
			if (ladderstuff == 1)
			{
				zfix tx = (dx < 0 ? (x+dx) : (x+8+dx));
				zfix tx2 = (dx < 0 ? 15 : 0);
				zfix tx3 = (dx < 0 ? -8 : 8);
				ladderx = TRUNCATE_HALF_TILE(tx.getInt());
				laddery = y.getTrunc();
				if (((iswaterex_z3(MAPCOMBO(ladderx+tx2,y+9), -1, ladderx+tx2,y+9) != 0) || getpitfall(ladderx+tx2,y+9))
				&& ((iswaterex_z3(MAPCOMBO(ladderx+tx2,y+15), -1, ladderx+tx2,y+15) != 0) || getpitfall(ladderx+tx2,y+15)))
				{
					ladderdir = left;
					ladderstart = dir;
				}
				else if (((iswaterex_z3(MAPCOMBO(ladderx+tx2+tx3,y+9), -1, ladderx+tx2+tx3,y+9) != 0) || getpitfall(ladderx+tx2+tx3,y+9))
				&& ((iswaterex_z3(MAPCOMBO(ladderx+tx2+tx3,y+15), -1, ladderx+tx2+tx3,y+15) != 0) || getpitfall(ladderx+tx2+tx3,y+15)))
				{
					ladderx = TRUNCATE_HALF_TILE(tx.getInt()+tx3.getInt());
					ladderdir = left;
					ladderstart = dir;
				}
				else
				{
					ladderx = 0;
					laddery = 0;
				}
			}
			if (ladderstuff > 0) ladderstuff = 0;
			x += dx;
		}
		else
		{
			bool stopped = true;
			bool shoved = false;
			if(shove)
			{
				zfix tx = (dx < 0 ? (x-4) : (x+20));
				int v=bigHitbox?0:8;
				zfix ly = y+v;
				zfix ry = y+15.9999_zf;
				zfix ly2 = y+v+6;
				zfix ry2 = y+9.9999_zf;
				auto mdir = GET_XDIR(dx);
				int laddershove = -1;
				int ladderhit = 0;
				bool onladder = (ladderx + laddery);
				bool hit_top = scr_walkflag(tx,ly,mdir,false, &laddershove);
				if (laddershove > 0)
				{
					hit_top = false;
					ladderhit += 1;
				}
				laddershove = -1;
				bool hit_bottom = scr_walkflag(tx,ry,mdir,false, &laddershove);
				if (laddershove > 0)
				{
					hit_bottom = false;
					ladderhit += 2;
				}
				laddershove = -1;
				bool hit_top2 = scr_walkflag(tx,ly2,mdir,false);
				bool hit_bottom2 = scr_walkflag(tx,ry2,mdir,false);
				if (!hit_top && ladderhit == 2)
				{
					hit_bottom = true;
				}
				if (!hit_bottom && ladderhit == 1)
				{
					hit_top = true;
				}
				if(hit_top!=hit_bottom)
				{
					if(hit_bottom) //shove up
					{
						if (onladder && (ladderdir == left || ladderdir == right) && !hit_bottom2)
						{
							y -= 1_zf;
							shoved = true;
						}
						else if(skipdmg || !checkdamagecombos(tx,get_qr(qr_SENSITIVE_SOLID_DAMAGE)?int32_t(y+15):(v+bigHitbox?11:4)))
						{
							if(optional<zfix> ty = get_solid_coord(tx,ry,up,mdir,false,ry-shove_offset, (ladderhit != 2)))
							{
								zfix dy = zc_max(-1_zf,*ty-y);
								if((shoved = dy))
									movexy(0, dy, kb, true, false, false);
							}
						}
					}
					else //shove down
					{
						if (onladder && (ladderdir == left || ladderdir == right) && !hit_top2)
						{
							y += 1_zf;
							shoved = true;
						}
						else if(skipdmg || !checkdamagecombos(tx,v+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4)))
						{
							if(optional<zfix> ty = get_solid_coord(tx,ly,down,mdir,false,ly+shove_offset, (ladderhit != 1)))
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
				if (ladderstuff > 0 && !shoved)
				{
					zfix tx = (dx < 0 ? (x-12) : (x+20));
					ladderx = TRUNCATE_HALF_TILE(tx.getInt());
					laddery = y.getTrunc();
					ladderdir = left;
					ladderstart = dir;
					x += dx;
					ladderstuff = 0;
				}
				else if(earlyret)
				{
					ret = false;
				}
				else
				{
					ret = false;
					int xsign = dx.sign();
					if(abs(dx) > 1)
					{
						while(scr_canmove(xsign, 0, kb, ign_sv))
						{
							x += xsign;
							dx -= xsign;
						}
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
	}
	ladderstuff = -1;
	if(dy)
	{
		if(scr_canmove(0, dy, kb, ign_sv, &ladderstuff))
		{
			if (ladderstuff == 1)
			{
				zfix ty = (dy < 0 ? (y+(bigHitbox?0:8)+dy) : (y+8+dy));
				zfix ty2 = (dy < 0 ? 15 : 0);
				zfix ty3 = (dy < 0 ? -8 : 8);
				
				ladderx = x.getTrunc();
				laddery = TRUNCATE_HALF_TILE(ty.getInt());
				if (((iswaterex_z3(MAPCOMBO(x+4,laddery+ty2), -1, x+4,laddery+ty2) != 0) || getpitfall(x+4,laddery+ty2))
				&& ((iswaterex_z3(MAPCOMBO(x+11,laddery+ty2), -1, x+11,laddery+ty2) != 0) || getpitfall(x+11,laddery+ty2)))
				{
					ladderdir = up;
					ladderstart = dir;
				}
				else if (((iswaterex_z3(MAPCOMBO(x+4,laddery+ty2+ty3), -1, x+4,laddery+ty2+ty3) != 0) || getpitfall(x+4,laddery+ty2+ty3))
				&& ((iswaterex_z3(MAPCOMBO(x+11,laddery+ty2+ty3), -1, x+11,laddery+ty2+ty3) != 0) || getpitfall(x+11,laddery+ty2+ty3)))
				{
					laddery = TRUNCATE_HALF_TILE(ty.getInt() + ty3.getInt());
					ladderdir = up;
					ladderstart = dir;
				}
				else
				{
					ladderx = 0;
					laddery = 0;
				}
			}
			if (ladderstuff > 0) ladderstuff = 0;
			y += dy;
		}
		else
		{
			bool stopped = true;
			bool shoved = false;
			if(shove)
			{
				zfix ty = (dy < 0 ? (y+(bigHitbox?0:8)-4) : (y+20));
				zfix lx = x;
				zfix rx = x+15.9999_zf;
				zfix lx2 = x+6;
				zfix rx2 = x+9.9999_zf;
				auto mdir = GET_YDIR(dy);
				int laddershove = -1;
				int ladderhit = 0;
				bool onladder = (ladderx + laddery);
				bool hit_left = scr_walkflag(lx,ty,mdir,false, &laddershove);
				if (laddershove > 0)
				{
					hit_left = false;
					ladderhit += 1;
				}
				laddershove = -1;
				bool hit_right = scr_walkflag(rx,ty,mdir,false, &laddershove);
				if (laddershove > 0)
				{
					hit_right = false;
					ladderhit += 2;
				}
				bool hit_left2 = scr_walkflag(lx2,ty,mdir,false);
				bool hit_right2 = scr_walkflag(rx2,ty,mdir,false);
				laddershove = -1;
				if (!hit_left && ladderhit == 2)
				{
					hit_right = true;
				}
				if (!hit_right && ladderhit == 1)
				{
					hit_left = true;
				}
				if(hit_left!=hit_right)
				{
					if(hit_right) //shove left
					{
						if (onladder && ladderdir <= down && !hit_right2)
						{
							x -= 1_zf;
							shoved = true;
						}
						else if(skipdmg || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?15:11),ty))
						{
							if(optional<zfix> tx = get_solid_coord(rx,ty,left,mdir,false,rx-shove_offset, (ladderhit != 2)))
							{
								zfix dx = zc_max(-1_zf,*tx-x);
								if((shoved = dx))
									movexy(dx, 0, kb, true, false, false);
							}
						}
					}
					else //shove right
					{
						if (onladder && ladderdir <= down && !hit_left2)
						{
							x += 1_zf;
							shoved = true;
						}
						else if(skipdmg || !checkdamagecombos(x+(get_qr(qr_SENSITIVE_SOLID_DAMAGE)?0:4),ty))
						{
							if(optional<zfix> tx = get_solid_coord(lx,ty,right,mdir,false,lx+shove_offset, (ladderhit != 1)))
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
				if (ladderstuff > 0 && !shoved)
				{
					zfix ty = (dy < 0 ? (y-(bigHitbox?12:4)) : (y+20));
					ladderx = x.getTrunc();
					laddery = TRUNCATE_HALF_TILE(ty.getInt());
					ladderdir = up;
					ladderstart = dir;
					y += dy;
					ladderstuff = 0;
				}
				else
				{
					if(earlyret) return false;
					ret = false;
					int ysign = dy.sign();
					if(abs(dy) > 1)
					{
						while(scr_canmove(0, ysign, kb, ign_sv))
						{
							y += ysign;
							dy -= ysign;
						}
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
	}
	
	if(earlyret)
		return ret;
	if(dy < 0 && !ign_sv && sideview_mode() && IsSideSwim() && checkladder)
	{
		if(!iswaterex_z3(MAPCOMBO(x, y+(bigHitbox?0:8)-2), -1, x, y+(bigHitbox?0:8) - 2, true, false)
			&& !canSideviewLadderRemote(x, y-4) && scr_canmove(0, -2, kb, true) && (y+(bigHitbox?0:8) - 4) > 0)
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
	if(is_autowalking()) return true;
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
	
	tick_diving();
	
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
	int32_t currentSwordOrWand = (itemsbuf[dowpn].type == itype_wand || itemsbuf[dowpn].type == itype_sword)?dowpn:-1;
	if((!attackclk && action!=attacking && action != sideswimattacking) || ((attack==wSword || attack==wWand) && (itemsbuf[currentSwordOrWand].flags & item_flag5)))
	{
		if (getInput(btnB, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Bwpn);
			dowpn = NEG_OR_MASK(Bwpn,0xFFF);
			directWpn = directItemB;
		}
		else if (getInput(btnA, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Awpn);
			dowpn = NEG_OR_MASK(Awpn,0xFFF);
			directWpn = directItemA;
		}
		else if (get_qr(qr_SET_XBUTTON_ITEMS) && getInput(btnEx1, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Xwpn);
			dowpn = NEG_OR_MASK(Xwpn,0xFFF);
			directWpn = directItemX;
		}
		else if (get_qr(qr_SET_YBUTTON_ITEMS) && getInput(btnEx2, INPUT_PRESS | INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			btnwpn=getItemFamily(itemsbuf,Ywpn);
			dowpn = NEG_OR_MASK(Ywpn,0xFFF);
			directWpn = directItemY;
		}
		
		auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
		if (on_cooldown(itmid))
		{
			directWpn = olddirectwpn;
			btnwpn = -1;
			dowpn = -1;
			did_scriptb = false;
		}
		else
		{
			if(directWpn >= MAXITEMS) directWpn = -1;
			
			// The Quick Sword only allows repeated sword or wand swings.
			if((action==attacking||action==sideswimattacking) && ((attack==wSword && btnwpn!=itype_sword) || (attack==wWand && btnwpn!=itype_wand)))
				btnwpn=-1;
		}
	}
	
	auto swordid = (directWpn>-1 ? directWpn : current_item_id(itype_sword));
	if(can_attack() && (swordid > -1 && itemsbuf[swordid].type==itype_sword) && checkitem_jinx(swordid) && btnwpn==itype_sword && charging==0)
	{
		attackid=directWpn>-1 ? directWpn : current_item_id(itype_sword);
		if(checkbunny(attackid) && (checkmagiccost(attackid) || !(itemsbuf[attackid].flags & item_flag6)))
		{
			if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_SWORD_COST))
			{
				paymagiccost(attackid,true);
				misc_internal_hero_flags |= LF_PAID_SWORD_COST;
			}
			SetAttack();
			attack=wSword;
			
			attackclk=0;
			sfx(itemsbuf[directWpn>-1 ? directWpn : current_item_id(itype_sword)].usesound, pan(x));
			
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
		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && canSideviewLadder())
		{
			setOnSideviewLadder(true);
		}
		else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && canSideviewLadder(true))
		{
			y+=1;
			setOnSideviewLadder(true);
		}
	}
	
	int32_t wx=x;
	int32_t wy=y;
	if(conv_forcedir > -1 && !spins) dir = conv_forcedir;
	else if((action==none || action==walking) && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
	{
		if((xoff==0)||diagonalMovement)
		{
			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;
			if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;
		}
		
		if((yoff==0)||diagonalMovement)
		{
			if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;
			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
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
	bool liftonly = lift_wpn && (liftflags & LIFTFL_DIS_ITEMS);
	auto itmid = directWpn>-1 ? directWpn : current_item_id(btnwpn);
	if(liftonly)
	{
		if(replay_version_check(38) && btnwpn > -1)
		{
			no_jinx = checkitem_jinx(itmid);
			if(no_jinx)
				startwpn(itmid);
			directWpn = olddirectwpn;
		}
	}
	else if(can_attack() && btnwpn>itype_sword && charging==0 && btnwpn!=itype_rupee) // This depends on item 0 being a rupee...
	{
		bool paidmagic = false;
		if(btnwpn==itype_wand && (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_wand : false) : current_item(itype_wand)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_wand);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & item_flag6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_WAND_COST)){
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
		else if((btnwpn==itype_hammer)&&!((action==attacking||action==sideswimattacking) && attack==wHammer)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_hammer : false) : current_item(itype_hammer)))
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
		else if((btnwpn==itype_candle)&&!((action==attacking||action==sideswimattacking) && attack==wFire)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_candle : false) : current_item(itype_candle)))
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
		else if((btnwpn==itype_cbyrna)&&!((action==attacking||action==sideswimattacking) && attack==wCByrna)
				&& (directWpn>-1 ? (!item_disabled(directWpn) ? itemsbuf[directWpn].type==itype_cbyrna : false) : current_item(itype_cbyrna)))
		{
			attackid=directWpn>-1 ? directWpn : current_item_id(itype_cbyrna);
			no_jinx = checkitem_jinx(attackid);
			if(no_jinx && checkbunny(attackid) && ((!(itemsbuf[attackid].flags & item_flag6)) || checkmagiccost(attackid)))
			{
				if((itemsbuf[attackid].flags & item_flag6) && !(misc_internal_hero_flags & LF_PAID_CBYRNA_COST)){
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
		else if((btnwpn==itype_bugnet)&&!((action==attacking||action==sideswimattacking) && attack==wBugNet)
				&& (directWpn>-1 ? (!item_disabled(directWpn) && itemsbuf[directWpn].type==itype_bugnet) : current_item(itype_bugnet)))
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
		if(conv_forcedir > -1 && !spins) dir = conv_forcedir;
		else if((attackclk==0) && action!=sideswimattacking && getOnSideviewLadder() && (get_qr(qr_SIDEVIEWLADDER_FACEUP)!=0)) //Allow DIR to change if standing still on sideview ladder, and force-face up.
		{
			if((xoff==0)||diagonalMovement)
			{
				if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;
				if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;
			}
			
			if((yoff==0)||diagonalMovement)
			{
				if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;
				if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
			}
		}
		
		bool attacked = doattack();
		
		// This section below interferes with script-setting Hero->Dir, so it comes after doattack
		if(conv_forcedir > -1 && !spins) dir = conv_forcedir;
		else if(!inlikelike && attackclk>4 && (attackclk&3)==0 && charging==0 && spins==0 && action!=sideswimattacking)
		{
			if((xoff==0)||diagonalMovement)
			{
				if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = up;
				if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = down;

			}
			
			if((yoff==0)||diagonalMovement)
			{
				if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = left;
				if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION)) dir = right;
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
					sfx(itemsbuf[id].usesound,pan(x));
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
	bool earlyret = false;
	bool nohorz = (isdungeon() && (y<=26 || y>=world_h-42) && !get_qr(qr_FREEFORM) && !walk_through_walls);
	bool novert = (isdungeon() && (x<=26 || x>=world_w - 42) && !get_qr(qr_FREEFORM) && !walk_through_walls);
	zfix dx, dy;
	auto push=pushing;
	pushing=0;
	
	if(!is_conveyor_stunned && !is_autowalking()) //these do not apply to conveyor auto-walk
	{
		if(pitslide()) //Check pit's 'pull'. If true, then Hero cannot fight the pull.
			return;
		
		if(action==walking) //still walking
		{
			if (!getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && !getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && !autostep)
			{
				if(attackclk>0) SetAttack();
				else {action = none; FFCore.setHeroAction(none);}
				hero_count=-1;
				earlyret = true;
				goto newmove_slide;
			}
			
			autostep=false;
		} // endif (action==walking)
		
		if((action!=swimming)&&(action!=sideswimming)&&(action !=sideswimhit)&&(action !=sideswimattacking)&&(action!=casting)&&(action!=sideswimcasting)&&(action!=drowning)&&(action!=sidedrowning)&&(action!=lavadrowning) && charging==0 && spins==0 && jumping<1)
		{
			action=none; FFCore.setHeroAction(none);
		}
	}
	
	if(is_autowalking())
	{
		if(check_pitslide() > -1)
			try_hover();
	}
	else if(is_conveyor_stunned)
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
			if (!getInput(btnUp, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case down:
			if (!getInput(btnDown, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case left:
			if (!getInput(btnLeft, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		case right:
			if (!getInput(btnRight, INPUT_HERO_ACTION))
			{
				holddir=-1;
			}
			
			break;
			
		default:
			break;
		} //end switch

		if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == up) && !novert)
		{
			if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
			{
				dir=up;
			}
			holddir=up;

			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left && !nohorz)
			{
				shiftdir=right;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
			}
			else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right && !nohorz)
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
		else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == down) && !novert)
		{
			if(charging==0 && spins==0 && action != sideswimattacking && !(IsSideSwim() && get_qr(qr_SIDESWIMDIR)))
			{
				dir=down;
			}
			holddir=down;

			if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != left && !nohorz)
			{
				shiftdir=right;
				if (IsSideSwim() && get_qr(qr_SIDESWIMDIR) && (charging==0 && spins==0)) dir = right;
				if (!IsSideSwim() || (charging==0 && spins==0)) sideswimdir = right;
			}
			else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != right && !nohorz)
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
		else if (getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == left) && !nohorz)
		{
			if(charging==0 && spins==0 && action != sideswimattacking)
			{
				dir=left;
			}
			sideswimdir = left;
			holddir=left;

			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down && !novert)
			{
				shiftdir=up;
			}
			else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up && !novert)
			{
				shiftdir=down;
			}
			else
			{
				shiftdir=-1;
			}
		}
		else if (getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) && (holddir == -1 || holddir == right) && !nohorz)
		{
			if(charging==0 && spins==0 && action != sideswimattacking)
			{
				dir=right;
			}
			sideswimdir = right;
			holddir=right;

			if (getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != down && !novert)
			{
				shiftdir=up;
			}
			else if (getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION) && shiftdir != up && !novert)
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
			int32_t wtry  = iswaterex_z3(MAPCOMBO(x,y+15), -1, x,y+15, true, false);
			int32_t wtry8 = iswaterex_z3(MAPCOMBO(x+15,y+15), -1, x+15,y+15, true, false);
			int32_t wtrx = iswaterex_z3(MAPCOMBO(x,y+(bigHitbox?0:8)), -1, x,y+(bigHitbox?0:8), true, false);
			int32_t wtrx8 = iswaterex_z3(MAPCOMBO(x+15,y+(bigHitbox?0:8)), -1, x+15,y+(bigHitbox?0:8), true, false);
			int32_t wtrc = iswaterex_z3(MAPCOMBO(x+8,y+(bigHitbox?8:12)), -1, x+8,y+(bigHitbox?8:12), true, false);
			
			if(can_use_item(itype_flippers,i_flippers)&&current_item(itype_flippers) >= combobuf[wtrc].attribytes[0]&&(!(combobuf[wtrc].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))&&!(ladderx+laddery)&&z==0&&fakez==0)
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
			earlyret = true;
		}
		if(earlyret)
			dx = dy = 0;
		else get_move(holddir,dx,dy,dir);
	}
	else //4-way
	{
		shiftdir = -1;
		holddir = -1;
		if (!novert && getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			holddir = dir = up;
		}
		else if (!novert && getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			holddir = dir = down;
		}
		else if (!nohorz && getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			holddir = dir = left;
		}
		else if (!nohorz && getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION))
		{
			holddir = dir = right;
		}
		get_move(holddir,dx,dy,dir);
	}
	
newmove_slide:
	if(conv_forcedir > -1 && !spins)
		dir = conv_forcedir;
	if(!is_conveyor_stunned && !is_autowalking())
	{
		bool is_inair = (z > 0 || fakez > 0) || (sideview_mode() && !get_qr(qr_BROKEN_ICY_FLOOR_SIDEVIEW) && !isStanding(true));
		auto ic = ice_combo;
		if(!is_inair) //maintain momentum when jumping
		{
			const int sens = 2;
			if(sideview_mode() && !get_qr(qr_BROKEN_ICY_FLOOR_SIDEVIEW)) // check *below you* in sideview
			{
				int xs[] = {x+sens, x+15-sens};
				bool found = false;
				for(int q = 0; q < 2; ++q)
				{
					auto ice = get_icy(xs[q], y+16, ICY_PLAYER);
					if(ice)
					{
						ic = ice_combo = ice;
						found = true;
						break;
					}
				}
				if(!found)
					ic = ice_combo = 0;
			}
			else
			{
				auto ty = y+(bigHitbox?0:8);
				int xs[] = {x+7, x+sens, x+sens, x+15-sens, x+15-sens};
				int ys[] = {y+(bigHitbox?8:12), ty+sens, y+15-sens, ty+sens, y+15-sens};
				bool found = false;
				for(int q = 0; q < 5; ++q)
				{
					auto ice = get_icy(xs[q], ys[q], ICY_PLAYER);
					if(ice)
					{
						ic = ice_combo = ice;
						found = true;
						break;
					}
				}
				if(!found)
					ic = ice_combo = 0;
			}
		}
		if(script_ice_combo)
		{
			if((unsigned(script_ice_combo) < MAXCOMBOS) && check_icy(combobuf[script_ice_combo], ICY_PLAYER))
				ic = script_ice_combo;
			else ic = 0;
		}
		if(ic)
		{
			handle_slide(combobuf[ic], dx, dy);
			earlyret = false;
		}
		else clear_ice();
	}
	else clear_ice();
	if(earlyret)
		return;
	
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
	
	if (isSideViewHero() && slopeid)
	{
		dy += (dx*slopeid);
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
	if(!dx && !dy && !is_autowalking()) return true;
	if(action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
	{
		herostep();
		
		//ack... don't walk if in midair! -DD
		if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(this) && !getOnSideviewLadder()))
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
	
	if(is_autowalking())
		autowalk_move();
	else if(charging==0 || attack!=wHammer)
		return movexy(dx,dy,false,false,true);
	return true;
}

void HeroClass::moveOld(int32_t d2)
{
    static bool totalskip = false;
    
    if( inlikelike || lstunclock > 0 || is_conveyor_stunned)
        return;
	
    int32_t dx=0,dy=0;
    int32_t xstep=lsteps[x.getInt()&7];
    int32_t ystep=lsteps[y.getInt()&7];
    int32_t z3skip=0;
    int32_t z3diagskip=0;
    bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && ((z==0 && fakez == 0) || hero_scr->flags2&fAIRCOMBOS)) ||
                     (isSideViewHero() && (on_sideview_solid_oldpos(this)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement);
    bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & item_flag10);
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
        if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(this) && !getOnSideviewLadder()))
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
	
    bool slowcombo = (combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1, -1) && ((z==0 && fakez==0) || hero_scr->flags2&fAIRCOMBOS)) ||
                     (isSideViewHero() && (on_sideview_solid_oldpos(this)||getOnSideviewLadder()) && combo_class_buf[combobuf[MAPCOMBO(x+7,y+8)].type].slow_movement && _effectflag(x+7,y+8,1, -1));
		     //!DIMITODO: add QR for slow combos under hero
	if(slowcombo) for (int32_t i = 1; i <= 2; ++i)
	{
		if (get_scr_layer_valid(current_screen, i))
		{
			if (get_qr(qr_OLD_BRIDGE_COMBOS))
			{
				if (combobuf[MAPCOMBO2(i-1,x+7,y+8)].type == cBRIDGE && !_walkflag_layer(x+7,y+8))
				{
					slowcombo = false;
					break;
				}
			}
			else
			{
				if (combobuf[MAPCOMBO2(i-1,x+7,y+8)].type == cBRIDGE && _effectflag_layer(x+7,y+8))
				{
					slowcombo = false;
					break;
				}
			}
		}
	}
    bool slowcharging = charging>0 && (itemsbuf[getWpnPressed(itype_sword)].flags & item_flag10);
    bool is_swimming = (action == swimming);
	bool fastSwim = (zinit.hero_swim_speed>60);
	zfix rate(steprate);
	int32_t shieldid = getCurrentActiveShield();
	if(shieldid > -1)
	{
		itemdata const& shield = itemsbuf[shieldid];
		if(shield.flags & item_flag10) //Change Speed flag
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
						if (!iswaterex_z3(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
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
							if (!iswaterex_z3(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
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
							if (!iswaterex_z3(MAPCOMBO(x,y+8-(bigHitbox*8)+floor(up_step)), -1, x, y+8-(bigHitbox*8)-2, true, false)) checkladder = true;
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
	if (isSideViewHero() && slopeid)
	{
		dy += (dx * slopeid);
	}
	if(dx == 0 && dy == 0) return;
	if(action != swimming && action != sideswimming && action != sideswimhit && action != sideswimattacking)
	{
		herostep();
		
		//ack... don't walk if in midair! -DD
		if(charging==0 && spins==0 && z==0 && fakez==0 && !(isSideViewHero() && !on_sideview_solid_oldpos(this) && !getOnSideviewLadder()))
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

void HeroClass::start_auto_walk(const combined_handle_t& target)
{
	auto& cmb = target.combo();
	if(cmb.type != cCUTSCENEEFFECT || cmb.attribytes[0] != CUTEFF_PLAYER_WALK) return;
	
	zfix dx = zslongToFix(cmb.attributes[0]), dy = zslongToFix(cmb.attributes[1]);
	if(cmb.usrflags & cflag1)
	{
		auto [cx, cy] = target.center_xy();
		dx += cx-8;
		dy += cy-8;
	}
	else if(cmb.usrflags & cflag2)
	{
		dx += x;
		dy += y;
	}
	
	reset_hookshot();
	
	autowalk_dest_x = dx;
	autowalk_dest_y = dy;
	autowalk_combo_id = target.data();
	autowalk_handle = target;
}
void HeroClass::finish_auto_walk()
{
	if(!autowalk_handle) return;
	
	if(autowalk_combo_id >= 0 && autowalk_handle.data() == autowalk_combo_id)
	{
		auto& cmb = autowalk_handle.combo();
		if(cmb.type == cCUTSCENEEFFECT && cmb.attribytes[0] == CUTEFF_PLAYER_WALK)
			do_trigger_ctype_causes(autowalk_handle);
	}
	autowalk_handle = rpos_handle_t();
	autowalk_combo_id = -1;
	autowalk_dest_x = autowalk_dest_y = 0;
}

void HeroClass::autowalk_move()
{
	if(x == autowalk_dest_x && y == autowalk_dest_y) // reached destination already
	{
		finish_auto_walk();
		return;
	}
	
	zfix pix_dist;
	if(autowalk_combo_id > -1)
	{
		newcombo const& cmb = combobuf[autowalk_combo_id];
		if(cmb.type == cCUTSCENEEFFECT && cmb.attribytes[0] == CUTEFF_PLAYER_WALK)
		{
			if(cmb.attributes[2])
				pix_dist = zslongToFix(cmb.attributes[2]);
		}
	}
	if(!pix_dist)
	{
		pix_dist = steprate / 100_zf;
		vector<zfix*> vec = { &pix_dist };
		mod_steps(vec);
		if(!pix_dist) // ensure not stuck
			pix_dist = 1;
	}
	
	// -1, 0, or 1
	int dir_x = (autowalk_dest_x > x) - (autowalk_dest_x < x);
	int dir_y = (autowalk_dest_y > y) - (autowalk_dest_y < y);
	
	double _MSVC2022_tmp1, _MSVC2022_tmp2;
	double angle = atan2_MSVC2022_FIX(double(autowalk_dest_y-y),double(autowalk_dest_x-x));
	zfix dx, dy;
	if(x != autowalk_dest_x)
		dx = zc::math::Cos(angle)*pix_dist;
	if(y != autowalk_dest_y)
		dy = zc::math::Sin(angle)*pix_dist;
	
	zfix cur_dest_x = x+dx, cur_dest_y = y+dy;
	// check if this move will reach or pass the destination, and make it land exactly on the destination instead
	if(dir_x != ((autowalk_dest_x > cur_dest_x) - (autowalk_dest_x < cur_dest_x)))
	{
		cur_dest_x = autowalk_dest_x;
		dx = cur_dest_x - x;
	}
	if(dir_y != ((autowalk_dest_y > cur_dest_y) - (autowalk_dest_y < cur_dest_y)))
	{
		cur_dest_y = autowalk_dest_y;
		dy = cur_dest_y - y;
	}
	
	movexy(dx, dy, false, true, false, true); // try to move for purposes such as the ladder
	x = cur_dest_x; y = cur_dest_y; // ... but just set the x/y to be sure nothing got in the way
	
	if(dx || dy)
	{
		if(abs(dx) > abs(dy))
		{
			if(dx < 0) dir = left;
			else dir = right;
		}
		else
		{
			if(dy < 0) dir = up;
			else dir = down;
		}
	}
	
	if(x == autowalk_dest_x && y == autowalk_dest_y) // reached destination
		finish_auto_walk();
}

bool HeroClass::is_autowalking() const
{
	return autowalk_combo_id >= 0;
}
bool HeroClass::no_control() const
{
	return is_autowalking(); // can possibly add other stuff here later, like maybe a script bool?
}
HeroClass::WalkflagInfo HeroClass::walkflag(zfix fx,zfix fy,int32_t cnt,byte d2)
{
	return walkflag(fx.getInt(), fy.getInt(), cnt, d2);
}
HeroClass::WalkflagInfo HeroClass::walkflag(int32_t wx,int32_t wy,int32_t cnt,byte d2)
{
    WalkflagInfo ret;

	if (!is_in_world_bounds(wx, wy))
	{
		ret.setUnwalkable(false);
		return ret;
	}

    if(walk_through_walls)
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
    
    if(isdungeon() && cur_screen<128 && wy<(bigHitbox?32:40) && (((diagonalMovement||NO_GRIDLOCK)?(x<=112||x>=128):x!=120) || _walkflag(120,24,2,get_standing_z_state()))
            && !get_qr(qr_FREEFORM))
    {
        ret.setUnwalkable(true);
        return ret;
    }
    
    bool wf = _walkflag(wx,wy,cnt,get_standing_z_state());
    
    if(isdungeon() && cur_screen<128 && !get_qr(qr_FREEFORM))
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
		if (_walkflag(x+7,y+(bigHitbox?6:11),1,get_standing_z_state())
                || _walkflag(x+7,y+(bigHitbox?9:12),1,get_standing_z_state())
		|| _walkflag(x+8,y+(bigHitbox?6:11),1,get_standing_z_state())
                || _walkflag(x+8,y+(bigHitbox?9:12),1,get_standing_z_state())) isthissolid = true;
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
                    else if(wy>world_h - (show_bottom_8px ? 0 : 8))
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
                    if(d2==left)
                    {
                        if(!iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?6:11)), -1, x-1,y+(bigHitbox?6:11)) &&
                           !iswaterex_z3(MAPCOMBO(x-1,y+(bigHitbox?9:12)), -1, x-1,y+(bigHitbox?9:12)) &&
                           !_walkflag(x-1,y+(bigHitbox?6:11),1,get_standing_z_state()) &&
                           !_walkflag(x-1,y+(bigHitbox?9:12),1,get_standing_z_state()))
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
                           !_walkflag(x+16,y+(bigHitbox?6:11),1,get_standing_z_state()) &&
                           !_walkflag(x+16,y+(bigHitbox?9:12),1,get_standing_z_state()))
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
                           !_walkflag(x+7,y+(bigHitbox?0:8)-1,1,get_standing_z_state()) &&
                           !_walkflag(x+8,y+(bigHitbox?0:8)-1,1,get_standing_z_state()))
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
                           !_walkflag(x+7,y+16,1,get_standing_z_state()) &&
                           !_walkflag(x+8,y+16,1,get_standing_z_state()))
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
                else if(wy > world_h - (show_bottom_8px ? 0 : 8));
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
			if (d2>=left && iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy))
			{
				ret.setUnwalkable(false);
                return ret;
			}
			else if (d2<=down && iswaterex_z3(MAPCOMBO(wx,wy), -1, wx,wy) && iswaterex_z3(MAPCOMBO(x+8,wy), -1, x+8,wy))
			{
				ret.setUnwalkable(false);
                return ret;
			}
        }
    }
    else if(ladderx+laddery)                                  // ladder is being used
    {
        int32_t lx = !(get_qr(qr_DROWN)&&iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11)&&!_walkflag(x+4,y+11,1,get_standing_z_state())) ? zfix(wx) : x;
        int32_t ly = !(get_qr(qr_DROWN)&&iswaterex_z3(MAPCOMBO(x+4,y+11), -1, x+4,y+11)&&!_walkflag(x+4,y+11,1,get_standing_z_state())) ? zfix(wy) : y;
        
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
                        ret.setUnwalkable(_walkflag(ladderx,laddery-8,1,get_standing_z_state()) ||
                                          _walkflag(ladderx+8,laddery-8,1,get_standing_z_state()));
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
                    ret.setUnwalkable(_walkflag(ladderx,wy,1,get_standing_z_state()) || _walkflag(ladderx+8,wy,1,get_standing_z_state()));
                    return ret;
                }
                
                ret.setUnwalkable(_walkflag(TRUNCATE_TILE(wx),wy,1,get_standing_z_state()) || _walkflag(TRUNCATE_TILE(wx)+8,wy,1,get_standing_z_state()));
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
        bool unwalkablex  = _walkflag(wx,wy,1,get_standing_z_state()); //will be used later for the ladder -DD
        bool unwalkablex8 = _walkflag(x+8,wy,1,get_standing_z_state());
        
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
		if (current_item(itype_flippers) >= combobuf[wtrx8].attribytes[0] && (!(combobuf[wtrx8].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))) //Don't swim if the water's required level is too high! -Dimi
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
			bool ladderpits = ldrid > -1 && (itemsbuf[ldrid].flags&item_flag1);
            
            if(wtrx || wtrx8)
            {
                if(isSideViewHero())
                {
                    wtrx  = !_walkflag(wx, wy+8, 1,get_standing_z_state()) && !_walkflag(wx, wy, 1,get_standing_z_state()) && dir!=down;
                    wtrx8 = !_walkflag(wx+8, wy+8, 1,get_standing_z_state()) && !_walkflag(wx+8, wy, 1,get_standing_z_state()) && dir!=down;
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
                bool flgx  = _walkflag(wx,wy,1,get_standing_z_state()) && !wtrx; // Solid, and not steppable
                bool flgx8 = _walkflag(x+8,wy,1,get_standing_z_state()) && !wtrx8; // Solid, and not steppable
                
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
	if(walk_through_walls) return ret;
	if (mblock2.active())
		ret.setUnwalkable(mblock2.hit(wx,wy,0,1,1,1));
	if (collide_object(wx, wy,1, 1))
		ret.setUnwalkable(true);
	return ret;
}

bool HeroClass::checksoliddamage()
{
	if(walk_through_walls) return false;

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
	if(cmb.only_gentrig)
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
				auto& cmb = rpos_handle.combo();
				t = cmb.type;
				if(cmb.only_gentrig)
					t = cNONE;
				// Solid damage combos use pushing>0, hence the code is here.
				if (!get_qr(qr_LESS_AWFUL_SIDESPIKES) || !isSideViewHero() || (dir != down && (dir != up || getOnSideviewLadder())))
				{
					if(combo_class_buf[t].modify_hp_amount && _walkflag(bx,by,1,get_standing_z_state()) && pushing>0 && hclk<1 && action!=casting && action != sideswimcasting)
					{
						// Bite Hero
						if (checkdamagecombos(bx, bx, by, by, i-1, true)) return true;
					}
				}
				if (dir < left) bx += (k % 2) ? 7 : 8;
				else by += (k % 2) ? 7 : 8;
				if (!is_in_world_bounds(bx, by)) break;
				if (get_qr(qr_LENIENT_SOLID_DAMAGE)) break;
			}
		}
		if(isSideViewHero() && !get_qr(qr_NO_SIDEVIEW_SOLID_DAMAGE) && // Check for sideview damage combos
			hclk<1 && action!=casting && action!=sideswimcasting) // ... but only if Hero could be hurt
		{
			if (get_qr(qr_LESS_AWFUL_SIDESPIKES))
			{
				if (on_sideview_solid_oldpos(this) && (!getOnSideviewLadder() || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)))
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
					if (checkdamagecombos(x+8-(zfix)(hero_scr->csensitive),
						x+8+(zc_max(hero_scr->csensitive-1,0)),
						y+17-(get_qr(qr_LTTPCOLLISION)?hero_scr->csensitive:(hero_scr->csensitive+1)/2),
						y+17+zc_max((get_qr(qr_LTTPCOLLISION)?hero_scr->csensitive:(hero_scr->csensitive+1)/2)-1,0), i-1, true))
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
	if(walk_through_walls) return;
	
	if(z!=0||fakez!=0) return;
	
	// Return early in some cases..
	bool earlyReturn=false;
	
	if(!(diagonalMovement||NO_GRIDLOCK) || dir==left)
		if(x.getInt()&15) earlyReturn=true;

	if(isSideViewHero() && !on_sideview_solid_oldpos(this)) return;
	
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

	bool limitedpush = (itemid>=0 && itemsbuf[itemid].flags & item_flag1);
	itemdata const* glove = itemid < 0 ? NULL : &itemsbuf[itemid];
	for(int lyr = 2; lyr >= 0; --lyr) //Top-down, in case of stacked push blocks
	{
		if(get_qr(qr_HESITANTPUSHBLOCKS)&&(pushing<4)) break;
		if(lyr && !get_qr(qr_PUSHBLOCK_LAYER_1_2))
			continue;
		auto rpos_handle = get_rpos_handle(rpos, lyr);
		cpos_info& cpinfo = rpos_handle.info();
		mapscr* m = rpos_handle.scr;
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
		
		if(waitblock && (pushing<16 || hasMainGuy(m->screen))) continue;
		
		if(heavy && (itemid<0 || glove->power < heavy ||
			(limitedpush && usecounts[itemid] >= zc_max(1, glove->misc3)))) continue;
		
		bool doit=false;
		bool changecombo=false;
		
		int blockdir = dir;
		if(blockdir > 3) blockdir = Y_DIR(dir);
		if(t == cPUSHBLOCK && (get_qr(qr_BROKEN_GENERIC_PUSHBLOCK_LOCKING) || (f!=mfPUSHED)))
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
			bool cancel_opp = (cmb.usrflags & cflag9);
			if(cmb.usrflags & cflag5) //Separate directions
			{
				int limit = cmb.attribytes[4+blockdir];
				if(limit || (cmb.usrflags & cflag9)) // don't limit if limit should be infinite
				{
					int pushes = cpinfo.pushes[blockdir];
					if(cancel_opp)
						pushes -= cpinfo.pushes[oppositeDir[blockdir]];
					if(pushes >= limit)
						doit = false;
				}
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
			if(is_push_flag(f,dir))
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
					if(MAPFLAG2(lyr-1,bx,by-8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx,by-8)==mfBLOCKHOLE)
						break; // ignore solidity for BLOCKHOLE flagged combos
					if(_walkflag(bx,by-8,2,get_standing_z_state()))
						doit = false;
					else if(!get_qr(qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS) && _walkflag(bx,by-16,2,get_standing_z_state()))
						doit = false; // top half wasn't checked before...
					break;
					
				case down:
					if(MAPFLAG2(lyr-1,bx,by+24)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx,by+24)==mfBLOCKHOLE)
						break; // ignore solidity for BLOCKHOLE flagged combos
					if(_walkflag(bx,by+24,2,get_standing_z_state()))
						doit = false;
					else if(!get_qr(qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS) && _walkflag(bx,by+16,2,get_standing_z_state()))
						doit = false; // top half wasn't checked before...
					break;
					
				case left:
					if(MAPFLAG2(lyr-1,bx-16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx-16,by+8)==mfBLOCKHOLE)
						break; // ignore solidity for BLOCKHOLE flagged combos
					if(_walkflag(bx-16,by+8,2,get_standing_z_state()))
						doit = false;
					else if(!get_qr(qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS) && _walkflag(bx-16,by,2,get_standing_z_state()))
						doit = false; // top half wasn't checked before...
					break;
					
				case right:
					if(MAPFLAG2(lyr-1,bx+16,by+8)==mfBLOCKHOLE||MAPCOMBOFLAG2(lyr-1,bx+16,by+8)==mfBLOCKHOLE)
						break; // ignore solidity for BLOCKHOLE flagged combos
					if(_walkflag(bx+16,by+8,2,get_standing_z_state()))
						doit = false;
					else if(!get_qr(qr_BROKEN_PUSHBLOCK_TOP_HALF_SOLIDS) && _walkflag(bx+16,by,2,get_standing_z_state()))
						doit = false; // top half wasn't checked before...
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
				if(is_push_flag(f,dir)) // Only erase push flags, because only push flags should be copied in the block code.
					m->sflag[combopos] = 0;
				
				mblock2.blockLayer = lyr;
				
				if(t == cPUSHBLOCK)
				{
					zfix blockstep = 0.5;
					if(cmb.attrishorts[0] > 0)
						blockstep = zslongToFix(cmb.attrishorts[0]*100);
					mblock2.push_new(zfix(bx),zfix(by),blockdir,f,blockstep);
					mblock2.blockinfo = cpinfo;
					mblock2.blockinfo.push(blockdir, cmb.usrflags&cflag8);
					cpinfo.clearInfo();
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
			(itemsbuf[itemid].flags & item_flag1 ? itemsbuf[itemid].power<dlevel
			 : itemsbuf[itemid].power!=dlevel))
	{
		if(game->lvlkeys[dlevel]!=0)
		{
			game->lvlkeys[dlevel]--;
			//run script for level key item
			int32_t key_item = 0; //current_item_id(itype_lkey); //not possible
			for ( int32_t q = 0; q < MAXITEMS; ++q )
			{
				if ( itemsbuf[q].type == itype_lkey )
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
					if ( itemsbuf[q].type == itype_key )
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
            (itemsbuf[itemid].flags & item_flag1 ? itemsbuf[itemid].power<dlevel
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
    bool ret = (((mc==7||mc==8||mc==23||mc==24) && origin_scr->door[up]==lock)
                || ((mc==151||mc==152||mc==167||mc==168) && origin_scr->door[down]==lock)
                || ((mc==64||mc==65||mc==80||mc==81) && origin_scr->door[left]==lock)
                || ((mc==78||mc==79||mc==94||mc==95) && origin_scr->door[right]==lock));
    return ret;
}

void HeroClass::oldchecklockblock()
{
	if(walk_through_walls) return;
	
	int32_t bx = TRUNCATE_TILE(x.getInt());
	int32_t bx2 = TRUNCATE_TILE(x.getInt() + 8);
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
	
	bool found1=false;
	bool found2=false;
	int32_t foundlayer = -1;
	int32_t cid1 = MAPCOMBO(bx, by), cid2 = MAPCOMBO(bx2, by);
	newcombo const& cmb = combobuf[cid1];
	newcombo const& cmb2 = combobuf[cid2];
	if((cmb.type==cLOCKBLOCK && !(cmb.only_gentrig) && _effectflag(bx,by,1, -1)))
	{
		// Context: https://discord.com/channels/876899628556091432/1278165595321405554
		// Layer 0 is overridden by Locked Doors (but only for dungeons) - in that case, checklocked will clear these combos
		bool ignore_layer_0 = isdungeon() && islockeddoor(bx,by,dLOCKED);
		if (!ignore_layer_0)
		{
			found1=true;
			foundlayer = 0;
		}
	}
	else if (cmb2.type==cLOCKBLOCK && !(cmb2.only_gentrig) && _effectflag(bx2,by,1, -1))
	{
		bool ignore_layer_0 = isdungeon() && islockeddoor(bx2,by,dLOCKED);
		if (!ignore_layer_0)
		{
			found2=true;
			foundlayer = 0;
		}
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
			if(cmb.type==cLOCKBLOCK && !(cmb.only_gentrig) && _effectflag(bx,by,1, i))
			{
				found1=true;
				foundlayer = i+1;
				break;
			}
			else if(cmb2.type==cLOCKBLOCK && !(cmb2.only_gentrig) && _effectflag(bx2,by,1, i))
			{
				found2=true;
				foundlayer = i+1;
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
		setxmapflag(rpos_handle.screen, 1<<cmb.attribytes[5]);
		remove_xstatecombos(create_screen_handles(rpos_handle.base_scr()), 1<<cmb.attribytes[5], false);
	}
	else
	{
		setmapflag(rpos_handle.scr, mLOCKBLOCK);
		remove_lockblocks(create_screen_handles(rpos_handle.base_scr()));
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
	if(walk_through_walls) return;
	
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
	int cmb_screen_index = 0;

	if((cmb.type==cBOSSLOCKBLOCK && !(cmb.only_gentrig) && _effectflag(bx,by,1, -1)))
	{
		// Context: https://discord.com/channels/876899628556091432/1278165595321405554
		// Layer 0 is overridden by Locked Doors (but only for dungeons) - in that case, checklocked will clear these combos
		bool ignore_layer_0 = isdungeon() && islockeddoor(bx,by,dBOSS);
		if (!ignore_layer_0)
		{
			found1=true;
			foundlayer = 0;
			cmb_screen_index = get_screen_for_world_xy(bx, by);
		}
	}
	else if (cmb2.type==cBOSSLOCKBLOCK && !(cmb2.only_gentrig) && _effectflag(bx2,by,1, -1))
	{
		bool ignore_layer_0 = isdungeon() && islockeddoor(bx2,by,dBOSS);
		if (!ignore_layer_0)
		{
			found2=true;
			foundlayer = 0;
			cmb_screen_index = get_screen_for_world_xy(bx2, by);
		}
	}

	for (int32_t i = 0; i <= 1; ++i)
	{
		if (get_qr(qr_OLD_BRIDGE_COMBOS))
		{
			if (combobuf[MAPCOMBO2(i, bx, by)].type == cBRIDGE && !_walkflag_layer(bx, by, i)) found1 = false;
			if (combobuf[MAPCOMBO2(i, bx2, by)].type == cBRIDGE && !_walkflag_layer(bx2, by, i)) found2 = false;
		}
		else
		{
			if (combobuf[MAPCOMBO2(i, bx, by)].type == cBRIDGE && _effectflag_layer(bx, by, i)) found1 = false;
			if (combobuf[MAPCOMBO2(i, bx2, by)].type == cBRIDGE && _effectflag_layer(bx2, by, i)) found2 = false;
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
				if (get_scr_layer_valid(current_screen, 2))
				{
					if (get_qr(qr_OLD_BRIDGE_COMBOS))
					{
						if (combobuf[cid1].type == cBRIDGE && !_walkflag_layer(bx, by)) continue;
						if (combobuf[cid2].type == cBRIDGE && !_walkflag_layer(bx2, by)) continue;
					}
					else
					{
						if (combobuf[cid1].type == cBRIDGE && _effectflag_layer(bx, by)) continue;
						if (combobuf[cid2].type == cBRIDGE && _effectflag_layer(bx2, by)) continue;
					}
				}
			}
			if (cmb.type == cBOSSLOCKBLOCK && !(cmb.only_gentrig) && _effectflag(bx, by, 1, i))
			{
				found1 = true;
				foundlayer = i;
				cmb_screen_index = get_screen_for_world_xy(bx, by);
				break;
			}
			else if (cmb2.type == cBOSSLOCKBLOCK && !(cmb2.only_gentrig) && _effectflag(bx2, by, 1, i))
			{
				found2 = true;
				foundlayer = i;
				cmb_screen_index = get_screen_for_world_xy(bx2, by);
				break;
			}
		}
	}

	if (!(found1 || found2) || pushing < 8)
	{
		return;
	}
	int32_t cid = found1 ? cid1 : cid2;
	
	if(!(game->lvlitems[dlevel]&(1 << li_boss_key))) return;
	
	
	// Run Boss Key Script
	int32_t key_item = 0; //current_item_id(itype_bosskey); //not possible
	for ( int32_t q = 0; q < MAXITEMS; ++q )
	{
		if ( itemsbuf[q].type == itype_bosskey )
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
	
	mapscr* scr = get_scr(cmb_screen_index);
	if(cmb.usrflags&cflag16)
	{
		setxmapflag(cmb_screen_index, 1<<cmb.attribytes[5]);
		remove_xstatecombos(create_screen_handles(scr), 1<<cmb.attribytes[5]);
	}
	else
	{
		setmapflag(scr, mBOSSLOCKBLOCK);
		remove_bosslockblocks(create_screen_handles(scr));
	}
	if ( (combobuf[cid].attribytes[3]) )
		sfx(combobuf[cid].attribytes[3]);
}

void HeroClass::oldcheckchest(int32_t type)
{
	// chests aren't affected by hero_scr->flags2&fAIRCOMBOS
	if(walk_through_walls || z>0 || fakez > 0) return;
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
		found_screen_index=get_screen_for_world_xy(bx, by);
	}
	else if (combobuf[MAPCOMBO(bx2,by)].type==type && _effectflag(bx2,by,1, -1))
	{
		found=true;
		found_screen_index=get_screen_for_world_xy(bx2, by);
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
				found_screen_index=get_screen_for_world_xy(bx, by);
				break;
			}
			else if (combobuf[MAPCOMBO2(i,bx2,by)].type==type && _effectflag(bx2,by,1, i))
			{
				found=true;
				found_screen_index=get_screen_for_world_xy(bx2, by);
				break;
			}
		}
	}
	
	if(!found)
	{
		return;
	}

	mapscr* scr = get_scr(found_screen_index);
	
	switch(type)
	{
		case cLOCKEDCHEST:
			if(!usekey()) return;
			
			setmapflag(scr, mLOCKEDCHEST);
			break;
			
		case cCHEST:
			setmapflag(scr, mCHEST);
			break;
			
		case cBOSSCHEST:
			if(!(game->lvlitems[dlevel]&(1 << li_boss_key))) return;
			// Run Boss Key Script
			int32_t key_item = 0; //current_item_id(itype_bosskey); //not possible
			for ( int32_t q = 0; q < MAXITEMS; ++q )
			{
				if ( itemsbuf[q].type == itype_bosskey )
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
			setmapflag(scr, mBOSSCHEST);
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
		add_item_for_screen(found_screen_index, new item(x, y,(zfix)0, scr->catchall, ipONETIME2 + ipBIGRANGE + ipHOLDUP | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0), 0));
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
	if(walk_through_walls || z>0 || fakez > 0) return;
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
	
	if(cmb->type==type && !(cmb->only_gentrig) && _effectflag(bx,by,1, -1))
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
		if(cmb->type==type && !(cmb->only_gentrig) && _effectflag(bx2,by2,1, -1))
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
			if(combobuf[MAPCOMBO2(i,bx,by)].type==type && !(cmb->only_gentrig) && _effectflag(bx,by,1, i))
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
			if(combobuf[MAPCOMBO2(i,bx2,by2)].type==type && !(cmb->only_gentrig) && _effectflag(bx2,by2,1, i))
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
				|| (isbosslocked && !(game->lvlitems[dlevel]&(1 << li_boss_key)))))
				prompt_combo = altcmb;
			prompt_cset = cmb->attribytes[4];
			prompt_x = cmb->attrishorts[0];
			prompt_y = cmb->attrishorts[1];
		}
		if(!getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK))
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
	if (rpos == rpos_t::None)
		return;

	for (int layer = 0; layer < 7; ++layer)
	{
		auto rpos_handle = get_rpos_handle(rpos, layer);
		trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
			if(!trig.trigger_flags.get(TRIGFLAG_PUSH)) return false;
			return pushing && !(pushing % zc_max(1,trig.trig_pushtime));
		});
	}
}

void HeroClass::checkgenpush()
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

	rpos_t rpos_1 = COMBOPOS_REGION_B(bx, by);
	rpos_t rpos_2 = COMBOPOS_REGION_B(bx2, by2);
	checkgenpush(rpos_1);
	if (rpos_1 != rpos_2) checkgenpush(rpos_2);

	if (!get_qr(qr_OLD_FFC_FUNCTIONALITY))
	{
		for_some_ffcs([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by) || ffcIsAt(ffc_handle, bx2, by2))
			{
				trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
					if(!trig.trigger_flags.get(TRIGFLAG_PUSH)) return false;
					return pushing && !(pushing % zc_max(1,trig.trig_pushtime));
				});
			}
			return true;
		});
	}
}

void HeroClass::checksigns() //Also checks for generic trigger buttons
{
	if(walk_through_walls || z>0 || fakez>0) return;
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
	int32_t found_screen = -1;
	std::optional<ffc_handle_t> foundffc;
	int32_t found_lyr = 0;
	bool found_sign = false;
	int32_t tmp_cid = MAPCOMBO(bx, by);
	int32_t screen = get_screen_for_world_xy(bx, by);
	newcombo const* tmp_cmb = &combobuf[tmp_cid];
	bool has_trigger_button = false;
	for(auto& trig : tmp_cmb->triggers)
		if(trig.triggerbtn)
		{
			has_trigger_button = true;
			break;
		}
	if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->only_gentrig))
		|| has_trigger_button) && _effectflag(bx,by,1, -1))
	{
		found = tmp_cid;
		found_screen = screen;
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
	screen = get_screen_for_world_xy(bx2, by2);
	tmp_cmb = &combobuf[tmp_cid];
	has_trigger_button = false;
	for(auto& trig : tmp_cmb->triggers)
		if(trig.triggerbtn)
		{
			has_trigger_button = true;
			break;
		}
	if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->only_gentrig))
		|| has_trigger_button) && _effectflag(bx2,by2,1, -1))
	{
		found = tmp_cid;
		found_screen = screen;
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
		foundffc = find_ffc([&](const ffc_handle_t& ffc_handle) {
			if (ffcIsAt(ffc_handle, bx, by) || ffcIsAt(ffc_handle, bx2, by2))
			{
				tmp_cmb = &ffc_handle.combo();
				has_trigger_button = false;
				for(auto& trig : tmp_cmb->triggers)
					if(trig.triggerbtn)
					{
						has_trigger_button = true;
						break;
					}
				if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->only_gentrig))
				|| has_trigger_button) && true) //!TODO: FFC effect flag?
				{
					return true;
				}
			}

			return false;
		});
	}
	
	if(found<0 && !foundffc)
	{
		screen = get_screen_for_world_xy(bx, by);
		for(int32_t i=0; i<6; i++)
		{
			tmp_cid = MAPCOMBO2(i,bx,by);
			tmp_cmb = &combobuf[tmp_cid];
			has_trigger_button = false;
			for(auto& trig : tmp_cmb->triggers)
				if(trig.triggerbtn)
				{
					has_trigger_button = true;
					break;
				}
			if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->only_gentrig))
				|| has_trigger_button) && _effectflag(bx,by,1, i))
			{
				found = tmp_cid;
				found_screen = screen;
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
			screen = get_screen_for_world_xy(bx2, by2);
			tmp_cid = MAPCOMBO2(i,bx2,by2);
			tmp_cmb = &combobuf[tmp_cid];
			has_trigger_button = false;
			for(auto& trig : tmp_cmb->triggers)
				if(trig.triggerbtn)
				{
					has_trigger_button = true;
					break;
				}
			if(((tmp_cmb->type==cSIGNPOST && !(tmp_cmb->only_gentrig))
				|| has_trigger_button) && _effectflag(bx2,by2,1, i))
			{
				found = tmp_cid;
				found_screen = screen;
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
	
	byte signInput = 0;
	bool didsign = false, didprompt = false;
	if(cmb.type == cSIGNPOST && !(cmb.only_gentrig))
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
			signInput = getIntBtnInput(intbtn, INPUT_PRESS | INPUT_DRUNK);
			if(!signInput)
			{
				if(cmb.usrflags & cflag13) //display prompt
				{
					prompt_combo = cmb.attributes[1]/10000;
					prompt_cset = cmb.attribytes[4];
					prompt_x = cmb.attrishorts[0];
					prompt_y = cmb.attrishorts[1];
					didprompt = true;
				}
				goto endsigns; //Button not pressed
			}
		}
		else if(pushing < 8 || pushing%8) goto endsigns; //Not pushing against sign enough
		
		trigger_sign(cmb, foundffc ? foundffc->screen : found_screen);
		didsign = true;
	}
endsigns:
	auto& cpos = cpos_get(get_rpos_handle_for_world_xy(fx, fy, found_lyr));
	int dir_trigflag;
	switch(dir)
	{
		case down:
			dir_trigflag = TRIGFLAG_BTN_TOP;
			break;
		case up:
			dir_trigflag = TRIGFLAG_BTN_BOTTOM;
			break;
		case right:
			dir_trigflag = TRIGFLAG_BTN_LEFT;
			break;
		case left:
			dir_trigflag = TRIGFLAG_BTN_RIGHT;
			break;
	}
	bool found_a_trigger_dir = false, did_trigger = false;
	combined_handle_t comb_handle;
	if(fx != -1 && fy != -1)
		comb_handle = get_rpos_handle_for_world_xy(fx, fy, found_lyr);
	if(foundffc)
		comb_handle = foundffc.value();
	did_trigger = trig_each_combo_trigger(comb_handle, [&](combo_trigger const& trig, size_t idx){
		if(!trig.trigger_flags.get(dir_trigflag)) return false;
		found_a_trigger_dir = true;
		auto& trig_data = cpos.trig_data[idx];
		if(fx != -1 && fy != -1 && trig_data.cooldown) return false;
		return trig.triggerbtn && (getIntBtnInput(trig.triggerbtn, INPUT_PRESS | INPUT_DRUNK) || checkIntBtnVal(trig.triggerbtn, signInput));
	});
	if(!found_a_trigger_dir || didprompt || did_trigger)
		return;
	else if(cmb.type == cBUTTONPROMPT)
	{
		prompt_combo = cmb.attributes[0]/10000;
		prompt_cset = cmb.attribytes[0];
		prompt_x = cmb.attrishorts[0];
		prompt_y = cmb.attrishorts[1];
	}
	else for (size_t idx = 0; idx < cmb.triggers.size(); ++idx)
	{
		auto& trig_data = cpos.trig_data[idx];
		if(fx != -1 && fy != -1 && trig_data.cooldown) continue;
		auto& trig = cmb.triggers[idx];
		bool cond = check_trig_conditions(comb_handle, idx);
		auto pcid = cond ? trig.prompt_cid : trig.fail_prompt_cid;
		auto pcs = cond ? trig.prompt_cs : trig.fail_prompt_cs;
		if(pcid)
		{
			prompt_combo = pcid;
			prompt_cset = pcs;
			prompt_x = trig.prompt_x;
			prompt_y = trig.prompt_y;
			break;
		}
	}
}

// Checks for locked doors, and potentially unlocks them.
// Only looks at `current_screen`.
void HeroClass::checklocked()
{
	if(walk_through_walls) return; //Walk through walls. 
	if(!isdungeon(current_screen)) return;
	if( !diagonalMovement && pushing!=8) return;
	//This is required to allow the player to open a door, while sliding along a wall (pressing in the direction of the door, and sliding left or right)
	if ( diagonalMovement && pushing < 8 ) return; //Allow wall walking Should I add a quest rule for this? -Z

	auto [offx, offy] = translate_screen_coordinates_to_world(current_screen);
	int x = this->x - offx;
	int y = this->y - offy;

	optional<int> openDir;
	if ( diagonalMovement || NO_GRIDLOCK) 
	{
		if(y <= 32 && x >= 112 && x <= 128 && Y_DIR(dir) == up)
			openDir = up;
		else if(y >= 128 && x >= 112 && x <= 128 && Y_DIR(dir) == down)
			openDir = down;
		else if(y > 72 && y < 88 && x <= 32 && X_DIR(dir) == left)
			openDir = left;
		else if(y > 72 && y < 88 && x >= 208 && X_DIR(dir) == right)
			openDir = right;
	}
	else
	{
		if(y <= 32 && x == 120 && Y_DIR(dir) == up)
			openDir = up;
		else if(y >= 128 && x == 120 && Y_DIR(dir) == down)
			openDir = down;
		else if(y == 80 && x <= 32 && X_DIR(dir) == left)
			openDir = left;
		else if(y == 80 && x >= 208 && X_DIR(dir) == right)
			openDir = right;
	}

	if(openDir)
	{
		int d = *openDir;
		if (hero_scr->door[d]==dLOCKED)
		{
			if(usekey())
			{
				putdoor(hero_scr, scrollbuf, d, dUNLOCKED);
				hero_scr->door[d]=dUNLOCKED;
				set_doorstate(current_screen, d);
				sfx(WAV_DOOR);
				markBmap();

				// set_doorstate updates the door state of the opposite screen too, but it doesn't
				// update anything for the current region. Do that here.
				if (mapscr* opp_scr = get_scr_current_region_dir(current_screen, (direction)d))
				{
					if (opp_scr->door[d^1] == dLOCKED)
					{
						opp_scr->door[d^1] = dUNLOCKED;
						putdoor(opp_scr, scrollbuf, d^1, dUNLOCKED);
					}
				}
			}
			else return;
		}
		else if(hero_scr->door[d]==dBOSS)
		{
			if(game->lvlitems[dlevel]&(1 << li_boss_key))
			{
				putdoor(hero_scr, scrollbuf, d, dOPENBOSS);
				hero_scr->door[d]=dOPENBOSS;
				set_doorstate(current_screen, d);
				sfx(WAV_DOOR);
				markBmap();

				// set_doorstate updates the door state of the opposite screen too, but it doesn't
				// update anything for the current region. Do that here.
				if (mapscr* opp_scr = get_scr_current_region_dir(current_screen, (direction)d))
				{
					if (opp_scr->door[d^1] == dBOSS)
					{
						opp_scr->door[d^1] = dOPENBOSS;
						putdoor(opp_scr, scrollbuf, d^1, dOPENBOSS);
					}
				}

				// Run Boss Key Script
				for ( int32_t q = 0; q < MAXITEMS; ++q )
					if ( itemsbuf[q].type == itype_bosskey )
					{
						if (itemsbuf[q].script && !(FFCore.doscript(ScriptType::Item, q) && get_qr(qr_ITEMSCRIPTSKEEPRUNNING)))
						{
							FFCore.reset_script_engine_data(ScriptType::Item, q);
							ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[q].script, q);
							FFCore.deallocateAllScriptOwned(ScriptType::Item, q);
						}
						break;
					}
			}
			else return;
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
		if (!getInput(btnUp, INPUT_HERO_ACTION)) return;
		
		by-=16;
		break;
		
	case down:
		if (!getInput(btnDown, INPUT_HERO_ACTION)) return;
		
		by+=16;
		bx+=8;
		break;
		
	case left:
		if (!getInput(btnLeft, INPUT_HERO_ACTION)) return;
		
		bx-=16;
		by+=8;
		break;
		
	case right:
		if (!getInput(btnRight, INPUT_HERO_ACTION)) return;
		
		bx+=16;
		by+=8;
		break;
	}
	
	if(!_walkflag(bx,by,0,get_standing_z_state())) return;
	
	attackclk=SWORDTAPFRAME;
	pushing=-8; //16 frames between taps
	tapping=true;

	if (!isCuttableType(COMBOTYPE(bx, by)))
	{
		int tap_sfx = -1;
		bool hollow = false;
		for(int lyr = 6; lyr >= 0; --lyr)
		{
			auto rpos_handle_lyr = get_rpos_handle_for_world_xy(bx, by, lyr);
			auto& cmb = rpos_handle_lyr.combo();
			if(cmb.sfx_tap)
			{
				tap_sfx = cmb.sfx_tap;
				break;
			}

			if(rpos_handle_lyr.sflag() == mfBOMB || rpos_handle_lyr.sflag() == mfSBOMB
				|| cmb.flag == mfBOMB || cmb.flag == mfSBOMB)
				hollow = true;
		}
		if(tap_sfx < 0 && get_qr(qr_SEPARATE_BOMBABLE_TAPPING_SFX))
		{
			if(hollow || (!is_in_scrolling_region() && origin_scr->door[dir]==dBOMB && ((dir==up||dir==down)
					? (bx>=112 && bx<144 && (by>=144 || by<=32))
					: by>=72 && by<104 && (bx>=224 || bx<=32))))
				tap_sfx = QMisc.miscsfx[sfxTAP_HOLLOW];
		}
		if(tap_sfx < 0)
			tap_sfx = QMisc.miscsfx[sfxTAP];
		if(tap_sfx)
			sfx(tap_sfx,pan(x));
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

static int32_t beamGrabComboFromPos(rpos_t rpos, int32_t type)
{
	for (int lyr = 6; lyr >= 0; --lyr)
	{
		auto rpos_handle = get_rpos_handle(rpos, lyr);
		if (rpos_handle.ctype() == type)
			return rpos_handle.data();
	}

	return -1;
}

typedef word spot_t;
static std::vector<int32_t> typeMap;
static std::vector<int32_t> customTypeMap;
static std::vector<int32_t> istrig;
// static std::map<int32_t, std::map<size_t, word>> MAPS_prism_dir_seen_map;
static rpos_t beam_hero_rpos = rpos_t::None;
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
	rpos_t pos() const
	{
		return COMBOPOS_REGION(vbound(x,0,world_w-1),vbound(y,0,world_h-1));
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

static void handleBeam(spot_t* grid, size_t age, byte spotdir, rpos_t rpos, byte set, bool block, bool refl)
{
	if(spotdir > 3) return; //invalid dir

	int combos_wide = cur_region.screen_width * 16;
	int combos_tall = cur_region.screen_height * 11;
	int32_t trigflag = set ? (1 << (set-1)) : ~0;
	bool doAge = true;
	spot_t f = 0;
	while (rpos < region_max_rpos)
	{
		auto [x, y] = COMBOXY_REGION_INDEX(rpos);

		f = SP_GOFLAG(spotdir);
		if((grid[(int)rpos] & f) == f)
			return;
		else grid[(int)rpos] |= f;
		f = SP_FLAG(spotdir);
		if((grid[(int)rpos] & f) != f)
		{
			grid[(int)rpos] |= f;
			istrig[(int)rpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		switch(spotdir)
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

		if (!(x >= 0 && x < combos_wide && y >= 0 && y < combos_tall)) return;

		rpos = COMBOPOS_REGION_INDEX(x, y);

		switch (typeMap[(int)rpos])
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				return;
		}

		if (rpos == beam_hero_rpos && block && spotdir == oppositeDir[Hero.getDir()])
			return;

		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[(int)rpos] & f) != f)
		{
			grid[(int)rpos] |= f;
			istrig[(int)rpos] |= trigflag;
			doAge = false;
			age = 0;
		}
		if(doAge)
		{
			if(++age > BEAM_AGE_LIMIT)
				return;
		}
		else doAge = true;
		
		if (rpos == beam_hero_rpos && refl)
			spotdir = Hero.getDir();
		else switch (typeMap[(int)rpos])
		{
			case cLIGHTTARGET:
			{
				int cid = beamGrabComboFromPos(rpos, cLIGHTTARGET);
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
					handleBeam(grid, age, d, rpos, set, block, refl);
				}
				return;
			case cMAGICPRISM4:
				for(byte d = 0; d < 4; ++d)
				{
					handleBeam(grid, age, d, rpos, set, block, refl);
				}
				return;
			case cMIRRORNEW:
			{
				auto cid = customTypeMap[(int)rpos];
				if(unsigned(cid) >= MAXCOMBOS) break;
				newcombo const& cmb = combobuf[cid];
				byte newdir = cmb.attribytes[spotdir];
				if(newdir > 7) return;
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
		f = SP_FLAG(spotdir);
		if((grid[curxy.ffpos()] & f) != f)
		{
			grid[curxy.ffpos()] |= f;
			istrig[(int)curxy.pos()] |= trigflag;
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
		switch(typeMap[(int)curpos])
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				return;
			case cMIRRORNEW:
			{
				auto cid = customTypeMap[(int)curpos];
				if(unsigned(cid) >= MAXCOMBOS) break;
				newcombo const& cmb = combobuf[cid];
				byte newdir = cmb.attribytes[spotdir];
				if(newdir > 7) return;
				break;
			}
		}
		bool collided_hero = beam_hero_rpos != rpos_t::None && curxy.herocollide(beamwid);
		if(block && (spotdir == oppositeDir[Hero.getDir()]) && collided_hero)
			return;
		
		f = SP_FLAG(oppositeDir[spotdir]);
		if((grid[curxy.ffpos()] & f) != f)
		{
			grid[curxy.ffpos()] |= f;
			istrig[(int)curpos] |= trigflag;
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
		else switch(typeMap[(int)curpos])
		{
			case cLIGHTTARGET:
			{
				int cid = beamGrabComboFromPos(curpos, cLIGHTTARGET);
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
				auto cid = customTypeMap[(int)curpos];
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

#define BEAMID_COLOR0 (1<<24) //must have rightmost 24 bits empty
static int get_beamid(newcombo const& cmb)
{
	//Positive ID is a tile, negative is a color trio. 0 is nil in either case.
	if(cmb.usrflags&cflag1) //use tile/cset style
		return std::max(0,cmb.attributes[0]/10000)|(cmb.attribytes[1]%12)<<24;
	else //use 3-color style
	{
		auto id = -((cmb.attribytes[3]<<16)|(cmb.attribytes[2]<<8)|(cmb.attribytes[1]));
		if(!id) //0 would clash with the tile/cset style
			id = BEAMID_COLOR0;
		return id;
	}
}
static bool launch_lightbeam(const rpos_handle_t& rpos_handle, std::map<int32_t, spot_t*>& maps, bool refl, bool block)
{
	auto& cmb = rpos_handle.combo();
	int32_t id = get_beamid(cmb);
	//Get the grid array for this tile/color
	spot_t* grid;
	if(maps[id])
		grid = maps[id];
	else
	{
		maps[id] = grid = new spot_t[region_num_rpos];
		memset(grid, 0, sizeof(spot_t)*region_num_rpos);
	}
	byte spotdir = cmb.attribytes[0];
	if(spotdir > 3)
	{
		grid[(int)rpos_handle.rpos] |= SP_VISITED;
		istrig[(int)rpos_handle.rpos] |= cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
	}
	if(refl && rpos_handle.rpos == beam_hero_rpos)
	{
		spotdir = Hero.getDir();
	}
	handleBeam(grid, 0, spotdir, rpos_handle.rpos, cmb.attribytes[4], block, refl);
	return true;
}

static bool launch_fflightbeam(ffcdata const& ffc,
	std::map<int32_t, std::map<dword,spot_t>>& ffmaps, bool refl, bool block)
{
	newcombo const& cmb = combobuf[ffc.data];
	int32_t id = get_beamid(cmb);
	//Get the grid array for this tile/color
	std::map<dword,spot_t>& grid = ffmaps[id]; // grid of (x<<16)|(y&0xFFFF)
	byte spotdir = cmb.attribytes[0];
	lightbeam_xy sxy(ffc.x+(ffc.hit_width/2), ffc.y+(ffc.hit_height/2));
	if(spotdir > 3 && sxy.valid())
	{
		grid[sxy.ffpos()] |= SP_VISITED;
		int32_t trigflag = cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
		istrig[(int)sxy.pos()] |= trigflag;
	}
	auto beamwid = cmb.attribytes[5] < 1 ? 8 : cmb.attribytes[5];
	if(refl && beam_hero_rpos != rpos_t::None && sxy.herocollide(beamwid))
	{
		spotdir = Hero.getDir();
	}
	switch(spotdir)
	{
		case up:
			sxy.y = zc_min(world_h-1,sxy.y);
			break;
		case down:
			sxy.y = zc_max(0,sxy.y);
			break;
		case left:
			sxy.x = zc_min(world_w-1,sxy.x);
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
	if(id < 0 || id == BEAMID_COLOR0)
	{
		int cid = (id == BEAMID_COLOR0) ? 0 : abs(id);
		byte c_inner = (cid & 0x0000FF);
		byte c_middle = (cid & 0x00FF00)>>8;
		byte c_outter = (cid & 0xFF0000)>>16;
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
	static bool had_spotlight = true;
	if (cpos_exists_spotlight())
	{
		istrig.clear();
		customTypeMap.clear();
		typeMap.clear();

		typeMap.resize(region_num_rpos);
		customTypeMap.resize(region_num_rpos);
		istrig.resize(region_num_rpos);

		//Store each different tile/color as grids
		std::map<int32_t, spot_t*> maps;
		std::map<int32_t, std::map<dword, spot_t>> ffmaps;
		int32_t shieldid = getCurrentShield(false);
		if((itemsbuf[shieldid].flags & item_flag9) && !usingActiveShield(shieldid))
			shieldid = -1;
		bool refl = shieldid > -1 && (itemsbuf[shieldid].misc2 & sh_lightbeam);
		bool block = !refl && shieldid > -1 && (itemsbuf[shieldid].misc1 & sh_lightbeam);
		beam_hero_rpos = COMBOPOS_REGION_B(x.getInt()+8, y.getInt()+8);

		clear_bitmap(lightbeam_bmp);

		bool found_any_light = false;

		for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
			bool pos_has_seen_cmb[176] = {0};

			for(int32_t lyr = 6; lyr >= 0; --lyr)
			{
				mapscr* layer_scr = get_scr_layer(scr->screen, lyr);
				for(size_t pos = 0; pos < 176; ++pos)
				{
					if (pos_has_seen_cmb[pos])
						continue;

					rpos_t rpos = POS_TO_RPOS(pos, region_scr_x, region_scr_y);
					auto& cmb = combobuf[layer_scr->data[pos]];
					bool found_combo = false;
					switch(cmb.type)
					{
						case cMIRROR: case cMIRRORSLASH: case cMIRRORBACKSLASH:
						case cMAGICPRISM: case cMAGICPRISM4:
							if(!cmb.attributes[0] || (cmb.attributes[0] & sh_lightbeam))
							{
								typeMap[(int)rpos] = cmb.type;
								found_combo = true;
							}
							break;
						case cBLOCKALL: case cLIGHTTARGET:
							typeMap[(int)rpos] = cmb.type;
							found_combo = true;
							break;
						case cMIRRORNEW:
							if(!cmb.attributes[0] || (cmb.attributes[0] & sh_lightbeam))
							{
								typeMap[(int)rpos] = cMIRRORNEW;
								customTypeMap[(int)rpos] = layer_scr->data[pos];
								found_combo = true;
							}
							break;
						case cGLASS:
							found_combo = true;
							// Already been initialized to zero.
							break;
						case cSPOTLIGHT:
							found_any_light = true;
							break;
					}
					if(found_combo)
						pos_has_seen_cmb[pos] = true;
					else if(lyr < 3 && (cmb.walk & 0xF))
						typeMap[(int)rpos] = SPTYPE_SOLID;
				}
			}

			for (int pos = 0; pos < 176; pos++)
			{
				rpos_t rpos = POS_TO_RPOS(pos, region_scr_x, region_scr_y);
				if (!get_qr(qr_SPOTLIGHT_IGNR_SOLIDOBJ) && !typeMap[(int)rpos])
				{
					auto [x, y] = COMBOXY_REGION(rpos);
					if (collide_object(x, y, 16, 16, this))
						typeMap[(int)rpos] = SPTYPE_SOLID;
				}
			}
		});

		// The world is dark and full of terrors.
		if (!found_any_light) return;
		
		switch (typeMap[(int)beam_hero_rpos])
		{
			case SPTYPE_SOLID: case cBLOCKALL:
				beam_hero_rpos = rpos_t::None; //Blocked from hitting player
		}

		for_every_rpos([&](const rpos_handle_t& rpos_handle) {
			if (rpos_handle.ctype() == cSPOTLIGHT)
				launch_lightbeam(rpos_handle, maps, refl, block);
		});

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			ffcdata& ffc = *ffc_handle.ffc;
			if (ffc.flags & (ffc_changer|ffc_ethereal))
				return;

			newcombo const& cmb = combobuf[ffc.data];
			if (ffc_handle.ctype() == cSPOTLIGHT && (cmb.usrflags&cflag2))
				launch_fflightbeam(ffc,ffmaps,refl,block);
		});
		
		lightbeam_present = !maps.empty() || !ffmaps.empty();
		
		//Draw visuals
		for(auto it = maps.begin(); it != maps.end();)
		{
			int32_t id = it->first;
			spot_t* grid = it->second;
			BITMAP* cbmp = generate_beam_bitmap(id);
			for (rpos_t rpos = (rpos_t)0; rpos < region_max_rpos; rpos = (rpos_t)((int)rpos + 1))
			{
				int32_t offs = get_beamoffs(grid[(int)rpos]);
				if(offs > -1)
				{
					auto [x, y] = COMBOXY_REGION(rpos);
					masked_blit(cbmp, lightbeam_bmp, offs*16, 0, x-viewport.x, y-viewport.y, 16, 16);
				}
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
	}
	else
	{
		if(had_spotlight)
		{
			istrig.clear();
			istrig.resize(region_num_rpos);
			clear_bitmap(lightbeam_bmp);
		}
		lightbeam_present = false;
	}

	had_spotlight = lightbeam_present;

	//Check triggers
	auto& combo_cache = combo_caches::spotlight;
	std::set<int> screens_triggered;
	bool istrigged = true;
	for_every_rpos([&](const rpos_handle_t& rpos_handle) {
		auto& mini_cmb = combo_cache.minis[rpos_handle.data()];
		if (mini_cmb.target)
		{
			auto& cmb = rpos_handle.combo();
			bool alltrig = getmapflag(rpos_handle.scr, mLIGHTBEAM);
			int32_t trigflag = cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
			screens_triggered.insert(rpos_handle.screen);
			bool trigged = (istrig[(int)rpos_handle.rpos]&trigflag);
			if(cmb.usrflags&cflag2) //Invert
				trigged = !trigged;
			if(cmb.usrflags&cflag1) //Solved Version
			{
				if(!(alltrig || trigged)) //Revert
				{
					rpos_handle.decrement_data();
					istrigged = false;
				}
			}
			else //Unsolved version
			{
				if(alltrig || trigged) //Light
					rpos_handle.increment_data();
				else istrigged = false;
			}
		}
		else if (mini_cmb.trigger)
		{
			trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
				int32_t trigflag = trig.triglbeam ? (1 << (trig.triglbeam-1)) : ~0;
				bool trigged = (istrig[(int)rpos_handle.rpos]&trigflag);
				return trig.trigger_flags.get(trigged ? TRIGFLAG_LIGHTON : TRIGFLAG_LIGHTOFF);
			});
		}
	});

	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		ffcdata& ffc = *ffc_handle.ffc;
		rpos_t rpos = get_qr(qr_BROKEN_LIGHTBEAM_HITBOX)
			? COMBOPOS_REGION_B(ffc.x+8, ffc.y+8)
			: COMBOPOS_REGION_B(ffc.x+(ffc.hit_width/2), ffc.y+(ffc.hit_height/2));
		if (rpos == rpos_t::None)
			return;

		auto& mini_cmb = combo_cache.minis[ffc_handle.data()];
		if (mini_cmb.target)
		{
			auto& cmb = ffc_handle.combo();
			bool alltrig = getmapflag(ffc_handle.scr, mLIGHTBEAM);
			int32_t trigflag = cmb.attribytes[4] ? (1 << (cmb.attribytes[4]-1)) : ~0;
			screens_triggered.insert(ffc_handle.screen);
			bool trigged = (istrig[(int)rpos]&trigflag);
			if(cmb.usrflags&cflag2) //Invert
				trigged = !trigged;
			if(cmb.usrflags&cflag1) //Solved Version
			{
				if(!(alltrig || trigged)) //Revert
				{
					zc_ffc_modify(ffc, -1);
					istrigged = false;
				}
			}
			else //Unsolved version
			{
				if(alltrig || trigged) //Light
					zc_ffc_modify(ffc, 1);
				else istrigged = false;
			}
		}
		else //if (mini_cmb.trigger)
		{
			trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
				int32_t trigflag = trig.triglbeam ? (1 << (trig.triglbeam-1)) : ~0;
				bool trigged = (istrig[(int)rpos]&trigflag);
				return trig.trigger_flags.get(trigged ? TRIGFLAG_LIGHTON : TRIGFLAG_LIGHTOFF);
			});
		}
	});

	// Trigger secrets for every screen that has a light trigger.
	if (istrigged && !screens_triggered.empty())
	{
		for (int screen : screens_triggered)
		{
			mapscr* scr = get_scr(screen);
			bool alltrig = getmapflag(scr, mLIGHTBEAM);
			if (alltrig)
				continue;

			trigger_secrets_for_screen(TriggerSource::LightTrigger, screen, false);
			sfx(scr->secretsfx);
			if(!(scr->flags5&fTEMPSECRETS))
			{
				setmapflag(scr, mSECRET);
				setmapflag(scr, mLIGHTBEAM);
			}
		}
	}
}

void HeroClass::checktouchblk()
{
	if (walk_through_walls) return;

	if (getAction() == hopping && !isSideViewHero())
		return;

	if (!pushing)
		return;

	int32_t tdir = dir; //Bad hack #2. _L_, your welcome to fix this properly. ;)

	if (charging > 0 || spins > 0) //if not I probably will at some point...
	{
		if (getInput(btnUp, INPUT_HERO_ACTION) && getInput(btnLeft, INPUT_HERO_ACTION)) tdir = (charging % 2) * 2;
		else if (getInput(btnUp, INPUT_HERO_ACTION) && getInput(btnRight, INPUT_HERO_ACTION)) tdir = (charging % 2) * 3;
		else if (getInput(btnDown, INPUT_HERO_ACTION) && getInput(btnLeft, INPUT_HERO_ACTION)) tdir = 1 + (charging % 2) * 1;
		else if (getInput(btnDown, INPUT_HERO_ACTION) && getInput(btnRight, INPUT_HERO_ACTION)) tdir = 1 + (charging % 2) * 2;
		else
		{
			if (getInput(btnUp, INPUT_HERO_ACTION)) tdir = 0;
			else if (getInput(btnDown, INPUT_HERO_ACTION)) tdir = 1;
			else if (getInput(btnLeft, INPUT_HERO_ACTION)) tdir = 2;
			else if (getInput(btnRight, INPUT_HERO_ACTION)) tdir = 3;
		}
	}

	int xs[2], ys[2];
	int pos_count = 0;
	bool fixed_big_hitbox = bigHitbox && !get_qr(qr_BROKEN_ARMOS_GRAVE_BIGHITBOX_COLLISION);
	switch (tdir)
	{
		case up:
			xs[0] = x;
			ys[0] = y + (fixed_big_hitbox ? -1 : (bigHitbox ? 0 : 7));
			xs[1] = x + 8;
			ys[1] = ys[0];
			pos_count = 2;
			break;

		case down:
			xs[0] = x;
			ys[0] = y + 16;
			xs[1] = x + 8;
			ys[1] = ys[0];
			pos_count = 2;
			break;

		case left:
			xs[0] = x - 1;
			ys[0] = y + 15;
			pos_count = 1;
			if (fixed_big_hitbox)
			{
				xs[1] = xs[0];
				ys[1] = y + 7;
				pos_count = 2;
			}
			break;

		case right:
			xs[0] = x + 16;
			ys[0] = y + 15;
			pos_count = 1;
			if (fixed_big_hitbox)
			{
				xs[1] = xs[0];
				ys[1] = y + 7;
				pos_count = 2;
			}
			break;
	}
	int max_layer = get_qr(qr_ARMOS_GRAVE_ON_LAYERS) ? 6 : 0;
	int start_layer = zc_max(2, max_layer);
	bool pos_done[2] = { false,false };
	for (int layer = start_layer; layer >= 0; --layer)
	{
		for (int idx = 0; idx < pos_count; ++idx)
		{
			if (pos_done[idx]) continue;
			auto tx = xs[idx];
			auto ty = ys[idx];
			auto rpos_handle = get_rpos_handle_for_world_xy(tx, ty, layer);
			newcombo const& cmb = rpos_handle.combo();
			if (cmb.type == cBRIDGE)
			{
				if (get_qr(qr_OLD_BRIDGE_COMBOS)
					? !_walkflag_layer(x, y, layer - 1)
					: _effectflag_layer(x, y, layer - 1))
				{
					pos_done[idx] = true; // cover lower layers at this position
					continue;
				}
			}
			if (layer > max_layer) continue; // only checking this layer for bridges
			if (!_effectflag(x, y, 1, layer - 1)) continue;
			if (cmb.only_gentrig) continue;
			switch (cmb.type)
			{
				case cBSGRAVE: case cGRAVE:
					if (cmb.flag || rpos_handle.sflag())
						continue;
					break;
				case cARMOS:
					break;
				default:
					continue;
			}
			trigger_armos_grave(rpos_handle, dir);
			break; // only one per layer, even if multiple poses match
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
		auto [map, screen] = nextscr2(cdir);
		if (map == -1)
			return 0;
        
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
		
		int32_t cmb = COMBOPOS(cx%256, cy%176);
		return get_canonical_scr(map, screen)->data[cmb];
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
    if(cx<0 || cy<0 || cx>=world_w || cy>=world_h)
    {
		auto [map, screen] = nextscr2(cdir);
		if (map == -1)
			return 0;
        
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
		
        int32_t pos = COMBOPOS(cx%256, cy%176);
        const mapscr* scr = get_canonical_scr(map, screen);
        if (!comboflag)
        {
            return scr->sflag[pos];
        }
        else
        {
            return combo_caches::flag.minis[scr->data[pos]].flag;
        }
    }
    
    if(comboflag)
    {
        return MAPCOMBOFLAG(cx,cy);
    }
    
    return MAPFLAG(cx,cy);
}

void HeroClass::checkspecial()
{
    checktouchblk();

	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		int screen = scr->screen;
		auto& state = get_screen_state(screen);
		bool hasmainguy = hasMainGuy(screen);
		if (!state.loaded_enemies || hasmainguy)
		{
			state.did_enemy_secret = false;
		}
		else
		{
			// Enemies have been defeated.

			// generic 'Enemies->' trigger
			for_every_combo_in_screen(create_screen_handles(scr), [&](const auto& handle) {
				trig_each_combo_trigger(handle, [&](combo_trigger const& trig){
					return trig.trigger_flags.get(TRIGFLAG_ENEMIESKILLED);
				});
			});

			if (scr->flags9 & fENEMY_WAVES)
			{
				hasmainguy = hasMainGuy(screen); //possibly un-beat the enemies (another 'wave'?)
			}
			if(!hasmainguy)
			{
				// item
				if (state.item_state == ScreenItemState::MustGiveToEnemy || state.item_state == ScreenItemState::CarriedByEnemy || state.item_state == ScreenItemState::WhenKillEnemies)
				{
					int32_t Item=scr->item;

					if((!getmapflag(screen, mITEM) || (scr->flags9&fITEMRETURN)) && (scr->hasitem != 0))
					{
						if (state.item_state == ScreenItemState::WhenKillEnemies)
							sfx(WAV_CLEARED);

						zfix x = region_scr_x*256 + scr->itemx;
						zfix y = region_scr_y*176 + ((scr->flags7&fITEMFALLS && isSideViewHero()) ? -170 : scr->itemy+1);
						add_item_for_screen(screen, new item(x, y, (scr->flags7&fITEMFALLS && !isSideViewHero()) ? (zfix)170 : (zfix)0,
										Item,ipONETIME|ipBIGRANGE|((itemsbuf[Item].type==itype_triforcepiece ||
												(scr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
					}

					state.item_state = ScreenItemState::None;
				}
				if(!getmapflag(scr, mTMPNORET))
				{
					// if room has traps, guys don't come back
					for (int32_t i=0; i<eMAXGUYS; i++)
					{
						if (guysbuf[i].type==eeTRAP && guysbuf[i].attributes[1]
							&& guys.idCount(i, screen))
						{
							setmapflag(scr, mTMPNORET);
							break;
						}
					}
				}
				// clear enemies and open secret
				if (!state.did_enemy_secret && (scr->flags2&fCLEARSECRET))
				{
					bool only16_31 = get_qr(qr_ENEMIES_SECRET_ONLY_16_31)?true:false;
					trigger_secrets_for_screen(TriggerSource::EnemiesScreenFlag, scr, only16_31);
					
					if (scr->flags4&fENEMYSCRTPERM && canPermSecret(cur_dmap, screen))
					{
						if (!(scr->flags5&fTEMPSECRETS)) setmapflag(scr, mSECRET);
					}
					
					sfx(scr->secretsfx);
					state.did_enemy_secret = true;
				}
				if(scr->flags9 & fENEMIES_STAY_DEAD)
				{
					if(!getmapflag(scr, mNO_ENEMIES_RETURN))
						setmapflag(scr, mNO_ENEMIES_RETURN);
				}
			}
		}

		// doors
		bool has_shutter = false;

		for (int i = 0; i < 4; i++)
		{
			if (scr->door[i]==dSHUTTER)
			{
				has_shutter = true;
				if (state.open_doors == 0 && state.loaded_enemies)
				{
					if (!(scr->flags&fSHUTTERS) && !hasmainguy)
						state.open_doors=12;
				}
				else if (state.open_doors < 0)
					++state.open_doors;
				else if (--state.open_doors == 0)
					openshutters(scr);
					
				break;
			}
		}

		if (!has_shutter && !state.open_doors && state.loaded_enemies && !(scr->flags&fSHUTTERS) && !hasmainguy)
		{
			openshutters(scr);
		}

    	// set boss flag when boss is gone
		if (state.loaded_enemies && scr->flags11&efBOSS && !hasmainguy)
		{
			game->lvlitems[dlevel]|=(1 << li_boss_killed);
			stop_sfx(scr->bosssfx);
		}

		auto screen_handles = create_screen_handles(scr);

		if (getmapflag(screen, mCHEST))              // if special stuff done before
		{
			remove_chests(screen_handles);
		}
		
		if(getmapflag(screen, mLOCKEDCHEST))              // if special stuff done before
		{
			remove_lockedchests(screen_handles);
		}
		
		if(getmapflag(screen, mBOSSCHEST))              // if special stuff done before
		{
			remove_bosschests(screen_handles);
		}

		clear_xdoors(screen_handles, true);
		clear_xstatecombos(screen_handles, true);

		if (state.triggered_secrets && state.item_state == ScreenItemState::WhenTriggerSecrets)
		{
			int32_t Item=scr->item;
			
			if((!getmapflag(screen, mITEM) || (scr->flags9&fITEMRETURN)) && (scr->hasitem != 0))
			{
				auto [x, y] = translate_screen_coordinates_to_world(screen, scr->itemx, (scr->flags7&fITEMFALLS && isSideViewHero()) ? -170 : scr->itemy+1);
				add_item_for_screen(screen, new item(x, y,
								(scr->flags7&fITEMFALLS && !isSideViewHero()) ? (zfix)170 : (zfix)0,
								Item,ipONETIME|ipBIGRANGE|((itemsbuf[Item].type==itype_triforcepiece ||
										(scr->flags3&fHOLDITEM)) ? ipHOLDUP : 0) | ((scr->flags8&fITEMSECRET) ? ipSECRETS : 0),0));
			}

			state.item_state = ScreenItemState::None;
		}
	});
}

// Returns 4 rpos indicated by all combinations of the coordinates, replacing duplicates with rpos_t::None
static std::array<rpos_t, 4> getRposes(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	std::array<rpos_t, 4> rposes;
	rpos_t tmp;

	rposes[0] = COMBOPOS_REGION_B(x1,y1);
	
	tmp = COMBOPOS_REGION_B(x1,y2);
	if (tmp == rposes[0])
		rposes[1] = rpos_t::None;
	else rposes[1] = tmp;
	
	tmp = COMBOPOS_REGION_B(x2,y1);
	if (tmp == rposes[0] || tmp == rposes[1])
		rposes[2] = rpos_t::None;
	else rposes[2] = tmp;
	
	tmp = COMBOPOS_REGION_B(x2,y2);
	if (tmp == rposes[0] || tmp == rposes[1] || tmp == rposes[2])
		rposes[3] = rpos_t::None;
	else rposes[3] = tmp;

	return rposes;
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
	
	if(walk_through_walls) return;
	
	bool didstrig = false;
	
	for(int32_t i=bigHitbox?0:8; i<16; i+=bigHitbox?15:7)
	{
		for(int32_t j=0; j<16; j+=15) for(int32_t k=0; k<2; k++)
		{
			int comboid = 0;
			mapscr* scr;
			if (k > 0)
			{
				auto ffc_handle = getFFCAt(x+j, y+i);
				if (ffc_handle)
				{
					comboid = ffc_handle->data();
					scr = ffc_handle->scr;
				}
			}
			else
			{
				auto rpos_handle = get_rpos_handle_for_world_xy(x+j, y+i, 0);
				comboid = rpos_handle.data();
				scr = rpos_handle.scr;
			}
			newcombo const& cmb = combobuf[comboid];
			int32_t stype = cmb.type;
			int32_t warpsound = cmb.attribytes[0];
			if(cmb.only_gentrig)
				stype = cNONE;
			if(stype==cSWARPA)
			{
				if(scr->flags5&fDIRECTSWARP)
				{
					setpit();
				}
				
				sdir=dir;
				dowarp(scr,0,0,warpsound);
				return;
			}
			
			if(stype==cSWARPB)
			{
				if(scr->flags5&fDIRECTSWARP)
				{
					setpit();
				}
				
				sdir=dir;
				dowarp(scr,0,1,warpsound);
				return;
			}
			
			if(stype==cSWARPC)
			{
				if(scr->flags5&fDIRECTSWARP)
				{
					setpit();
				}
				
				sdir=dir;
				dowarp(scr,0,2,warpsound);
				return;
			}
			
			if(stype==cSWARPD)
			{
				if(scr->flags5&fDIRECTSWARP)
				{
					setpit();
				}
				
				sdir=dir;
				dowarp(scr,0,3,warpsound);
				return;
			}
			
			if(stype==cSWARPR)
			{
				if(scr->flags5&fDIRECTSWARP)
				{
					setpit();
				}
				
				sdir=dir;
				dowarp(scr,0,(zc_oldrand()%4),warpsound);
				return;
			}
			
			rpos_t rpos = COMBOPOS_REGION(x+j, y+i);
			if((stype==cSTRIGNOFLAG || stype==cSTRIGFLAG) && stepsecret!=rpos)
			{
				auto rpos_handle = get_rpos_handle(rpos, 0);
				
				if(stype==cSTRIGFLAG && canPermSecret(cur_dmap, rpos_handle.screen))
				{ 
					if(!didstrig)
					{
						stepsecret = rpos;
						
						if(!(rpos_handle.scr->flags5&fTEMPSECRETS))
						{
							setmapflag(rpos_handle.scr, mSECRET);
						}
						sfx(warpsound,pan((int32_t)x));
						trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.base_scr(), false);
						didstrig = true;
					}
				}
				else
				{ 
					if(!didstrig)
					{
						stepsecret = rpos;

						bool high16only = get_qr(qr_STEPTEMP_SECRET_ONLY_16_31)?true:false;
						trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.base_scr(), high16only);
						didstrig = true;
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
		if(TRUNCATE_HALF_TILE(int32_t(y))==warpy)
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
	mapscr* flag_scr=nullptr;
	mapscr* flag2_scr=nullptr;
	mapscr* flag3_scr=nullptr;
	mapscr* scrs[4]={};
	rpos_handle_t rpos_handles[4];
	int32_t type=0;
	int32_t water=0;
	int32_t index = 0;
	
	bool setsave=false;
	int32_t warpsfx2 = 0;
	if (RaftPass) goto RaftingStuff;

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

	rpos_handles[0] = get_rpos_handle_for_world_xy(x1, y1, 0);
	rpos_handles[1] = get_rpos_handle_for_world_xy(x1, y2, 0);
	rpos_handles[2] = get_rpos_handle_for_world_xy(x2, y1, 0);
	rpos_handles[3] = get_rpos_handle_for_world_xy(x2, y2, 0);

	types[0] = rpos_handles[0].sflag();
	types[1] = rpos_handles[1].sflag();
	types[2] = rpos_handles[2].sflag();
	types[3] = rpos_handles[3].sflag();

	flag_scr = rpos_handles[0].scr;

	//MAPFFCOMBO
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag = types[0];
	// 2.10 compatibility...
	else if(y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag = types[0];
		
		
	types[0] = rpos_handles[0].cflag();
	types[1] = rpos_handles[1].cflag();
	types[2] = rpos_handles[2].cflag();
	types[3] = rpos_handles[3].cflag();
	flag2_scr = rpos_handles[0].scr;
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag2 = types[0];
	else if(!get_qr(qr_FAIRY_FLAG_COMPAT) && y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag2 = types[0];

	{
		auto ffc_handle_1 = getFFCAt(x1, y1);
		auto ffc_handle_2 = getFFCAt(x1, y2);
		auto ffc_handle_3 = getFFCAt(x2, y1);
		auto ffc_handle_4 = getFFCAt(x2, y2);

		types[0] = ffc_handle_1 ? ffc_handle_1->cflag() : 0;
		types[1] = ffc_handle_2 ? ffc_handle_2->cflag() : 0;
		types[2] = ffc_handle_3 ? ffc_handle_3->cflag() : 0;
		types[3] = ffc_handle_4 ? ffc_handle_4->cflag() : 0;

		if (ffc_handle_1)
			flag3_scr = ffc_handle_1->scr;
		else if (ffc_handle_2)
			flag3_scr = ffc_handle_2->scr;
		else if (ffc_handle_3)
			flag3_scr = ffc_handle_3->scr;
		else if (ffc_handle_4)
			flag3_scr = ffc_handle_4->scr;
	}

	//
	
	if(types[0]==types[1]&&types[2]==types[3]&&types[1]==types[2])
		flag3 = types[0];
	else if(!get_qr(qr_FAIRY_FLAG_COMPAT) && y.getInt()%16==8 && types[0]==types[2] && (types[0]==mfFAIRY || types[0]==mfMAGICFAIRY || types[0]==mfALLFAIRY))
		flag3 = types[0];
		
	//
	// Now, let's check for warp combos...
	//
	
	//
	
	cids[0] = rpos_handles[0].data();
	cids[1] = rpos_handles[1].data();
	cids[2] = rpos_handles[2].data();
	cids[3] = rpos_handles[3].data();
	
	types[0] = rpos_handles[0].ctype();
	if (auto ffc_handle = getFFCAt(x1, y1))
	{
		types[0] = ffc_handle->ctype();
		cids[0] = ffc_handle->data();
	}
	
	types[1] = rpos_handles[1].ctype();
	if (auto ffc_handle = getFFCAt(x1, y2))
	{
		types[1] = ffc_handle->ctype();
		cids[1] = ffc_handle->data();
	}

	types[2] = rpos_handles[2].ctype();
	if (auto ffc_handle = getFFCAt(x2, y1))
	{
		types[2] = ffc_handle->ctype();
		cids[2] = ffc_handle->data();
	}

	types[3] = rpos_handles[3].ctype();
	if (auto ffc_handle = getFFCAt(x2, y2))
	{
		types[3] = ffc_handle->ctype();
		cids[3] = ffc_handle->data();
	}

	// Change B, C and D warps into A, for the comparison below...
	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].only_gentrig)
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
	if(action!=freeze&&action!=sideswimfreeze&&(!msg_active || !get_qr(qr_MSGFREEZE)))
	{
		auto rposes = diagonalMovement||NO_GRIDLOCK ?
			getRposes(tx+4, ty+4, tx+11, ty+11) :
			getRposes(tx+4, ty+4, tx+11, ty+11);
		auto sensRposes = getRposes(tx, ty+(bigHitbox?0:8), tx+15, ty+15);
		int32_t xPoses[4] = {tx + 4, tx + 11, tx, tx + 15};
		int32_t yPoses[4] = {ty + 4, ty + 11, ty+(bigHitbox?0:8), ty + 15};
		
		bool hasStep[4] = {false};
		for(auto p = 0; p < 4; ++p)
		{
			if (rposes[p] == rpos_t::None) continue;

			for (auto lyr = 0; lyr < 7; ++lyr)
			{
				auto rpos_handle = get_rpos_handle(rposes[p], lyr);
				if ((z > 0 || fakez > 0) && !(rpos_handle.base_scr()->flags2 & fAIRCOMBOS))
					continue;
				auto& cmb = rpos_handle.combo();
				for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
				{
					auto& trig = cmb.triggers[idx];
					if (trig.trigger_flags.any({TRIGFLAG_STEP,TRIGFLAG_STEPSENS})
						|| types[p] == cSTEP)
					{
						hasStep[p] = true;
						break;
					}
				}
				if(hasStep[p])
					break;
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
		for (auto p = 0; p < 4; ++p)
		{
			for (auto lyr = 0; lyr < 7; ++lyr)
			{
				if (rposes[p] != rpos_t::None)
				{
					auto rpos_handle = get_rpos_handle(rposes[p], lyr);
					bool did_trig = canNormalStep && trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
						return trig.trigger_flags.get(TRIGFLAG_STEP);
					});
					if (did_trig && rposes[p] == sensRposes[p]) continue;
				}
				if (sensRposes[p] != rpos_t::None)
				{
					auto rpos_handle = get_rpos_handle(sensRposes[p], lyr);
					trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
						return trig.trigger_flags.get(TRIGFLAG_STEPSENS);
					});
				}
			}
		}

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
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
				trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
					return trig.trigger_flags.any({TRIGFLAG_STEP,TRIGFLAG_STEPSENS});
				});
			}
		});
	}

	if(isDiving()) //Dive-> triggerflag
	{
		rpos_t rpos = COMBOPOS_REGION(x+8,y+8);
		int x1=x,x2=x+15,y1=y+(bigHitbox?0:8),y2=y+15;
		int xposes[] = {x1,x1,x2,x2};
		int yposes[] = {y1,y2,y1,y2};
		auto rposes = getRposes(x1,y1,x2,y2);
		for(auto lyr = 0; lyr < 7; ++lyr)
		{
			bool didtrig = false;
			auto rpos_handle = get_rpos_handle(rpos, lyr);
			auto& cmb = rpos_handle.combo();
			auto cid = rpos_handle.data();
			didtrig = trig_each_combo_trigger(rpos_handle, [&](combo_trigger const& trig){
				return trig.trigger_flags.get(TRIGFLAG_DIVETRIG);
			});
			for(auto q = 0; q < 4; ++q)
			{
				if(rposes[q] == rpos_t::None) continue;
				if(rposes[q] == rpos && didtrig) continue;

				auto rpos_handle_2 = get_rpos_handle(rposes[q], lyr);
				trig_each_combo_trigger(rpos_handle_2, [&](combo_trigger const& trig){
					return trig.trigger_flags.get(TRIGFLAG_DIVESENSTRIG);
				});
			}
		}

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
			if(ffcIsAt(ffc_handle, x+8, y+8))
			{
				if(trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
					return trig.trigger_flags.get(TRIGFLAG_DIVETRIG);
				})) return;
			}
			for(auto q = 0; q < 4; ++q)
			{
				if(ffcIsAt(ffc_handle, xposes[q], yposes[q]))
				{
					if(trig_each_combo_trigger(ffc_handle, [&](combo_trigger const& trig){
						return trig.trigger_flags.get(TRIGFLAG_DIVESENSTRIG);
					})) break;
				}
			}
		});
	}
	
	//
	// Now, let's check for Save combos...
	//
	x1 = tx+4;
	x2 = tx+11;
	y1 = ty+4;
	y2 = ty+11;

	rpos_handles[0] = get_rpos_handle_for_world_xy(x1, y1, 0);
	rpos_handles[1] = get_rpos_handle_for_world_xy(x1, y2, 0);
	rpos_handles[2] = get_rpos_handle_for_world_xy(x2, y1, 0);
	rpos_handles[3] = get_rpos_handle_for_world_xy(x2, y2, 0);

	cids[0] = rpos_handles[0].data();
	cids[1] = rpos_handles[1].data();
	cids[2] = rpos_handles[2].data();
	cids[3] = rpos_handles[3].data();

	types[0] = rpos_handles[0].ctype();
	if (auto ffc_handle = getFFCAt(x1, y1))
	{
		types[0] = ffc_handle->ctype();
		cids[0] = ffc_handle->data();
	}
	
	types[1] = rpos_handles[1].ctype();
	if (auto ffc_handle = getFFCAt(x1, y2))
	{
		types[1] = ffc_handle->ctype();
		cids[1] = ffc_handle->data();
	}

	types[2] = rpos_handles[2].ctype();
	if (auto ffc_handle = getFFCAt(x2, y1))
	{
		types[2] = ffc_handle->ctype();
		cids[2] = ffc_handle->data();
	}

	types[3] = rpos_handles[3].ctype();
	if (auto ffc_handle = getFFCAt(x2, y2))
	{
		types[3] = ffc_handle->ctype();
		cids[3] = ffc_handle->data();
	}
	
	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].only_gentrig)
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
		water = onWater(false);
		if (water && diveclk == 1)
		{
			auto old_stand_z = standing_on_z;
			standing_on_z = 0;
			if (!onWater(false)) // player hits head on the solid above them, stay underwater longer
				diveclk = 2;
			standing_on_z = old_stand_z;
		}
	}
	
	// Pits (aka direct warps) have a bigger 'hitbox' than stairs...
	x1 = tx+7;
	x2 = tx+8;
	y1 = ty+7+(bigHitbox?0:4);
	y2 = ty+8+(bigHitbox?0:4);
	
	rpos_handles[0] = get_rpos_handle_for_world_xy(x1, y1, 0);
	rpos_handles[1] = get_rpos_handle_for_world_xy(x1, y2, 0);
	rpos_handles[2] = get_rpos_handle_for_world_xy(x2, y1, 0);
	rpos_handles[3] = get_rpos_handle_for_world_xy(x2, y2, 0);

	cids[0] = rpos_handles[0].data();
	cids[1] = rpos_handles[1].data();
	cids[2] = rpos_handles[2].data();
	cids[3] = rpos_handles[3].data();
	
	types[0] = rpos_handles[0].ctype();
	scrs[0] = rpos_handles[0].scr;
	if (auto ffc_handle = getFFCAt(x1, y1))
	{
		types[0] = ffc_handle->ctype();
		cids[0] = ffc_handle->data();
		scrs[0] = ffc_handle->scr;
	}
	
	types[1] = rpos_handles[1].ctype();
	scrs[1] = rpos_handles[1].scr;
	if (auto ffc_handle = getFFCAt(x1, y2))
	{
		types[1] = ffc_handle->ctype();
		cids[1] = ffc_handle->data();
		scrs[1] = ffc_handle->scr;
	}

	types[2] = rpos_handles[2].ctype();
	scrs[2] = rpos_handles[2].scr;
	if (auto ffc_handle = getFFCAt(x2, y1))
	{
		types[2] = ffc_handle->ctype();
		cids[2] = ffc_handle->data();
		scrs[2] = ffc_handle->scr;
	}

	types[3] = rpos_handles[3].ctype();
	scrs[3] = rpos_handles[3].scr;
	if (auto ffc_handle = getFFCAt(x2, y2))
	{
		types[3] = ffc_handle->ctype();
		cids[3] = ffc_handle->data();
		scrs[3] = ffc_handle->scr;
	}

	for(int32_t i=0; i<4; i++)
	{
		if(combobuf[cids[i]].only_gentrig)
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
			type = cPIT;
			
	//
	// Time to act on our results for type, flag, flag2 and flag3...
	//
	if(type==cSAVE&&cur_screen<128)
		*ls=1;
		
	if(type==cSAVE2&&cur_screen<128)
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
	
	if((z>0 || fakez>0) && !(hero_scr->flags2&fAIRCOMBOS))
		return;
		
	if((type==cTRIGNOFLAG || type==cTRIGFLAG))
	{ 
		if (COMBOPOS_REGION(tx+8, ty+8)!=stepsecret || get_qr(qr_TRIGGERSREPEAT))
		{
			stepsecret = COMBOPOS_REGION(tx+8, ty+8);
			auto rpos_handle = get_rpos_handle(stepsecret, 0);
			sfx(combobuf[MAPCOMBO(rpos_handle)].attribytes[0],pan((int32_t)x));
			
			if(type==cTRIGFLAG && canPermSecret(cur_dmap, rpos_handle.screen))
			{ 
				if(!(rpos_handle.scr->flags5&fTEMPSECRETS)) setmapflag(rpos_handle.scr, mSECRET);
				
				trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.base_scr(), false);
			}
			else 
			{
				bool only16_31 = get_qr(qr_STEPTEMP_SECRET_ONLY_16_31)?true:false;
				trigger_secrets_for_screen(TriggerSource::Unspecified, rpos_handle.base_scr(), only16_31);
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
	if(water > 0 && !ladderx && hoverclk==0 && action!=rafting && !inlikelike && !DRIEDLAKE
		&& ((get_qr(qr_DROWN) && z==0 && fakez==0 && fall>=0 && fakefall>=0) || CanSideSwim())
		&& (sideview_mode() || !platform_ffc))
	{
		if(!is_autowalking() && (current_item(itype_flippers) <= 0 || current_item(itype_flippers) < combobuf[water].attribytes[0] || ((combobuf[water].usrflags&cflag1) && !(itemsbuf[current_item_id(itype_flippers)].flags & item_flag3))))
		{
			if(!(ladderx+laddery)) drownCombo = water;
			if (combobuf[water].usrflags&cflag1) Drown(1);
			else Drown();
			if(byte drown_sfx = combobuf[water].attribytes[4])
				sfx(drown_sfx, pan(x));
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
		rpos_t next_step = COMBOPOS_REGION(tx+8, ty+8);
		if (next_step != stepnext)
		{
			stepnext = next_step;
			auto rpos_handle = get_rpos_handle(stepnext, 0);
			int cid = MAPCOMBO(rpos_handle);
			
			if
		(
			COMBOTYPE(tx+8,ty+8)==cSTEP && /*required item*/
			(!combobuf[cid].attribytes[1] || (combobuf[cid].attribytes[1] && game->item[combobuf[cid].attribytes[1]]) )
			&& /*HEAVY*/
			( ( !(combobuf[cid].usrflags&cflag1) ) || ((combobuf[cid].usrflags&cflag1) && Hero.HasHeavyBoots() ) )
		)
		{
			sfx(combobuf[cid].attribytes[0],pan((int32_t)x));
			rpos_handle.increment_data();
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
				for_every_rpos([&](const rpos_handle_t& rpos_handle) {
					if (rpos_handle.data() == cid)
						rpos_handle.increment_data();
				});
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

				for_every_rpos([&](const rpos_handle_t& rpos_handle) {
					if (isStepType(rpos_handle.ctype()))
						rpos_handle.increment_data();
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
							if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
							else sfx(get_scr_for_world_xy(tx,ty+11)->secretsfx);
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
									if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
									else sfx(get_scr_for_world_xy(tx+8,ty+11)->secretsfx);
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
			if(isDiving() && (!getmapflag(flag_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (flag_scr->flags9&fBELOWRETURN)))
			{
				additem(x, y, flag_scr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((flag_scr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(flag_scr->secretsfx);
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
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
						else sfx(get_scr_for_world_xy(tx,ty)->secretsfx);
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
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
								else sfx(get_scr_for_world_xy(tx+8,ty+8)->secretsfx);
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
			if(isDiving() && (!getmapflag(flag2_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (flag2_scr->flags9&fBELOWRETURN)))
			{
				additem(x, y, flag2_scr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((flag2_scr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(flag2_scr->secretsfx);
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
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
						else sfx(get_scr_for_world_xy(tx,ty)->secretsfx);
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
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
								else sfx(get_scr_for_world_xy(tx+8,ty+8)->secretsfx);
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
			if(isDiving() && (!getmapflag(flag3_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM) || (flag3_scr->flags9&fBELOWRETURN)))
			{
				additem(x, y, flag3_scr->catchall,
						ipONETIME2 | ipBIGRANGE | ipHOLDUP | ipNODRAW | ((flag3_scr->flags8&fITEMSECRET) ? ipSECRETS : 0));
				sfx(flag3_scr->secretsfx);
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
						if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
						else sfx(get_scr_for_world_xy(tx,ty)->secretsfx);
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
								if (get_qr(qr_RAFT_SOUND)) sfx(itemsbuf[current_item_id(itype_raft)].usesound,pan(x));
								else sfx(get_scr_for_world_xy(tx+8,ty+8)->secretsfx);
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
	
	// Either the screen the hero is currently in, or if in a 0x80 room the screen player came from.
	mapscr* base_scr = cur_screen >= 128 ? special_warp_return_scr : hero_scr;
	
	if((type==cCAVE || type==cCAVE2) && (base_scr->tilewarptype[index]==wtNOWARP)) return;
	
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
				if ((DMaps[cur_dmap].flags & dmfCAVES) && base_scr->tilewarptype[index] == wtCAVE)
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
				else if (DMaps[hero_scr->tilewarpdmap[index]].midi != (currmidi - ZC_MIDI_COUNT + 4) &&
					get_canonical_scr(DMaps[tdm].map, base_scr->tilewarpscr[index] + DMaps[tdm].xoff)->screen_midi != (currmidi - ZC_MIDI_COUNT + 4))
					music_stop();
			}
		}
		
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
		bool opening = (base_scr->tilewarptype[index]<=wtPASS && !(DMaps[cur_dmap].flags&dmfCAVES && base_scr->tilewarptype[index]==wtCAVE)
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
		setpit();
		warp_sound = warpsfx2;
	}

	mapscr* scr = nullptr;
	for (int i = 0; i < 4 && !scr; i++) scr = scrs[i];
	if (!scr) scr = hero_scr;

	if (!is_in_scrolling_region()
			&& DMaps[cur_dmap].flags&dmf3STAIR
			&& (cur_screen==129 || !(DMaps[cur_dmap].flags&dmfGUYCAVES))
			&& (specialcave > 0 && DMaps[cur_dmap].flags & dmfGUYCAVES ? special_warp_return_scr : scr)->room==rWARP && type==cSTAIR)
	{
		if(!skippedaframe)
		{
			FFCore.warpScriptCheck();
			draw_screen();
			advanceframe(true);
		}
		
		// "take any road you want"
		int32_t dw = x<112 ? 1 : (x>136 ? 3 : 2);
		int32_t code = WARPCODE(cur_dmap,home_screen,dw);
		
		if(code>-1)
		{
			bool changedlevel = false;
			bool changeddmap = false;
			if(cur_dmap != code>>8)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
				changeddmap = true;
			}
			if(dlevel != DMaps[code>>8].level)
			{
				timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
				changedlevel = true;
			}
			cur_dmap = code>>8;
			dlevel = DMaps[cur_dmap].level;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			
			cur_map = DMaps[cur_dmap].map;
			home_screen = (code&0xFF) + DMaps[cur_dmap].xoff;
			init_dmap();
			
			if(canPermSecret(cur_dmap, cur_screen))
				setmapflag_homescr(mSECRET);
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
		
		for(auto flag : {mSECRET,mITEM,mSPECIALITEM,mNEVERRET,mCHEST,mLOCKEDCHEST,mBOSSCHEST,
			mLOCKBLOCK,mBOSSLOCKBLOCK,mTMPNORET,mVISITED,mLIGHTBEAM,mNO_ENEMIES_RETURN})
		{
			if(!(scr->noreset&flag))
				unsetmapflag(scr, flag);
		}
		
		if(isdungeon())
		{
			for(auto flag : {mDOOR_LEFT,mDOOR_RIGHT,mDOOR_DOWN,mDOOR_UP})
			{
				if(!(scr->noreset&flag))
					unsetmapflag(scr, flag);
			}
		}
		
		if(scr->exstate_reset)
		{
			for(uint q = 0; q < 32; ++q)
				if(scr->exstate_reset&(1<<q))
					unsetxmapflag(scr->screen, (1<<q));
		}
		
		setpit();
		sdir=dir;
		dowarp(scr, 4, 0, warpsfx2);
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
		dowarp(scr, 0, index, warpsfx2);
	}
}

int32_t selectWlevel(int32_t d)
{
    if(TriforceCount()==0)
        return 0;
        
    word l = game->get_wlevel();
    
    do
    {
        if(d==0 && (game->lvlitems[l+1] & (1 << li_mcguffin)))
            break;
        else if(d<0)
            l = (l==0) ? 7 : l-1;
        else
            l = (l==7) ? 0 : l+1;
    }
    while(!(game->lvlitems[l+1] & (1 << li_mcguffin)));
    
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
    if(hero_scr->room==rGANON)
        stop_sfx(WAV_ROAR);
}

bool HeroClass::HasHeavyBoots()
{
	for ( int32_t q = 0; q < MAXITEMS; ++q )
	{
		if ( game->item[q] && ( itemsbuf[q].type == itype_boots ) && /*iron*/ (itemsbuf[q].flags&item_flag2) ) return true;
	}
	return false;
}

bool HeroClass::dowarp(const mapscr* scr, int32_t type, int32_t index, int32_t warpsfx)
{
	if (!scr) scr = hero_scr;

	byte reposition_sword_postwarp = 0;
	if (index < 0)
	{
		return false;
	}
	finish_auto_walk();
	is_warping = true;
	for (int32_t q = 0; q < Lwpns.Count(); ++q)
	{
		weapon* swd = NULL;
		swd = (weapon*)Lwpns.spr(q);
		if (swd->id == (attack == wSword ? wSword : wWand))
		{
			Lwpns.del(q);
		}
	}

	attackclk = charging = spins = tapping = 0;
	attack = none;
	if (warp_sound > 0) warpsfx = warp_sound;
	warp_sound = 0;
	word wdmap = 0;
	byte wscr = 0, wtype = 0, t = 0;
	bool overlay = false;
	t = (cur_screen < 128) ? 0 : 1;
	int32_t wrindex = 0;
	bool wasSideview = isSideViewGravity(t);

	// Either the current screen, or if in a 0x80 room the screen player came from.
	const mapscr* base_scr = cur_screen >= 128 ? special_warp_return_scr : scr;

	// Drawing commands probably shouldn't carry over...
	if (!get_qr(qr_SCRIPTDRAWSINWARPS))
		script_drawing_commands.Clear();

	if (replay_version_check(35))
	{
		// TODO: Would be better to remove a bit later, after fading out the current screen. For now, do this here.
		// https://github.com/ZQuestClassic/ZQuestClassic/pull/1017#discussion_r1828880117
		reset_ladder();
	}
	switch(type)
	{
		case 0:                                                 // tile warp
			wtype = base_scr->tilewarptype[index];
			wdmap = base_scr->tilewarpdmap[index];
			wscr = base_scr->tilewarpscr[index];
			overlay = get_bit(&base_scr->tilewarpoverlayflags,index)?1:0;
			wrindex=(scr->warpreturnc>>(index*2))&3;
			break;
			
		case 1:                                                 // side warp
			wtype = base_scr->sidewarptype[index];
			wdmap = base_scr->sidewarpdmap[index];
			wscr = base_scr->sidewarpscr[index];
			overlay = get_bit(&base_scr->sidewarpoverlayflags,index)?1:0;
			wrindex=(scr->warpreturnc>>(8+(index*2)))&3;
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
			wdmap = cur_dmap;
			wscr = home_screen-DMaps[cur_dmap].xoff;
			break;
	}
	
	bool intradmap = (wdmap == cur_dmap);
	int32_t olddmap = cur_dmap;
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

		if(DMaps[cur_dmap].flags&dmfCAVES)                                         // cave
		{
			if (updatemusic || !musicnocut || !get_qr(qr_SCREEN80_OWN_MUSIC))
				music_stop();
			kill_sfx();

			int destscr = 0x80;
			if(scr->room==rWARP)
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
			scr = hero_scr;
			//preloaded freeform combos
			ffscript_engine(true);
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
					int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
			kill_sfx();
			draw_screen(false);
			
			//unless the room is already dark, fade to black
			if (!get_qr(qr_NEW_DARKROOM))
			{
				if(!darkroom)
				{
					darkroom = true;
					fade(DMaps[cur_dmap].color,true,false);
				}
			}
			else
				fade(DMaps[cur_dmap].color, true, false);
			
			blackscr(30,true);

			bool no_x80_dir = true; // TODO: is this necessary?
			loadscr(wdmap, 0x80, down, false, no_x80_dir);
			scr = hero_scr;
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
					int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
		// some draw_screen code (the passive subscreen compass dot) depends
		// on cur_screen being set before initiating the screen wipes for a warp.
		// Without this the compass dot would remain drawn while warping.
		// This is better, but for now this code keeps the rendering equivalent to before
		// z3 refactor.
		// demosp253.zplay and first_quest_layered.zplay showcases this behavior.
		// TODO(replays): remove in future bulk replay update.
		currscr_for_passive_subscr = 0x81;

		kill_sfx();
		ALLOFF();
		//play sound
		if(warpsfx > 0) sfx(warpsfx,pan(x));
		specialcave = PASSAGEWAY;
		byte warp_screen_2 = wscr + DMaps[wdmap].xoff;
		draw_screen(false);
		
		if(!get_qr(qr_NEW_DARKROOM))
		{
			if(!darkroom)
				fade(DMaps[cur_dmap].color,true,false);
				
			darkroom=true;
		}
		else
			fade(DMaps[cur_dmap].color,true,false);
		blackscr(30,true);
		bool no_x80_dir = true;
		loadscr(wdmap, 0x81, down, false, no_x80_dir);
		scr = hero_scr;
		//preloaded freeform combos
		ffscript_engine(true);
		if ( dontdraw < 2 ) { dontdraw=1; }
		draw_screen(false);
		lighting(false, true);
		if (get_qr(qr_NEW_DARKROOM))
			fade(0xB, false, true);
		dir=down;
		x=48;
		
		if((home_screen&15) > (warp_screen_2&15))
		{
			x=192;
		}
		
		if((home_screen&15) == (warp_screen_2&15))
		{
			if((cur_screen>>4) > (warp_screen_2>>4))
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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
		stepoutscreen = warp_screen_2;
		stepoutdmap = wdmap;
		stepoutwr=wrindex;
		if (get_qr(qr_SCREEN80_OWN_MUSIC) && (updatemusic || !musicnocut))
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
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
		if(cur_dmap != wdmap)
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
		cur_dmap = wdmap;
		if(changeddmap)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
		}
		if(changedlevel)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
		}
		
		cur_map=DMaps[cur_dmap].map;
		init_dmap();
		update_subscreens(wdmap);
		loadfullpal();
		ringcolor(false);
		loadlvlpal(DMaps[cur_dmap].color);
		int destscr = wscr + DMaps[cur_dmap].xoff;
		loadscr(cur_dmap, destscr, -1, overlay);
		scr = hero_scr;

		if (is_dark(scr) && !get_qr(qr_NEW_DARKROOM))
		{
			if(get_qr(qr_FADE))
			{
				interpolatedfade();
			}
			else
			{
				loadfadepal((DMaps[cur_dmap].color)*pdLEVEL+poFADE3);
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
			wrx=scr->warpreturnx[0];
			wry=scr->warpreturny[0];
		}
		else
		{
			wrx=scr->warparrivalx;
			wry=scr->warparrivaly;
		}
		
		if(((wrx>0||wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))&&(!(scr->flags6&fNOCONTINUEHERE)))
		{
			if(dlevel)
			{
				lastentrance = cur_screen;
			}
			else
			{
				lastentrance = DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff;
			}
			
			lastentrance_dmap = wdmap;
		}
		
		if(dlevel)
		{
			if(get_qr(qr_NOARRIVALPOINT))
			{
				x=scr->warpreturnx[wrindex];
				y=scr->warpreturny[wrindex];
			}
			else
			{
				x=scr->warparrivalx;
				y=scr->warparrivaly;
			}
		}
		else
		{
			x=scr->warpreturnx[wrindex];
			y=scr->warpreturny[wrindex];
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
		update_viewport();
		
		if(dlevel)
		{
			// reset enemy kill counts
			for(int32_t i=0; i<128; i++)
			{
				int mi = mapind(cur_map, i);
				game->guys[mi] = 0;
				game->maps[mi] &= ~mTMPNORET;
			}
		}
		
		markBmap(dir^1, current_screen);
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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
				walkup(COOLSCROLL);
			}
			else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
			{
				reset_pal_cycling();
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
		currcset=DMaps[cur_dmap].color;
		dointro();
		set_respawn_point();
		trySideviewLadder();
		
		for(int32_t i=0; i<6; i++)
			visited[i]=-1;
			
		break;
	}
	
	case wtSCROLL:                                          // scrolling warp
	{
		int32_t c = DMaps[cur_dmap].color;
		scrolling_dmap = cur_dmap;
		scrolling_map = cur_map;
		cur_map = DMaps[wdmap].map;
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
		scr = hero_scr;

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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
				positionSword(swd,item_id);
				break;
				}
			}
		}
		if(!intradmap)
		{
			init_dmap();
			
			int32_t wrx,wry;
			
			if(get_qr(qr_NOARRIVALPOINT))
			{
				wrx=hero_scr->warpreturnx[0];
				wry=hero_scr->warpreturny[0];
			}
			else
			{
				wrx=hero_scr->warparrivalx;
				wry=hero_scr->warparrivaly;
			}
			
			if(((wrx>0||wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))&&(!get_qr(qr_NOSCROLLCONTINUE))&&(!(scr->flags6&fNOCONTINUEHERE)))
			{
				if(dlevel)
				{
					lastentrance = cur_screen;
				}
				else
				{
					lastentrance = DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff;
				}
				
				lastentrance_dmap = wdmap;
			}
		}
		if(DMaps[cur_dmap].color != c)
		{
			lighting(false, true);
		}
		
		if (updatemusic)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[cur_dmap].color;
		dointro();
	}
	break;
	
	case wtWHISTLE:                                         // whistle warp
	{
		scrolling_dmap = cur_dmap;
		scrolling_map = cur_map;
		cur_map = DMaps[wdmap].map;
		scrollscr(index, wscr+DMaps[wdmap].xoff, wdmap);
		scr = hero_scr;
		reset_hookshot();
		cur_dmap=wdmap;
		dlevel=DMaps[cur_dmap].level;
		lighting(false, true);
		init_dmap();
		
		if (updatemusic)
		{
			playLevelMusic();
			if (musicrevert)
				FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
		}
		currcset=DMaps[cur_dmap].color;
		dointro();
		action=inwind; FFCore.setHeroAction(inwind);
		int32_t wry;
		
		if(get_qr(qr_NOARRIVALPOINT))
			wry=hero_scr->warpreturny[0];
		else wry=hero_scr->warparrivaly;
		
		int32_t wrx;
		
		if(get_qr(qr_NOARRIVALPOINT))
			wrx=hero_scr->warpreturnx[0];
		else wrx=hero_scr->warparrivalx;

		wrx += region_scr_dx * 256;
		wry += region_scr_dy * 176;

		calculate_viewport(viewport, cur_dmap, cur_screen, world_w, world_h, wrx + Hero.txsz*16/2, wry + Hero.tysz*16/2);

		zfix whistle_x = index==left?viewport.right()-16:index==right?viewport.left():wrx;
		zfix whistle_y = index==down?viewport.top():index==up?viewport.bottom()-16:wry;

		auto wind = new weapon(whistle_x,whistle_y,
			(zfix)0,wWind,1,0,index,whistleitem,getUID(),false,false,true,1);
		wind->screen_spawned = hero_scr->screen;
		Lwpns.add(wind);
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
		
		bool kill_action = !(scr->flags3&fIWARP_SPRITE_CARRYOVER);
		if(kill_action)
		{
			//ALLOFF kills the action, but we want to preserve Hero's action if he's swimming or diving -DD
			bool wasswimming = (action == swimming);
			int32_t olddiveclk = diveclk;
			ALLOFF();
			
			if(wasswimming)
			{
				Hero.SetSwim();
				set_dive(olddiveclk);
			}
			
			kill_sfx();
		}
		//play sound
		if(warpsfx > 0) sfx(warpsfx,pan(x));
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
		
		int32_t c = DMaps[cur_dmap].color;
		bool changedlevel = false;
		bool changeddmap = false;
		if(cur_dmap != wdmap)
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
		cur_dmap = wdmap;
		if(changeddmap)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
		}
		if(changedlevel)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
		}

		cur_map = DMaps[cur_dmap].map;
		init_dmap();
		update_subscreens(wdmap);
		
		ringcolor(false);
		
		if(DMaps[cur_dmap].color != c)
			loadlvlpal(DMaps[cur_dmap].color);
		
		auto prev_region = cur_region;
		int prev_origin_screen = cur_screen;
		loadscr(cur_dmap, wscr + DMaps[cur_dmap].xoff, -1, overlay);
		scr = hero_scr;
		lightingInstant(); // Also sets naturaldark

		// In the case where we did not call ALLOFF, preserve the "enemies have spawned"
		// state for the new screen.
		if (!kill_action)
		{
			// Mark the "loaded_enemies" state true if the previous screen had it as true.
			// The new screen is attempted to be matched 1:1 with screens in the old region, but if
			// the regions are different sizes then the nearest screen is used (it's clamped).
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				int previous_screen_x = zc_min(prev_region.screen_width, region_scr_x);
				int previous_screen_y = zc_min(prev_region.screen_height, region_scr_y);
				int previous_screen = prev_origin_screen + previous_screen_x + previous_screen_y*16;
				if (get_screen_state(previous_screen).loaded_enemies)
					get_screen_state(scr->screen).loaded_enemies = true;
			});

			// Update "screen_spawned" for every sprite to keep it valid.
			for_every_sprite([&](sprite& spr) {
				int x = get_region_relative_dx(spr.screen_spawned, prev_origin_screen);
				int y = get_region_relative_dy(spr.screen_spawned, prev_origin_screen);
				x = zc_min(x, cur_region.screen_width);
				y = zc_min(y, cur_region.screen_height);
				spr.screen_spawned = cur_screen + x + y*16;
			});
		}

		x = hero_scr->warpreturnx[wrindex];
		y = hero_scr->warpreturny[wrindex];
		
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
		update_viewport();
		
		markBmap(dir^1, current_screen);
		
		int32_t checkwater = iswaterex_z3(MAPCOMBO(x,y+8), -1, x,y+(bigHitbox?8:12)); //iswaterex can be intensive, so let's avoid as many calls as we can.
		
		if(checkwater && _walkflag(x,y+(bigHitbox?8:12),0,get_standing_z_state()) && current_item(itype_flippers) > 0 && current_item(itype_flippers) >= combobuf[checkwater].attribytes[0] && (!(combobuf[checkwater].usrflags&cflag1) || (itemsbuf[current_item_id(itype_flippers)].flags & item_flag3)))
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
		
		putscr(hero_scr, scrollbuf, 0, 0);
		putscrdoors(hero_scr, scrollbuf, 0, 0);
		
		if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
		{
			reset_pal_cycling();
			putscr(hero_scr, scrollbuf, 0, 0);
			putscrdoors(hero_scr, scrollbuf, 0, 0);
			walkup(COOLSCROLL);
		}
		else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
		{
			reset_pal_cycling();
			putscr(hero_scr, scrollbuf, 0, 0);
			putscrdoors(hero_scr, scrollbuf, 0, 0);
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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
		currcset=DMaps[cur_dmap].color;
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
					 
			if(!(scr->flags3&fIWARP_SPRITE_CARRYOVER))
			{
				//ALLOFF kills the action, but we want to preserve Hero's action if he's swimming or diving -DD
				bool wasswimming = (action == swimming);
				int32_t olddiveclk = diveclk;
				ALLOFF();
				
				if(wasswimming)
				{
					Hero.SetSwim();
					set_dive(olddiveclk);
				}
				
				kill_sfx();
			}
			//play sound
			if(warpsfx > 0) sfx(warpsfx,pan(x));	
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
			
			int32_t c = DMaps[cur_dmap].color;
			bool changedlevel = false;
			bool changeddmap = false;
			if(cur_dmap != wdmap)
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
			cur_dmap = wdmap;
			if(changeddmap)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
			}
			if(changedlevel)
			{
				throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
			}
			cur_map = DMaps[cur_dmap].map;
			init_dmap();
			update_subscreens(wdmap);
			
			ringcolor(false);
			
			if(DMaps[cur_dmap].color != c)
				loadlvlpal(DMaps[cur_dmap].color);
			
			loadscr(cur_dmap, wscr + DMaps[cur_dmap].xoff, -1, overlay);
			scr = hero_scr;
			lightingInstant(); // Also sets naturaldark
			
			x = hero_scr->warpreturnx[wrindex];
			y = hero_scr->warpreturny[wrindex];
			
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
			update_viewport();
			
			markBmap(dir^1, current_screen);
			
			if(iswaterex_z3(MAPCOMBO(x,y+8), -1, x,y+8) && _walkflag(x,y+8,0,get_standing_z_state()) && current_item(itype_flippers))
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
			
			putscr(hero_scr, scrollbuf, 0, 0);
			putscrdoors(hero_scr, scrollbuf, 0, 0);

			if((type1==cCAVE)||(type1>=cCAVEB && type1<=cCAVED) || (type2==cCAVE)||(type2>=cCAVEB && type2<=cCAVED))
			{
				reset_pal_cycling();
				putscr(hero_scr, scrollbuf, 0, 0);
				putscrdoors(hero_scr, scrollbuf, 0, 0);
				walkup(COOLSCROLL);
			}
			else if((type3==cCAVE2)||(type3>=cCAVE2B && type3<=cCAVE2D) || (type2==cCAVE2)||(type2>=cCAVE2B && type2<=cCAVE2D))
			{
				reset_pal_cycling();
				putscr(hero_scr, scrollbuf, 0, 0);
				putscrdoors(hero_scr, scrollbuf, 0, 0);
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
						int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
						positionSword(swd,item_id);
						break;
					}
				}
			}
			show_subscreen_life=true;
			show_subscreen_numbers=true;
			playLevelMusic();
			currcset=DMaps[cur_dmap].color;
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
						int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
	if(action!=rafting && checkwater && (_walkflag(x,y+(bigHitbox?8:12),0,get_standing_z_state()) || get_qr(qr_DROWN))
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
			
			if(((enemy*)guys.spr(i))->type!=eeTRAP && ((enemy*)guys.spr(i))->type!=eeSPINTILE)
				guys.spr(i)->yofs += 2;
		}
	}
	else if(wasSideview && !tmpscr_is_sideview)
	{
		for(int32_t i=0; i<guys.Count(); i++)
		{
			if(((enemy*)guys.spr(i))->type!=eeTRAP && ((enemy*)guys.spr(i))->type!=eeSPINTILE)
				guys.spr(i)->yofs -= 2;
		}
	}
	
	if((DMaps[cur_dmap].type&dmfCONTINUE) || (cur_dmap==0&&get_qr(qr_DMAP_0_CONTINUE_BUG)))
	{
		if(dlevel)
		{
			int32_t wrx,wry;
			
			if(get_qr(qr_NOARRIVALPOINT))
			{
				wrx=origin_scr->warpreturnx[0];
				wry=origin_scr->warpreturny[0];
			}
			else
			{
				wrx=origin_scr->warparrivalx;
				wry=origin_scr->warparrivaly;
			}
			
			if((wtype == wtEXIT)
					|| (((wtype == wtSCROLL) && !intradmap) && ((wrx>0 || wry>0)||(get_qr(qr_WARPSIGNOREARRIVALPOINT)))))
			{
				if(!(wtype==wtSCROLL)||!(get_qr(qr_NOSCROLLCONTINUE)))
				{
					game->set_continue_scrn(home_screen);
				}
				else if(cur_dmap != game->get_continue_dmap())
				{
					game->set_continue_scrn(DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff);
				}
			}
			else
			{
				if(cur_dmap != game->get_continue_dmap())
				{
					game->set_continue_scrn(DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff);
				}
			}
		}
		else
		{
			game->set_continue_scrn(DMaps[cur_dmap].cont + DMaps[cur_dmap].xoff);
		}
		
		game->set_continue_dmap(cur_dmap);
		lastentrance_dmap = cur_dmap;
		lastentrance = game->get_continue_scrn();
	}
	
	if(origin_scr->flags4&fAUTOSAVE)
	{
		save_game(true,0);
	}
	
	if(origin_scr->flags6&fCONTINUEHERE)
	{
		lastentrance_dmap = cur_dmap;
		lastentrance = home_screen;
	}
	
	update_subscreens();
	verifyBothWeapons();

	static const char roomtype_names[rMAX][255] =
	{
		"(None)","Special Item","Pay for Info","Secret Money","Gamble",
		"Door Repair","Red Potion or Heart Container","Feed the Goriya","Level 9 Entrance",
		"Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
		"3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash",
		"More Arrows","Take One Item","Bottle Shop"
	};

	if(wtype==wtCAVE)
	{
		if(DMaps[cur_dmap].flags&dmfGUYCAVES)
			Z_eventlog("Entered %s containing %s.\n",DMaps[cur_dmap].flags&dmfCAVES ? "Cave" : "Item Cellar",
					   (char *)roomtype_names[special_warp_return_scr->room]);
		else
			Z_eventlog("Entered %s.",DMaps[cur_dmap].flags&dmfCAVES ? "Cave" : "Item Cellar");
	}
	else Z_eventlog("Warped to DMap %d: %s, screen %d, via %s.\n", cur_dmap, DMaps[cur_dmap].name,cur_screen,
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
				int32_t item_id = (directWpn>-1 && itemsbuf[directWpn].type==itype) ? directWpn : current_item_id(itype);
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
	}
	is_warping = false;
	if(!get_qr(qr_SCROLLWARP_NO_RESET_FRAME))
		GameFlags |= GAMEFLAG_RESET_GAME_LOOP;
	return true;
}

void HeroClass::exitcave()
{
	bool updatemusic = FFCore.can_dmap_change_music(cur_dmap);
	bool musicnocut = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_NOCUT;

    stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
    loadscr(cur_dmap, home_screen, 255, false);                                   // bogus direction
    x = hero_scr->warpreturnx[0];
    y = hero_scr->warpreturny[0];
    
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
	update_viewport();
        
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
    loadlvlpal(DMaps[cur_dmap].color);
    lighting(false, true);
	if (updatemusic || !musicnocut)
		music_stop();
    kill_sfx();
    putscr(hero_scr, scrollbuf, 0, 0);
    putscrdoors(hero_scr, scrollbuf, 0, 0);
    
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
    currcset=DMaps[cur_dmap].color;
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
        update_viewport();
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

void HeroClass::setpit()
{
	didpit = true;
	pitx = x.getInt() % 256;
	pity = y.getInt() % 176;
}

void HeroClass::walkdown(bool opening) //entering cave
{
    if(opening)
    {
        close_black_opening(x+8, y+8+playing_field_offset, false);
    }
    
    hclk=0;
    stop_item_sfx(itype_brang);
    sfx(WAV_STAIRS,pan(x));
    clk=0;
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

    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==3)
		{
            ++y;
			update_viewport();
		}

        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }

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
    sfx(WAV_STAIRS,pan(x));
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
    
    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==3)
        {
            ++y;
            update_viewport();
        }
            
        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }
    
	
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
    sfx(WAV_STAIRS,pan(x));
    dir=down;
    clk=0;
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

    for(int32_t i=0; i<64; i++)
    {
        herostep();
        
        if(zinit.heroAnimationStyle==las_zelda3 || zinit.heroAnimationStyle==las_zelda3slow)
            hero_count=(hero_count+1)%16;
            
        if((i&3)==0)
        {
            --y;
            update_viewport();
        }
            
        draw_screen();
        advanceframe(true);
        
        if(Quit)
            break;
    }

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
    sfx(WAV_STAIRS,pan(x));
    dir=up;
    clk=0;
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
        {
            --y;
            update_viewport();
        }
            
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
	bool updatemusic = FFCore.can_dmap_change_music(cur_dmap);
	bool musicnocut = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_NOCUT;

	int32_t sc = specialcave; // This gets erased by ALLOFF()
    ALLOFF();
    stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
    kill_sfx();
    draw_screen(false);
    fade(sc>=GUYCAVE?10:11,true,false);
    blackscr(30,true);
    ringcolor(false);
    
    if(sc==PASSAGEWAY && abs(x-warpx)>16) // How did Hero leave the passageway?
    {
        cur_dmap=stepoutdmap;
        cur_map=DMaps[cur_dmap].map;
        dlevel=DMaps[cur_dmap].level;
        
        //we might have just left a passage, so be sure to update the CSet record -DD
        currcset=DMaps[cur_dmap].color;
        
        init_dmap();
        home_screen=stepoutscreen;
    }
    
    loadscr(cur_dmap, home_screen, 255, false);                                   // bogus direction
    draw_screen(false);
    
    if(get_qr(qr_NEW_DARKROOM) || !is_dark(hero_scr))
    {
        darkroom = naturaldark = false;
        fade(DMaps[cur_dmap].color,true,true);
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
            loadfadepal((DMaps[cur_dmap].color)*pdLEVEL+poFADE3);
        }
		byte *si = colordata + CSET(DMaps[cur_dmap].color*pdLEVEL+poLEVEL)*3;
		si+=3*48;
			
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(9)+i] = _RGB(si);
			tempgreypal[CSET(9)+i] = _RGB(si); //preserve monochrome
			si+=3;
		}
    }
    
	if(didpit)
	{
		didpit=false;
		x=pitx;
		y=pity;
	}
	else
	{
		x = hero_scr->warpreturnx[stepoutwr];
		y = hero_scr->warpreturny[stepoutwr];
	}

	x += region_scr_dx * 256;
	y += region_scr_dy * 176;
	update_viewport();
    
    if(x+y == 0)
        x = y = 80;
        
    dir=down;
    
    set_respawn_point();
    
    // Let's use the 'exit cave' animation if we entered this cellar via a cave combo.
    int32_t type = combobuf[MAPCOMBO(x,y)].type;
    
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
    markBmap();
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
    if(walk_through_walls || (action!=swimming && !IsSideSwim() && action != swimhit) || hopclk==0) //!DIMITODO: ...does swimming just let you ignore smart scrolling entirely!?
        return false;
        
    // assumes Hero is about to scroll screens
    auto [map, screen] = nextscr2(d2);
    if (map == -1)
        return false;

    const mapscr* scr = get_canonical_scr(map, screen);
    
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
    
    int32_t cmb = COMBOPOS(cx%256, cy%176);    
    const newcombo* c = &combobuf[scr->data[cmb]];
    bool dried = iswater_type(c->type) && DRIEDLAKE;
    bool swim = iswater_type(c->type) && (current_item(itype_flippers)) && !dried;
    int32_t b=1;
    
    if(cx&8) b<<=2;
    
    if(cy&8) b<<=1;
    
    if((c->walk&b) && !dried && !swim)
        return true;
        
    // next block (i.e. cnt==2)
    if(!(cx&8))
    {
        b<<=2;
    }
    else
    {
        c = &combobuf[scr->data[++cmb]];
        dried = iswater_type(c->type) && DRIEDLAKE;
        swim = iswater_type(c->type) && (current_item(itype_flippers)) && !dried;
        b=1;
        
        if(cy&8)
        {
            b<<=1;
        }
    }
    
    return (c->walk&b) ? !dried && !swim : false;
}

bool HeroClass::nextcombo_solid(int32_t d2)
{
	if(walk_through_walls || cur_screen>=128)
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
	
	for(int32_t i=0; i<=((bigHitbox&&!(d2==up||d2==down))?((initcy&7)?2:1):((initcy&7)?1:0)) && cy < 176; cy+=(cy%2)?7:8,i++)
	{
		cx = initcx;
		for(int32_t k=0; k<=(smarter_scroll?((initcx&7)?2:1):0) && cx < 256; cx+=(cx%2)?7:8,k++)
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
	else                   assert(false);

	mapscr* scr = hero_scr;

	if(get_qr(qr_SMARTSCREENSCROLL)&&(!(scr->flags&fMAZE))&&action!=inwind &&action!=scrolling && !(scr->flags2&dir_flag))
	{
		if(nextcombo_solid(dir))
			should_scroll = false;
	}

	if (should_scroll || action == inwind)
	{
		finish_auto_walk();
		if(cur_screen>=128)
		{
			if(specialcave >= GUYCAVE)
				exitcave();
			else stepout();
		}
		else if(action==inwind)
		{
			if(DMaps[cur_dmap].flags&dmfWHIRLWINDRET)
			{
				action=none; FFCore.setHeroAction(none);
				restart_level();
			}
			else
			{
				dowarp(scr, 2, dir);
			}
		}
		else if(scr->flags2&dir_flag && (!(scr->flags8&fMAZEvSIDEWARP) || checkmaze(scr,false)))
		{
			sdir=dir;
			dowarp(scr, 1, (scr->sidewarpindex >> (sdir*2))&3);
		}
		else if(!edge_of_dmap(dir) && edge_of_region(dir))
		{
			scrolling_dmap = cur_dmap;
			scrolling_map = cur_map;
			scrollscr(dir);
			
			if(hero_scr->flags4&fAUTOSAVE)
			{
				save_game(true,0);
			}
			
			if(hero_scr->flags6&fCONTINUEHERE)
			{
				lastentrance_dmap = cur_dmap;
				lastentrance = home_screen;
			}
		}
	}
}

static bool has_advanced_maze(mapscr* scr)
{
	bool loopy = scr->flags10&fMAZE_LOOPY;
	bool can_get_lost = scr->flags10&fMAZE_CAN_GET_LOST;
	int maze_transition_wipe = scr->maze_transition_wipe;
	return loopy || can_get_lost || maze_transition_wipe || is_in_scrolling_region();
}

void HeroClass::maybe_begin_advanced_maze()
{
	if (!(hero_scr->flags&fMAZE) || current_screen == scrolling_maze_last_solved_screen || maze_state.active)
		return;

	// Basic mazes are handled in scrollscr.
	if (!has_advanced_maze(hero_scr))
		return;

	maze_state = {};
	maze_state.active = true;
	maze_state.loopy = hero_scr->flags10&fMAZE_LOOPY;
	maze_state.transition_wipe = hero_scr->maze_transition_wipe;
	maze_state.can_get_lost = hero_scr->flags10&fMAZE_CAN_GET_LOST;
	maze_state.scr = hero_scr;
	maze_state.exit_screen = screen_index_direction(current_screen, (direction)hero_scr->exitdir);
	maze_state.last_check_herox = x;
	maze_state.last_check_heroy = y;

	if (maze_state.loopy)
	{
		int dx = get_region_relative_dx(prev_hero_scr->screen) - get_region_relative_dx(current_screen);
		int dy = get_region_relative_dy(prev_hero_scr->screen) - get_region_relative_dy(current_screen);
		maze_state.enter_dir = XY_DELTA_TO_DIR(sign2(dx), sign2(dy));
	}
}

// Checks if hero is beyond the bounds of the screen, and if so begins scrolling.
// Returns after scrolling is finished.
void HeroClass::checkscroll()
{
	//DO NOT scroll if Hero is vibrating due to Farore's Wind effect -DD
	if(action == casting||action==sideswimcasting)
		return;
	
	if(!check_prescroll()) // prevent scrolling when you should drown/fall/etc before the screen edge
		return;

	if (action == inwind && whirlwind == 0)
	{
		if (x > viewport.right()-16)
		{
			x = viewport.right()-16;
			do_scroll_direction(right);
		}
		else if (x < viewport.left())
		{
			x = viewport.left();
			do_scroll_direction(left);
		}
		else if (y > viewport.bottom()-16)
		{
			y = viewport.bottom()-16;
			do_scroll_direction(down);
		}
		else if (y < viewport.top())
		{
			y = viewport.top();
			do_scroll_direction(up);
		}

		return;
	}

	if (!maze_state.active)
		maybe_begin_advanced_maze();

	if (maze_state.active)
	{
		if (action == inwind)
		{
			maze_state.active = false;
			return;
		}

		if (!maze_state.lost)
		{
			if (determine_hero_screen_from_coords()->screen == maze_state.exit_screen)
			{
				maze_state.active = false;
				update_current_screen();
				return;
			}

			if (maze_state.enter_dir != dir_invalid && determine_hero_screen_from_coords() == prev_hero_scr)
			{
				maze_state.active = false;
				update_current_screen();
				return;
			}
		}

		int maze_screen = maze_state.scr->screen;
		auto [sx, sy] = translate_screen_coordinates_to_world(maze_screen);

		int x0 = x.getInt();
		int y0 = y.getInt();
		direction advance_dir = dir_invalid;
		if (x0 > (sx+256)-16 || x0 > world_w - 16) advance_dir = right;
		if (x0 < sx || x0 < 0)                     advance_dir = left;
		if (y0 > (sy+176)-16 || y0 > world_h - 16) advance_dir = down;
		if (y0 < sy || y0 < 0)                     advance_dir = up;

		if (advance_dir == dir_invalid)
		{
			maze_state.last_check_herox = -10000;
			maze_state.last_check_heroy = -10000;
		}
		else
		{
			bool can_check_again = std::abs(maze_state.last_check_herox - x0) >= 16 || std::abs(maze_state.last_check_heroy - y0) >= 16;

			if (can_check_again && maze_enabled_sizewarp(maze_state.scr, advance_dir))
			{
				maze_state.active = false;
				return;
			}

			bool found_exit = !maze_state.lost && (lastdir[3] == maze_state.scr->exitdir || lastdir[3] == maze_state.enter_dir);
			if (found_exit)
			{
				// Do nothing.
			}
			else if (can_check_again && checkmaze_ignore_exit(maze_state.scr, true))
			{
				maze_state.last_check_herox = x;
				maze_state.last_check_heroy = y;
				maze_state.enter_dir = dir_invalid;

				int dest_screen = screen_index_direction(maze_screen, advance_dir);
				if (is_in_current_region(dest_screen))
					scrolling_maze_last_solved_screen = maze_screen;
				else
					scrollscr(advance_dir, dest_screen);

				maze_state.active = false;
			}
			else if (can_check_again && maze_state.loopy)
			{
				if (maze_state.transition_wipe)
					closescreen(maze_state.transition_wipe - 1);

				if (advance_dir == left)  x += 256;
				if (advance_dir == right) x -= 256;
				if (advance_dir == up)    y += 176;
				if (advance_dir == down)  y -= 176;
				if (advance_dir == left || advance_dir == right)
					x.doClamp(0, world_w - 16);
				if (advance_dir == up || advance_dir == down)
					y.doClamp(0, world_h - 16);

				maze_state.last_check_herox = x;
				maze_state.last_check_heroy = y;
				maze_state.enter_dir = dir_invalid;

				if (maze_state.can_get_lost)
				{
					if (advance_dir == maze_state.scr->exitdir)
						maze_state.lost = false;
					else if (advance_dir != maze_state.enter_dir)
						maze_state.lost = true;
				}

				if (maze_state.transition_wipe)
					openscreen(maze_state.transition_wipe - 1);
			}
			else if (!maze_state.loopy)
			{
				if (maze_state.transition_wipe)
					closescreen(maze_state.transition_wipe - 1);

				loadscr(cur_dmap, current_screen, -1, false);
				maze_state.scr = get_scr(maze_screen);

				// A bit janky, but works: clear all state (as usual during a screen change), but keep
				// the maze state.
				auto prev_maze_state = maze_state;
				ALLOFF();
				maze_state = prev_maze_state;

				if (advance_dir == left)  x = (get_region_relative_dx(maze_screen) + 1) * 256 - 16;
				if (advance_dir == right) x = (get_region_relative_dx(maze_screen)) * 256;
				if (advance_dir == up)    y = (get_region_relative_dy(maze_screen) + 1) * 176 - 16;
				if (advance_dir == down)  y = (get_region_relative_dy(maze_screen)) * 176;

				if (maze_state.can_get_lost)
				{
					if (advance_dir == maze_state.scr->exitdir)
						maze_state.lost = false;
					else
						maze_state.lost = true;
				}

				if (maze_state.transition_wipe)
					openscreen(maze_state.transition_wipe - 1);
			}
		}

		if (maze_state.active && (maze_state.lost || advance_dir != maze_state.scr->exitdir))
			return;
	}

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

bool HeroClass::check_prescroll()
{
	if (walk_through_walls)
		return true;
	if(get_qr(qr_BROKEN_SCROLL_INSTEAD_OF_DROWN_FALL))
		return true; // skip checks
	if (x <= world_w-16 && x >= 0 && y <= world_h-16 && y >= 0)
		return true; // in bounds, no need for checks
	zfix tx = x, ty = y, tz = z;
	x = vbound(x, 0, world_w-16);
	y = vbound(y, 0, world_h-16);
	if(onWater(true) || drownclk)
		return false; // would drown before scrolling
	if(pitslide() || fallclk)
		return false; // would fall before scrolling
	x = tx; y = ty; z = tz;
	return true;
}

// assumes current direction is in lastdir[3]
// compares directions with scr->path and scr->exitdir
bool HeroClass::checkmaze(const mapscr *scr, bool sound)
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

// assumes current direction is in lastdir[3]
// compares directions with scr->path and scr->exitdir
bool HeroClass::checkmaze_ignore_exit(const mapscr *scr, bool sound)
{
    if(!(scr->flags&fMAZE))
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
    if (!has_advanced_maze(origin_scr) && checkmaze(origin_scr, false) == false)
        return false;

    // needs fixin'
    // should check dmap style
    switch(side)
    {
    case up:
        return current_screen<16;
        
    case down:
        return current_screen>=112;
        
    case left:
        if((current_screen&15)==0)
            return true;
            
        if((DMaps[cur_dmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((current_screen&15)-DMaps[cur_dmap].xoff)<=0);
            
        break;
        
    case right:
        if((current_screen&15)==15)
            return true;
            
        if((DMaps[cur_dmap].type&dmfTYPE)!=dmOVERW)
            //    if(dlevel)
            return (((current_screen&15)-DMaps[cur_dmap].xoff)>=7);
            
        break;
    }
    
    return false;
}

static std::pair<int, int> lookahead_coords(int scrolldir, int x, int y)
{
	x = vbound(x, 0, world_w - 16);
	y = vbound(y, 0, world_h - 16);
	return {x, y};
}

// Helper for scrollscr that gets next combo on next screen.
static int32_t lookahead(int32_t scrolldir, int x, int y)
{
	auto [cx, cy] = lookahead_coords(scrolldir, x, y);
	return MAPCOMBO(cx, cy);
}

static bool lookaheadraftflag(int scroll_dir, int x, int y)
{
    int cx = x;
    int cy = y + 8;
	bound(cx, 0, world_w - 16);
	bound(cy, 0, world_h - 8);

	auto rpos_handle = get_rpos_handle_for_world_xy(cx, cy, 0);
    return (isRaftFlag(rpos_handle.cflag()) || isRaftFlag(rpos_handle.sflag()));
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
		if ( (!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.waitdraw(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
			FFCore.waitdraw(ScriptType::Hero) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].script,cur_dmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script,cur_dmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW);

		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (scr->script != 0 && FFCore.waitdraw(ScriptType::Screen, scr->screen) && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);  
					FFCore.waitdraw(ScriptType::Screen, scr->screen) = 0;
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
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (scr->script != 0 && scr->preloadscript)
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);
				}
			});
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_FFCS);
		if ((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.doscript(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_GLOBAL_ACTIVE);
		if ((!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.doscript(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255)
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_ACTIVE);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].script,cur_dmap);
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 ) 
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script,cur_dmap);
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
static region_t scrolling_new_region;

void HeroClass::run_scrolling_script_old(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
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

	// Also, hero coordinates should remain unchanged.
	// For compat, this is happening after the generic scripts above...
	zfix storex = x, storey = y;

	switch(scrolldir)
	{
	case up:
		if(y < scrolling_new_region.height - 16) y = scrolling_new_region.height;
		else if(cx > 0 && !end_frames) y = sy + scrolling_new_region.height - 20;
		else y = scrolling_new_region.height - 16;

		x = new_hero_x;
		break;
		
	case down:
		if(y > 0) y = -16;
		else if(cx > 0 && !end_frames) y = sy - 172;
		else y = 0;

		x = new_hero_x;
		break;
		
	case left:
		if(x < scrolling_new_region.width - 16) x = scrolling_new_region.width;
		else if(cx > 0) x = sx + scrolling_new_region.width - 20;
		else x = scrolling_new_region.width - 16;

		y = new_hero_y;
		break;
		
	case right:
		if(x > 0) x = -16;
		else if(cx > 0)	x = sx - 252;
		else x = 0;

		y = new_hero_y;
		break;
	}

	viewport.x -= new_region_offset_x;
	viewport.y -= new_region_offset_y;

	run_scrolling_script_int(waitdraw);

	viewport.x += new_region_offset_x;
	viewport.y += new_region_offset_y;

	x = storex, y = storey;
	action=lastaction; FFCore.setHeroAction(lastaction);
}

void HeroClass::run_scrolling_script(int32_t scrolldir, int32_t cx, int32_t sx, int32_t sy, bool end_frames, bool waitdraw)
{
	if (get_qr(qr_BROKEN_SCRIPTS_SCROLLING_HERO_POSITION))
	{
		// Old code has an off-by-one error, and doesn't clamp the hero position to the viewport.
		//
		// Although this is behind a compat qr, most test replays wouldn't fail. The ones that do are:
		//
		// 	nargads: different sfx plays during last frames of scrolling, sometimes
		// 	crucible: fog is slightly different on last frames of scrolling
		// 	yuurand: breaks. blocks are not removed when returning to a screen during scroll
		run_scrolling_script_old(scrolldir, cx, sx, sy, end_frames, waitdraw);
		return;
	}

	// For rafting (and possibly other esoteric things)
	// Hero's action should remain unchanged while scrolling,
	// but for the sake of scripts, here's an eye-watering kludge.
	actiontype lastaction = action;
	action=scrolling; FFCore.setHeroAction(scrolling);

	// Also, hero coordinates should remain unchanged.
	zfix storex = x, storey = y;

	if(waitdraw)
	{
		FFCore.runGenericPassiveEngine(SCR_TIMING_WAITDRAW);
	}
	else
	{
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_FFCS-1);
	}

	switch(scrolldir)
	{
	case up:
		if(y <= scrolling_new_region.height - 16) y = scrolling_new_region.height;
		else y = scrolling_new_region.height - 16;

		x = new_hero_x;
		break;
		
	case down:
		if(y >= 0) y = -16;
		else y = 0;

		x = new_hero_x;
		break;
		
	case left:
		if(x <= scrolling_new_region.width - 16) x = scrolling_new_region.width;
		else x = scrolling_new_region.width - 16;

		y = new_hero_y;
		break;
		
	case right:
		if(x >= 0) x = -16;
		else x = 0;

		y = new_hero_y;
		break;
	}

	viewport.x -= new_region_offset_x;
	viewport.y -= new_region_offset_y;

	if (scrolldir == left || scrolldir == right)
		x.doClamp(viewport.left(), viewport.right() - 16);
	if (scrolldir == up || scrolldir == down)
		y.doClamp(viewport.top(), viewport.bottom() - 16);

	run_scrolling_script_int(waitdraw);

	viewport.x += new_region_offset_x;
	viewport.y += new_region_offset_y;
	
	x = storex, y = storey;
	action=lastaction; FFCore.setHeroAction(lastaction);
}

//Has solving the maze enabled a side warp?
//Only used just before scrolling screens
// Note: since scrollscr() calls this, and dowarp() calls scrollscr(),
// return true to abort the topmost scrollscr() call. -L
bool HeroClass::maze_enabled_sizewarp(const mapscr *scr, int32_t scrolldir)
{
	maze_state.last_check_herox = x;
	maze_state.last_check_heroy = y;

    for(int32_t i = 0; i < 3; i++) lastdir[i] = lastdir[i+1];
    
    lastdir[3] = scr->flags&fMAZE ? scrolldir : 0xFF;
    
    if(scr->flags8&fMAZEvSIDEWARP && scr->flags&fMAZE && scrolldir != scr->exitdir)
    {
        switch(scrolldir)
        {
        case up:
            if(scr->flags2&wfUP && checkmaze(scr,true))
            {
                lastdir[3] = 0xFF;
                sdir=up;
                dowarp(scr,1,(scr->sidewarpindex)&3);
                return true;
            }
            
            break;
            
        case down:
            if(scr->flags2&wfDOWN && checkmaze(scr,true))
            {
                lastdir[3] = 0xFF;
                sdir=down;
                dowarp(scr,1,(scr->sidewarpindex>>2)&3);
                return true;
            }
            
            break;
            
        case left:
            if(scr->flags2&wfLEFT && checkmaze(scr,true))
            {
                lastdir[3] = 0xFF;
                sdir=left;
                dowarp(scr,1,(scr->sidewarpindex>>4)&3);
                return true;
            }
            
            break;
            
        case right:
            if(scr->flags2&wfRIGHT && checkmaze(scr,true))
            {
                lastdir[3] = 0xFF;
                sdir=right;
                dowarp(scr,1,(scr->sidewarpindex)&3);
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

void HeroClass::calc_darkroom_hero(int32_t x1, int32_t y1)
{
	if(!get_qr(qr_NEW_DARKROOM)) return;
	int32_t lampid = current_item_id(itype_lantern);
	if(lampid < 0) return;
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
	optional<word> wave_opt;
	if(lamp.flags & item_flag1)
		wave_opt = 0; // cancel wave effect
	handle_lighting(hx, hy, lamp.misc1, lamp.misc2, dir, darkscr_bmp, NULL, -1, -1, -1, -1, wave_opt, wave_opt);
}

struct nearby_scrolling_screen_t
{
	int screen;
	int offx;
	int offy;
	bool is_new;
	screen_handles_t screen_handles;
};

struct rect_t
{
	int l, r, t, b;

	rect_t() = default;
	explicit rect_t(const viewport_t& viewport) : l(viewport.left()), r(viewport.right()), t(viewport.top()), b(viewport.bottom()) {}

	void union_with(const rect_t& other)
	{
		l = std::min(l, other.l);
		r = std::max(r, other.r);
		t = std::min(t, other.t);
		b = std::max(b, other.b);
	}

	void intersect_with(const rect_t& other)
	{
		l = std::max(l, other.l);
		r = std::min(r, other.r);
		t = std::max(t, other.t);
		b = std::min(b, other.b);

		if (l > r || t > b)
			l = r = t = b = 0;
	}

	bool intersects_with(const rect_t& other) const
	{
		return std::max(t, other.t) < std::min(b, other.b) && std::max(l, other.l) < std::min(r, other.r);
	}

	int left() const {return l;}
	int right() const {return r;}
	int top() const {return t;}
	int bottom() const {return b;}
};

struct nearby_scrolling_screens_t
{
	std::vector<nearby_scrolling_screen_t> screens;
	// Only ever true during whistle warp scrolling.
	bool has_overlapping_screens;
	rect_t old_screens_rect;
	rect_t new_screens_rect;
};

// Returns all the screens (old and new region) that need to be rendered during scrolling, along
// with thier draw offsets.
// Note: The destination screen is Hero.current_screen, the starting screen is scrolling_hero_screen.
// old_viewport_start: the viewport in the old region at start of scrolling
// new_viewport_final: the viewport in the new region at end of scrolling
// old_region_visible: the viewport in the old region that will be visible
// new_region_visible: the viewport in the new region that will be visible
static nearby_scrolling_screens_t get_nearby_scrolling_screens(const std::vector<mapscr*>& old_temporary_screens, viewport_t old_viewport_start, viewport_t new_viewport_final, rect_t old_region_visible, rect_t new_region_visible)
{
	bool is_whistle_warp_scroll = HeroInOutgoingWhistleWarp();
	int old_region = get_region_id(scrolling_map, scrolling_hero_screen);

	nearby_scrolling_screens_t nearby_screens{};
	nearby_screens.has_overlapping_screens = is_whistle_warp_scroll;

	// base_scr, use_new_screens, offx, offy
	std::vector<std::tuple<mapscr*, bool, int, int>> old_screen_deltas;
	std::vector<std::tuple<mapscr*, bool, int, int>> new_screen_deltas;

	int old_screens_x0 = std::clamp(old_region_visible.left() / 256, 0, 15);
	int old_screens_x1 = std::clamp((old_region_visible.right() - 1) / 256, 0, 15);
	int old_screens_y0 = std::clamp(old_region_visible.top() / 176, 0, 8);
	int old_screens_y1 = std::clamp((old_region_visible.bottom() - 1) / 176, 0, 8);
	for (int x = old_screens_x0; x <= old_screens_x1; x++)
	{
		for (int y = old_screens_y0; y <= old_screens_y1; y++)
		{
			int screen = scrolling_region.origin_screen + x + y*16;
			if (get_region_id(scrolling_map, screen) != old_region)
				continue;

			mapscr* base_scr = old_temporary_screens[screen*7];
			CHECK(base_scr);
			bool use_new_screens = false;
			int offx = get_region_relative_dx(screen, scrolling_region.origin_screen) * 256;
			int offy = get_region_relative_dy(screen, scrolling_region.origin_screen) * 176;
			old_screen_deltas.push_back({base_scr, use_new_screens, offx, offy});
		}
	}

	// This translates from the new region to the old region's coordinates ...
	int dx, dy;
	if (is_whistle_warp_scroll)
	{
		// ... anchored at the current viewport.
		if (scrolling_dir == right)
			dx = old_viewport_start.right() - new_viewport_final.right();
		else
			dx = old_viewport_start.left() - new_viewport_final.left();

		if (scrolling_dir == up)
			dy = old_viewport_start.bottom() - new_viewport_final.bottom();
		else
			dy = old_viewport_start.top() - new_viewport_final.top();

		if      (scrolling_dir == up) dy -= old_viewport_start.h;
		else if (scrolling_dir == down) dy += old_viewport_start.h;
		else if (scrolling_dir == left) dx -= old_viewport_start.w;
		else if (scrolling_dir == right) dx += old_viewport_start.w;
	}
	else
	{
		// ... anchored at the point where the screen scrolling starts.
		dx = get_region_relative_dx(cur_screen, scrolling_region.origin_screen) -
			(get_region_relative_dx(Hero.current_screen, scrolling_region.origin_screen) - get_region_relative_dx(scrolling_hero_screen, scrolling_region.origin_screen));
		dy = get_region_relative_dy(cur_screen, scrolling_region.origin_screen) -
			(get_region_relative_dy(Hero.current_screen, scrolling_region.origin_screen) - get_region_relative_dy(scrolling_hero_screen, scrolling_region.origin_screen));
		if      (scrolling_dir == up) dy -= 1;
		else if (scrolling_dir == down) dy += 1;
		else if (scrolling_dir == left) dx -= 1;
		else if (scrolling_dir == right) dx += 1;
		dx *= 256;
		dy *= 176;
	}

	int new_screens_x0 = std::clamp(new_region_visible.left() / 256, 0, 15);
	int new_screens_x1 = std::clamp((new_region_visible.right() - 1) / 256, 0, 15);
	int new_screens_y0 = std::clamp(new_region_visible.top() / 176, 0, 8);
	int new_screens_y1 = std::clamp((new_region_visible.bottom() - 1) / 176, 0, 8);

	for (int x = new_screens_x0; x <= new_screens_x1; x++)
	{
		for (int y = new_screens_y0; y <= new_screens_y1; y++)
		{
			int screen = cur_screen + x + y*16;
			if (!is_in_current_region(screen))
				continue;

			int sx = get_region_relative_dx(screen, cur_screen);
			int sy = get_region_relative_dy(screen, cur_screen);
			int offx = sx * 256 + dx;
			int offy = sy * 176 + dy;

			mapscr* base_scr = get_scr(cur_map, screen);
			bool use_new_screens = true;
			new_screen_deltas.push_back({base_scr, use_new_screens, offx, offy});
		}
	}

	// First handle the old screens, then the new screens.
	std::vector<std::tuple<mapscr*, bool, int, int>> screen_deltas;
	screen_deltas.insert(screen_deltas.end(), old_screen_deltas.begin(), old_screen_deltas.end());
	screen_deltas.insert(screen_deltas.end(), new_screen_deltas.begin(), new_screen_deltas.end());

	for (const auto& pair : screen_deltas)
	{
		mapscr* base_scr = std::get<0>(pair);
		bool use_new_screens = std::get<1>(pair);
		int offx = std::get<2>(pair);
		int offy = std::get<3>(pair);

		int screen = base_scr->screen;
		int map = base_scr->map;

		auto& nearby_screen = nearby_screens.screens.emplace_back();
		nearby_screen.screen = screen;
		nearby_screen.offx = offx;
		nearby_screen.offy = offy;
		nearby_screen.is_new = use_new_screens;

		nearby_screen.screen_handles[0] = {base_scr, base_scr, screen, 0};
		for (int i = 1; i <= 6; i++)
		{
			mapscr* scr = use_new_screens ?
				get_scr_layer(map, screen, i) :
				old_temporary_screens[screen*7 + i];
			nearby_screen.screen_handles[i] = {base_scr, scr->is_valid() ? scr : nullptr, screen, i};
		}
	}

	// When old/new screens are possibly overlapping, reduce the drawing clip used for new screens based on the old viewport.
	if (nearby_screens.has_overlapping_screens)
	{
		rect_t old_rect = rect_t(old_viewport_start);

		std::vector<rect_t> new_rects;
		for (const auto& nearby_screen : nearby_screens.screens)
		{
			if (!nearby_screen.is_new)
				continue;

			rect_t rect;
			rect.l = nearby_screen.offx;
			rect.r = nearby_screen.offx + 256;
			rect.t = nearby_screen.offy;
			rect.b = nearby_screen.offy + 176;
			new_rects.push_back(rect);
		}

		rect_t new_rect = new_rects.at(0);
		for (int i = 1; i < new_rects.size(); i++)
			new_rect.union_with(new_rects[i]);

		if (new_rect.intersects_with(old_rect))
		{
			if (scrolling_dir == right && new_rect.l < old_rect.r)
				new_rect.l = old_rect.r + 1;
			else if (scrolling_dir == left && new_rect.r > old_rect.l)
				new_rect.r = old_rect.l - 1;
			else if (scrolling_dir == up && new_rect.b > old_rect.t)
				new_rect.b = old_rect.t - 1;
			else if (scrolling_dir == down && new_rect.t < old_rect.b)
				new_rect.t = old_rect.b + 1;
		}

		nearby_screens.old_screens_rect = old_rect;
		nearby_screens.new_screens_rect = new_rect;
	}

	return nearby_screens;
}

static void for_every_nearby_screen_during_scroll(
	const nearby_scrolling_screens_t& nearby_screens,
	const std::function <void (screen_handles_t, int, int, int, bool)>& fn)
{
	if (!nearby_screens.has_overlapping_screens)
	{
		for (const auto& nearby_screen : nearby_screens.screens)
			fn(nearby_screen.screen_handles, nearby_screen.screen, nearby_screen.offx, nearby_screen.offy, nearby_screen.is_new);
		return;
	}

	int cl = framebuf->cl;
	int cr = framebuf->cr;
	int ct = framebuf->ct;
	int cb = framebuf->cb;

	for (const auto& nearby_screen : nearby_screens.screens)
	{
		auto& rect = nearby_screen.is_new ? nearby_screens.new_screens_rect : nearby_screens.old_screens_rect;
		int l = rect.l - viewport.x;
		int t = rect.t - viewport.y + playing_field_offset;
		int r = rect.r - viewport.x;
		int b = rect.b - viewport.y + playing_field_offset;
		add_clip_rect(framebuf, l, t, r, b);

		fn(nearby_screen.screen_handles, nearby_screen.screen, nearby_screen.offx, nearby_screen.offy, nearby_screen.is_new);

		set_clip_rect(framebuf, cl, ct, cr, cb);
	}
}

static void scrollscr_handle_dark(mapscr* newscr, mapscr* oldscr, const nearby_scrolling_screens_t& nearby_screens)
{
	extern int dither_offx;
	extern int dither_offy;

	clear_darkroom_bitmaps();
	set_clip_rect(framebuf, 0, playing_field_offset, 256, framebuf->h);

	for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
		mapscr* base_scr = screen_handles[0].scr;
		bool dark = is_new_screen ? is_dark(base_scr) : scrolling_is_dark(base_scr);
		if (!dark)
		{
			offy += playing_field_offset;
			rectfill(darkscr_bmp, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
			rectfill(darkscr_bmp_trans, offx - viewport.x, offy - viewport.y, offx - viewport.x + 256 - 1, offy - viewport.y + 176 - 1, 0);
		}
	});

	for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
		mapscr* base_scr = screen_handles[0].scr;

		dither_offx = is_new_screen ? -new_region_offset_x : 0;
		dither_offy = is_new_screen ? -new_region_offset_y : 0;
		calc_darkroom_combos(base_scr, offx, offy + playing_field_offset);

		int offx2 = is_new_screen ? new_region_offset_x : 0;
		int offy2 = is_new_screen ? new_region_offset_y : 0;
		calc_darkroom_ffcs(base_scr, offx2, offy2 + playing_field_offset);
	});

	Hero.calc_darkroom_hero(0, -playing_field_offset);
	dither_offx = 0;
	dither_offy = 0;

	do_primitives(framebuf, SPLAYER_DARKROOM_UNDER);
	set_clip_rect(framebuf, 0, playing_field_offset, framebuf->w, framebuf->h);
	if (hero_scr->flags9 & fDARK_DITHER) //dither the entire bitmap
	{
		ditherblit(darkscr_bmp,darkscr_bmp,0,game->get_dither_type(),game->get_dither_arg());
		ditherblit(darkscr_bmp_trans,darkscr_bmp_trans,0,game->get_dither_type(),game->get_dither_arg());
	}
	
	color_map = &trans_table2;
	if(hero_scr->flags9 & fDARK_TRANS) //draw the dark as transparent
	{
		draw_trans_sprite(framebuf, darkscr_bmp, 0, 0);
		if(get_qr(qr_NEWDARK_TRANS_STACKING))
			draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
	}
	else
	{
		masked_blit(darkscr_bmp, framebuf, 0, 0, 0, 0, framebuf->w, framebuf->h);
		draw_trans_sprite(framebuf, darkscr_bmp_trans, 0, 0);
	}
	color_map = &trans_table;
	
	set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
	do_primitives(framebuf, SPLAYER_DARKROOM_OVER);
}

static void do_ffc_scroll_layer(BITMAP* dest, int layer, const nearby_scrolling_screens_t& nearby_screens, int xoff, int yoff)
{
	if(!get_qr(qr_FFCSCROLL))
		return;
	// Draw all FFCs from the previous region.
	for (int i = 0; i < FFCore.ScrollingScreensAll.size(); i += 7)
	{
		mapscr* scr = FFCore.ScrollingScreensAll[i];
		if (!scr)
			continue;

		auto screen_handle = screen_handle_t{scr, scr, scr->screen, 0};
		do_ffc_layer(dest, layer, screen_handle, 0, 0);
	}

	for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
		if (!is_new_screen)
			return;

		do_ffc_layer(dest, layer, screen_handles[0], xoff, yoff);
	});
}

void HeroClass::scrollscr(int32_t scrolldir, int32_t dest_screen, int32_t destdmap)
{
	if(action==freeze||action==sideswimfreeze)
	{
		return;
	}

	// If on a slope in sideview mode, move along that slope a bit.
	zfix sideview_scrolling_slope;
	if (sideview_mode())
	{
		y+=2+0.0001_zf;
		slope_info const& s = get_slope(this, true).get_info();
		y-=2+0.0001_zf;

		sideview_scrolling_slope = s.slope();
		if (sideview_scrolling_slope)
		{
			int dx = 0;
			int dy = 0;
			if (scrolldir == left)
				dx = -1;
			else if (scrolldir == right)
				dx = 1;
			else if (scrolldir == up)
				dy = -1;
			else if (scrolldir == down)
				dy = 1;

			// "Ride" the slope for 16 pixels.
			for (int i = 0; i < 16; i++)
			{
				if (scrolldir == left || scrolldir == right)
				{
					x += dx;
					y = s.getY(x + (sideview_scrolling_slope > 0 ? 0 : hit_width)) - hit_height;
				}
				else
				{
					x += 1 / sideview_scrolling_slope * dy;
					y = s.getY(x + (sideview_scrolling_slope > 0 ? 0 : hit_width)) - hit_height;
				}

				herostep();
				draw_screen();
				advanceframe(true);

				// Check if the slope the player is on has changed.
				y+=2+0.0001_zf;
				slope_info const& s = get_slope(this, true).get_info();
				y-=2+0.0001_zf;
				sideview_scrolling_slope = s.slope();
				if (!sideview_scrolling_slope)
					break;
			}
		}
	}

	if (viewport_mode != ViewportMode::CenterAndBound || get_viewport_sprite() != &Hero)
	{
		set_viewport_sprite(&Hero);
		viewport_mode = ViewportMode::CenterAndBound;
		update_viewport();
	}

	bool overlay = false;
	if(scrolldir >= 0 && scrolldir <= 3)
	{
		overlay = get_bit(&(cur_screen >= 128 ? special_warp_return_scr : origin_scr)->sidewarpoverlayflags, scrolldir) ? true : false;
	}

	int old_dmap = cur_dmap;
	int new_dmap = destdmap >= 0 ? destdmap : cur_dmap;
	int new_map = DMaps[new_dmap].map;

	bool updatemusic = FFCore.can_dmap_change_music(destdmap);
	bool musicrevert = FFCore.music_update_flags & MUSIC_UPDATE_FLAG_REVERT;

	if (!has_advanced_maze(origin_scr) && maze_enabled_sizewarp(origin_scr, scrolldir))
		return; // dowarp() was called

	int original_destscr = dest_screen;
	if (dest_screen == -1)
	{
		dest_screen = current_screen;
		if (checkmaze(origin_scr, false) && !edge_of_dmap(scrolldir)) {
			dest_screen = screen_index_direction(dest_screen, (direction)scrolldir);
		}
	}

	int old_region_scr_dx = region_scr_dx;
	int old_region_scr_dy = region_scr_dy;
	int old_world_w = world_w;
	int old_world_h = world_h;
	int old_original_playing_field_offset = original_playing_field_offset;
	viewport_t old_viewport = viewport;
	region_t old_region = cur_region;

	// Determine what the player position will be after scrolling (within the new screen's coordinate system),
	// and what the new viewport will be.
	viewport_t new_viewport;
	region_t new_region;
	int new_scr_dx, new_scr_dy;
	auto calc_new_viewport_and_pos = [&](){
		calculate_region(new_map, dest_screen, new_region, new_scr_dx, new_scr_dy);

		// These mark the top-left coordinate of the new region, in the old region's world coordinates.
		new_region_offset_x = (new_region.origin_screen_x - old_region.origin_screen_x)*256;
		new_region_offset_y = (new_region.origin_screen_y - old_region.origin_screen_y)*176;

		new_hero_x = 0;
		new_hero_y = 0;
		switch (scrolldir)
		{
			case up:
			{
				new_hero_x.val = (new_scr_dx - old_region_scr_dx) * 256 * 10000L + x.val;
				new_hero_y = new_region.height - 16;
			}
			break;

			case down:
			{
				new_hero_x.val = (new_scr_dx - old_region_scr_dx) * 256 * 10000L + x.val;
				new_hero_y = 0;
			}
			break;

			case left:
			{
				new_hero_x = new_region.width - 16;
				new_hero_y.val = (new_scr_dy - old_region_scr_dy) * 176 * 10000L + y.val;
			}
			break;

			case right:
			{
				new_hero_x = 0;
				new_hero_y.val = (new_scr_dy - old_region_scr_dy) * 176 * 10000L + y.val;
			}
			break;

			// Should never happen ...
			default:
			{
				abort();
			}
		}

		int new_hero_x_for_viewport = new_hero_x;
		int new_hero_y_for_viewport = new_hero_y;

		// If arriving in a whistle warp, the new viewport will be based on the warp destination instead.
		if (HeroInOutgoingWhistleWarp())
		{
			const mapscr* newscr = get_canonical_scr(new_map, dest_screen);

			if(get_qr(qr_NOARRIVALPOINT))
				new_hero_x_for_viewport=newscr->warpreturnx[0];
			else new_hero_x_for_viewport=newscr->warparrivalx;
			new_hero_x_for_viewport += new_scr_dx*256;

			if(get_qr(qr_NOARRIVALPOINT))
				new_hero_y_for_viewport=newscr->warpreturny[0];
			else new_hero_y_for_viewport=newscr->warparrivaly;
			new_hero_y_for_viewport += new_scr_dy*176;
		}

		new_viewport = {};
		calculate_viewport(new_viewport, new_dmap, dest_screen, new_region.width, new_region.height, new_hero_x_for_viewport + Hero.txsz*16/2, new_hero_y_for_viewport + Hero.tysz*16/2);

		scrolling_new_region = new_region;
	};
	calc_new_viewport_and_pos();

	int step = get_scroll_step(scrolldir);
	int delay = get_scroll_delay(scrolldir);

	int scroll_counter, scroll_amount, dx, dy;
	int secondary_axis_alignment_amount;
	auto calc_scroll_data = [&](){
		int scroll_height = std::min(viewport.h, new_viewport.h);
		int scroll_width = std::min(viewport.w, new_viewport.w);
		scroll_amount = scrolldir == up || scrolldir == down ? scroll_height : scroll_width;
		scroll_counter = scroll_amount / step;
		if (scroll_amount % step != 0)
			scroll_counter++;

		dx = 0;
		dy = 0;
		if (scrolldir == up)    dy = -1;
		if (scrolldir == down)  dy = 1;
		if (scrolldir == left)  dx = -1;
		if (scrolldir == right) dx = 1;

		// Determine by how much we need to align to the new region's viewport.
		// This sets `secondary_axis_alignment_amount` to the number of pixels needed to adjust along the secondary axis
		// to move (the position of link relative to the display) from the old viewport to the new viewport.
		int old_hero_screen_x = x.getInt() - old_viewport.x;
		int old_hero_screen_y = y.getInt() - old_viewport.y + old_original_playing_field_offset;
		int new_hero_screen_x = new_hero_x - new_viewport.x;
		int new_hero_screen_y = new_hero_y - new_viewport.y + (232 - new_viewport.h);
		if (dx)      secondary_axis_alignment_amount = new_hero_screen_y - old_hero_screen_y;
		else if (dy) secondary_axis_alignment_amount = new_hero_screen_x - old_hero_screen_x;
		else         secondary_axis_alignment_amount = 0;

		if (HeroInOutgoingWhistleWarp()) secondary_axis_alignment_amount = 0;
	};
	calc_scroll_data();

	bool isForceFaceUp = getOnSideviewLadder() && canSideviewLadder() &&
		!(jumping<0 || fall!=0 || fakefall!=0) && get_qr(qr_SIDEVIEWLADDER_FACEUP);
	if (isForceFaceUp) dir = up;

	bool is_unsmooth_vertical_scrolling =
		(scrolldir == up || scrolldir == down) && get_qr(qr_SMOOTHVERTICALSCROLLING) == 0;

	kill_enemy_sfx();
	stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
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
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_DELTA_X] = new_region_offset_x;
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_DELTA_Y] = new_region_offset_y;
	FFCore.ScrollingData[SCROLLDATA_NRX] = new_region_offset_x - viewport.x;
	FFCore.ScrollingData[SCROLLDATA_NRY] = new_region_offset_y - viewport.y;
	FFCore.ScrollingData[SCROLLDATA_ORX] = -viewport.x;
	FFCore.ScrollingData[SCROLLDATA_ORY] = -viewport.y;

	// Get the screen coords of the top-left of the screen we are scrolling away from.
	auto [old_sx, old_sy] = translate_screen_coordinates_to_world(current_screen);
	FFCore.ScrollingData[SCROLLDATA_OX] = old_sx - viewport.x;
	FFCore.ScrollingData[SCROLLDATA_OY] = old_sy - viewport.y;

	FFCore.ScrollingData[SCROLLDATA_NEW_SCREEN_X] = new_scr_dx * 256;
	FFCore.ScrollingData[SCROLLDATA_NEW_SCREEN_Y] = new_scr_dy * 176;

	FFCore.ScrollingData[SCROLLDATA_OLD_SCREEN_X] = old_sx;
	FFCore.ScrollingData[SCROLLDATA_OLD_SCREEN_Y] = old_sy;

	FFCore.ScrollingData[SCROLLDATA_NEW_HERO_X] = new_hero_x.getInt();
	FFCore.ScrollingData[SCROLLDATA_NEW_HERO_Y] = new_hero_y.getInt();

	FFCore.ScrollingData[SCROLLDATA_OLD_HERO_X] = x.getInt();
	FFCore.ScrollingData[SCROLLDATA_OLD_HERO_Y] = y.getInt();

	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_SCREEN_WIDTH] = new_region.screen_width;
	FFCore.ScrollingData[SCROLLDATA_NEW_REGION_SCREEN_HEIGHT] = new_region.screen_height;

	FFCore.ScrollingData[SCROLLDATA_OLD_REGION_SCREEN_WIDTH] = cur_region.screen_width;
	FFCore.ScrollingData[SCROLLDATA_OLD_REGION_SCREEN_HEIGHT] = cur_region.screen_height;

	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_WIDTH] = new_viewport.w;
	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_HEIGHT] = new_viewport.h;

	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_WIDTH] = viewport.w;
	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_HEIGHT] = viewport.h;

	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_X] = new_viewport.x;
	FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_Y] = new_viewport.y;

	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_X] = viewport.x;
	FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_Y] = viewport.y;

	FFCore.clear_combo_scripts();

	conveyclk = 2;
	screenscrolling = true;
	scrolling_dir = (direction) scrolldir;
	scrolling_hero_screen = current_screen;
	scrolling_region = cur_region;
	scrolling_region_is_lit = region_is_lit;

	int32_t scx = get_qr(qr_FASTDNGN) ? 30 : 0;
	if(get_qr(qr_VERYFASTSCROLLING)) //just a minor adjustment.
		scx = 32; //for sideview very fast screolling.
	
	auto hero_x_before_scripts = x;
	auto hero_y_before_scripts = y;

	// Don't signal to scripts that scrolling has "started" (and thus all the Game->Scrolling variables are valid)
	// just yet. Store what we calculated and apply them after this next frame.
	// TODO(replays): update. And probably just move the calculation to after this frame renders (rather than cache).
	bool crucible_quest_compat = replay_is_debug() && replay_get_meta_str("qst") == "crucible_quest.qst";
	int cached_scrolling[SZ_SCROLLDATA];
	if (!crucible_quest_compat)
	{
		for (int i = 0; i < SZ_SCROLLDATA; i++)
			cached_scrolling[i] = FFCore.ScrollingData[i];
		memset(FFCore.ScrollingData, 0, sizeof(int32_t) * SZ_SCROLLDATA);
		FFCore.ScrollingData[SCROLLDATA_DIR] = -1;
	}

	// Wait one frame. This still uses the old region's coordinates.
	int32_t lastattackclk = attackclk, lastspins = spins, lastcharging = charging;
	bool lasttapping = tapping;
	actiontype lastaction = action;
	{
		ALLOFF(false, false);
		// for now, restore Hero's previous action
		if(!get_qr(qr_SCROLLING_KILLS_CHARGE))
			charging = lastcharging;
		if (replay_version_check(0, 28))
		{
			// nargads_trail_crystal_crusades replay tests need this.
			if(!get_qr(qr_SCROLLING_KILLS_CHARGE)) attackclk = lastattackclk;
			spins = lastspins; charging = lastcharging; tapping = lasttapping;
		}

		action=lastaction; FFCore.setHeroAction(lastaction);

		lstep = (lstep + 6) % 12;
		FFCore.runGenericPassiveEngine(SCR_TIMING_WAITDRAW);
		if((!( FFCore.system_suspend[susptGLOBALGAME] )) && FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME))
		{
			ZScriptVersion::RunScript(ScriptType::Global, GLOBAL_SCRIPT_GAME, GLOBAL_SCRIPT_GAME);
			FFCore.waitdraw(ScriptType::Global, GLOBAL_SCRIPT_GAME) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_GLOBAL_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptHEROACTIVE] )) && FFCore.waitdraw(ScriptType::Hero) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::Hero, SCRIPT_HERO_ACTIVE);
			FFCore.waitdraw(ScriptType::Hero) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_PLAYER_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::DMap) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::DMap, DMaps[cur_dmap].script,cur_dmap);
			FFCore.waitdraw(ScriptType::DMap) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_ACTIVE_WAITDRAW);
		if ( (!( FFCore.system_suspend[susptDMAPSCRIPT] )) && FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && FFCore.getQuestHeaderInfo(vZelda) >= 0x255 )
		{
			ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script,cur_dmap);
			FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DMAPDATA_PASSIVESUBSCREEN_WAITDRAW);

		if (FFCore.getQuestHeaderInfo(vZelda) >= 0x255 && !FFCore.system_suspend[susptSCREENSCRIPTS])
		{
			for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
				if (scr->script != 0 && FFCore.waitdraw(ScriptType::Screen, scr->screen))
				{
					ZScriptVersion::RunScript(ScriptType::Screen, scr->script, scr->screen);  
					FFCore.waitdraw(ScriptType::Screen, scr->screen) = 0;
				}
			});
		}
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_SCREEN_WAITDRAW);

		for_every_ffc([&](const ffc_handle_t& ffc_handle) {
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

	if (!crucible_quest_compat)
	{
		for (int i = 0; i < SZ_SCROLLDATA; i++)
			FFCore.ScrollingData[i] = cached_scrolling[i];
	}

	// cur_dmap won't change until the end of the scroll. Store new dmap in this global variable.
	scrolling_destdmap = new_dmap;

	// Calling functions are responsible for setting cur_map (but not cur_screen...), but before we
	// _actually_ start to scroll we draw a few frames of the current screen (draw_screen). So we
	// need cur_map to be the old value initially. Callers also set the old map value to
	// `scrolling_map`, so we can use that.
	int destmap = cur_map;
	cur_map = scrolling_map;

	// This adjusts how drawing commands are interpreted in `do_drawing_command`.
	// Currently, since only one set of screen scripts/item scripts/etc. can run at a time during
	// scrolling (either the old screens, which was above and this next "waiting" phase, or the new
	// screens, which comes after this next part) - a boolean is enough to capture this. If this
	// changes, we need to vary this behavior based on "is this from the new or old set of screens?"
	scrolling_using_new_region_coords = true;

	// Wait at least one frame, possibly 32.
	// These frames will use the new region's coordinates.
	{
		int wait_counter = scx + 1;
		while (wait_counter < 32)
		{
			if(isForceFaceUp) dir = up;
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

	script_drawing_commands.Clear();
	FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);

	// Just trying to play the sound.
	if (original_destscr == -1)
		checkmaze(origin_scr, true);

	switch(DMaps[cur_dmap].type&dmfTYPE)
	{
		case dmDNGN:
			if(!get_qr(qr_DUNGEONS_USE_CLASSIC_CHARTING))
			{
				markBmap(scrolldir, current_screen);
			}
			break;
		case dmOVERW: case dmBSOVERW:
			if(get_qr(qr_NO_OVERWORLD_MAP_CHARTING))
				break;
			[[fallthrough]];
		case dmCAVE:
			markBmap(scrolldir, current_screen);
			break;
	}

	// Remember everything about the current region, because `loadscr` is about to reset this data.
	std::vector<mapscr*> old_temporary_screens;
	old_temporary_screens = take_temporary_scrs();
	FFCore.ScrollingScreensAll = old_temporary_screens;
	cur_map = destmap;

	loadscr(destdmap, dest_screen, scrolldir, overlay);
	mapscr* oldscr = old_temporary_screens[scrolling_hero_screen * 7];
	mapscr* newscr = get_scr(destmap, dest_screen);

	// Start scrolling with the previous pfo, and adjust during scrolling if necessary.
	int new_playing_field_offset = playing_field_offset;
	playing_field_offset = old_original_playing_field_offset;

	// We must recalculate the new hero position and viewport, if a script run above just change the
	// hero position.
	if (hero_x_before_scripts != x || hero_y_before_scripts != y)
	{
		calc_new_viewport_and_pos();
		calc_scroll_data();

		FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_X] = new_viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NEW_VIEWPORT_Y] = new_viewport.y;

		FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_X] = viewport.x;
		FFCore.ScrollingData[SCROLLDATA_OLD_VIEWPORT_Y] = viewport.y;

		FFCore.ScrollingData[SCROLLDATA_NEW_HERO_X] = new_hero_x.getInt();
		FFCore.ScrollingData[SCROLLDATA_NEW_HERO_Y] = new_hero_y.getInt();
	}

	// 0 for change playing field offset, then scroll.
	// 1 for scroll, then change playing field offset.
	int pfo_mode = 0;

	if (dy == 1)
		pfo_mode = 0;
	else if (dy == -1)
		pfo_mode = 1;
	else if (old_region_scr_dy == 0 && sign(new_playing_field_offset - old_original_playing_field_offset) == -1)
		pfo_mode = 1;

	int pfo_counter = abs(new_playing_field_offset - old_original_playing_field_offset);

	// When scrolling up/down, and if scrolling first and the final pfo is less, reduce the scroll distance.
	if (dy && pfo_mode == 1 && new_playing_field_offset > old_original_playing_field_offset)
	{
		scroll_amount += new_playing_field_offset - old_original_playing_field_offset;
		scroll_counter = scroll_amount / step;
	}

	// If adjusting pfo first and the final pfo is more, increase the scroll distance.
	// Also make the pfo adjust instantly.
	if (dy && pfo_mode == 0 && new_playing_field_offset < old_original_playing_field_offset)
	{
		scroll_amount -= new_playing_field_offset - old_original_playing_field_offset;
		scroll_counter = scroll_amount / step;
		pfo_counter = 1;
	}

	viewport_t old_world_rect;
	old_world_rect.x = 0;
	old_world_rect.y = 0;
	old_world_rect.w = old_world_w;
	old_world_rect.h = old_world_h;

	viewport_t new_world_rect;
	new_world_rect.x = 0;
	new_world_rect.y = 0;
	new_world_rect.w = world_w;
	new_world_rect.h = world_h;

	// 0 for align, then scroll.
	// 1 for scroll, then align.
	int align_mode = 0;
	int align_counter = abs(secondary_axis_alignment_amount);
	// Align first, unless that would show screens outside the old region.
	if (align_counter)
	{
		viewport_t old_viewport_aligned = old_viewport;
		old_viewport_aligned.x -= (dy ? secondary_axis_alignment_amount : 0);
		old_viewport_aligned.y -= (dx ? secondary_axis_alignment_amount : 0);
		// The playing field offset is changed before aligning, so apply the delta in this check.
		if (pfo_mode == 0)
			old_viewport_aligned.y += new_playing_field_offset - old_original_playing_field_offset;
		if (old_world_rect.contains_or_on(old_viewport_aligned))
			align_mode = 0;
		else
			align_mode = 1;
	}

	// Determine the area that will be visible in the old and the new regions,
	// taking into account any possible alignment. All screens within these
	// area will be fetched by get_nearby_scrolling_screens.
	rect_t old_region_visible = rect_t(old_viewport);
	rect_t new_region_visible = rect_t(new_viewport);
	if (secondary_axis_alignment_amount)
	{
		rect_t& r = align_mode == 0 ? old_region_visible : new_region_visible;
		int delta = align_mode == 0 ? -secondary_axis_alignment_amount : secondary_axis_alignment_amount;
		if (dy)
		{
			if (delta > 0)
				r.r += delta;
			else
				r.l += delta;
		}
		else if (dx)
		{
			if (delta > 0)
				r.b += delta;
			else
				r.t += delta;
		}
	}

	old_region_visible.intersect_with(rect_t(old_world_rect));
	new_region_visible.intersect_with(rect_t(new_world_rect));

	// For the duration of the scrolling, the old region coordinate system is used for all drawing
	// operations. This means that the new screens are drawn with offsets relative to the old
	// coordinate system. These offsets are determined in get_nearby_scrolling_screens.
	auto nearby_screens = get_nearby_scrolling_screens(old_temporary_screens, old_viewport, new_viewport, old_region_visible, new_region_visible);

	int sx = viewport.x + (scrolldir == left ? viewport.w : 0);
	int sy = viewport.y + (scrolldir == up ? viewport.h : 0);
	if (is_unsmooth_vertical_scrolling) sy += 3;

	// We must recalculate the new hero position, if a script run above just change the
	// hero position.
	if (hero_x_before_scripts != x || hero_y_before_scripts != y)
	{
		calc_new_viewport_and_pos();

		FFCore.ScrollingData[SCROLLDATA_NEW_HERO_X] = new_hero_x.getInt();
		FFCore.ScrollingData[SCROLLDATA_NEW_HERO_Y] = new_hero_y.getInt();
	}

	// change Hero's state if entering water
	int32_t ahead = lookahead(scrolldir, new_hero_x, new_hero_y + 8);
	auto [lookaheadx, lookaheady] = lookahead_coords(scrolldir, new_hero_x + 8, new_hero_y + (bigHitbox?8:12));
	auto [wateraheadx1, wateraheady1] = lookahead_coords(scrolldir, new_hero_x + 4, new_hero_y + 9);
	auto [wateraheadx2, wateraheady2] = lookahead_coords(scrolldir, new_hero_x + 11, new_hero_y + 15);
	
	bool nowinwater = false;
	{
		if(lastaction != inwind)
		{
			if(lastaction == rafting ) //&& isRaftFlag(aheadflag))
			{
				if (lookaheadraftflag(scrolldir, new_hero_x, new_hero_y))
				{
					action=rafting; FFCore.setHeroAction(rafting);
					raftclk=0;
				}
			}
			else if(iswaterex_z3(ahead, -1, wateraheadx1,wateraheady1) && iswaterex_z3(ahead, -1, wateraheadx2,wateraheady2) && (current_item(itype_flippers)))
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
		else if((lastaction == attacking || lastaction == sideswimattacking) && charging)
		{
			action = lastaction; FFCore.setHeroAction(lastaction);
		}
		else
		{
			action=none; FFCore.setHeroAction(none);
		}

		isForceFaceUp = isForceFaceUp && canSideviewLadderRemote(lookaheadx,lookaheady);
		
		// The naturaldark state can be read/set by an FFC script before
		// fade() or lighting() is called.
		naturaldark = !get_qr(qr_NEW_DARKROOM) && is_dark(newscr);
		
		if(newscr->oceansfx != oldscr->oceansfx)	adjust_sfx(oldscr->oceansfx, 128, false);
		
		if(newscr->bosssfx != oldscr->bosssfx)	adjust_sfx(oldscr->bosssfx, 128, false);

		//Preloaded ffc scripts
		{
			// Kludge
			cur_dmap = new_dmap;
			ffscript_engine(true);
			cur_dmap = old_dmap;
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
	for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
		mapscr* base_scr = screen_handles[0].scr;
		bool dark = is_new_screen ? is_dark(base_scr) : scrolling_is_dark(base_scr);
		draw_dark |= dark;
	});

	int no_move = 0;
	int move_counter = 0;
	bool end_frames = false;

	scroll_counter *= delay;

	if (get_qr(qr_NOSCROLL))
	{
		delay = 0;
		secondary_axis_alignment_amount = 0;
		pfo_counter = 0;
		align_counter = 0;
	}

	viewport_t initial_viewport = old_viewport;
	viewport = initial_viewport;
	if (is_unsmooth_vertical_scrolling) viewport.y += 3;

	// These mark the top-left coordinate of the new screen and the old screen, in the old region
	// coordinates.
	int nx = 0;
	int ny = 0;
	int ox = 0;
	int oy = 0;
	for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
		if (screen == dest_screen && is_new_screen)
		{
			nx = offx;
			ny = offy;
		}
		else if (screen == scrolling_hero_screen && !is_new_screen)
		{
			ox = offx;
			oy = offy;
		}
	});

	// FFCs coordinates are world positions, and so don't need an offset like when drawing a
	// specific screen's combos in do_scrolling_layer. But since their coordinates are in the new
	// region's coordinate system, an offset of the difference between the two coordinate systems is
	// needed.
	// TODO: figure out how to calculate this in calc_new_viewport_and_pos, before their first usage...
	new_region_offset_x = nx - get_region_relative_dx(dest_screen)*256;
	new_region_offset_y = ny - get_region_relative_dy(dest_screen)*176;

	cur_dmap = new_dmap;
	bool classic_draw = get_qr(qr_CLASSIC_DRAWING_ORDER);
	for (int i = 0; (scroll_counter >= 0 && delay != 0) || align_counter || pfo_counter; i++, scroll_counter--)
	{
		// Scripts see the hero position as if relative to the scrolling viewport. This is a weird
		// quirk that should probably be placed behind a compat QR.
		zfix prev_x, prev_y;
		#define SAVE_HERO_POS {prev_x = x; prev_y = y;}
		#define USE_COMPAT_HERO_POS {x -= viewport.x; y -= viewport.y;}
		#define RESTORE_HERO_POS {x = prev_x; y = prev_y;}

		if (replay_version_check(0, 3))
		{
			replay_poll();
		}

		if(Quit)
		{
			// Just for compat with pre-z3 replays that quit during a scroll.
			if (replay_is_recording())
			{
				USE_COMPAT_HERO_POS;
			}
			screenscrolling = false;
			return;
		}

		SAVE_HERO_POS;
		USE_COMPAT_HERO_POS;
		ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, false);
		RESTORE_HERO_POS;

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
				do_pfo_adjust = true;
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
			viewport.h = 232 - playing_field_offset;
			viewport.y = initial_viewport.y + step * move_counter * dy + (playing_field_offset - old_original_playing_field_offset);
			if (pfo_mode == 0 && pfo_counter)
				scroll_counter++;
		}

		if (lift_wpn)
			lift_wpn->yofs = playing_field_offset - 2;

		if(!no_move)
		{
			switch(scrolldir)
			{
			case up:
				sy -= step;
				break;
				
			case down:
				sy += step;
				break;
				
			case left:
				sx -= step;
				break;
				
			case right:
				sx += step;
				break;
			}

			move_counter++;
			{
				viewport.x = initial_viewport.x + step * move_counter * dx;
				viewport.y = initial_viewport.y + std::min(step * move_counter, scroll_amount) * dy + playing_field_offset - old_original_playing_field_offset;
			}

			// This is the only thing that moves the hero.
			if (!sideview_scrolling_slope)
			{
				if (scrolling_dir == left || scrolling_dir == right)
				{
					x.doClamp(viewport.x, viewport.right() - 16);
				}
				else
				{
					y.doClamp(viewport.y, viewport.bottom() - 16);
				}
			}

			if (is_unsmooth_vertical_scrolling) viewport.y += 3;

			if(ladderx > 0 || laddery > 0)
			{
				// If the ladder moves on both axes, the player can
				// gradually shift it by going back and forth
				if(scrolldir==up || scrolldir==down)
					laddery = y.getInt();
				else
					ladderx = x.getInt();
			}
		}

		if (secondary_axis_alignment_amount)
		{
			int delta = (align_counter - abs(secondary_axis_alignment_amount)) * sign(secondary_axis_alignment_amount);
			if (scrolldir == up || scrolldir == down) viewport.x = initial_viewport.x + delta;
			else                                      viewport.y = initial_viewport.y + delta + (playing_field_offset - old_original_playing_field_offset);
		}

		FFCore.ScrollingData[SCROLLDATA_NX] = nx - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NY] = ny - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_OX] = ox - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_OY] = oy - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_NRX] = new_region_offset_x - viewport.x;
		FFCore.ScrollingData[SCROLLDATA_NRY] = new_region_offset_y - viewport.y;
		FFCore.ScrollingData[SCROLLDATA_ORX] = -viewport.x;
		FFCore.ScrollingData[SCROLLDATA_ORY] = -viewport.y;

		SAVE_HERO_POS;
		USE_COMPAT_HERO_POS;
		ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, true); //Waitdraw
		FFCore.runGenericPassiveEngine(SCR_TIMING_PRE_DRAW);
		RESTORE_HERO_POS;

		clear_bitmap(framebuf);
		clear_info_bmp();
		if (!classic_draw)
			for (int layer = -7; layer <= -4; ++layer)
				do_ffc_scroll_layer(framebuf, layer, nearby_screens, new_region_offset_x, new_region_offset_y);
		
		if(classic_draw) // -2 < -3
		{
			for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
				mapscr* base_scr = screen_handles[0].base_scr;
				if(XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)) do_layer(framebuf, 0, screen_handles[2], offx, offy);
			});
			if(XOR((newscr->flags7&fLAYER2BG) || (oldscr->flags7&fLAYER2BG), DMaps[cur_dmap].flags&dmfLAYER2BG))
				do_primitives(framebuf, 2);
			do_ffc_scroll_layer(framebuf, -2, nearby_screens, new_region_offset_x, new_region_offset_y);
		}
		
		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			mapscr* base_scr = screen_handles[0].base_scr;
			if(XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)) do_layer(framebuf, 0, screen_handles[3], offx, offy);
		});
		if(XOR((newscr->flags7&fLAYER3BG) || (oldscr->flags7&fLAYER3BG), DMaps[cur_dmap].flags&dmfLAYER3BG))
			do_primitives(framebuf, 3);
		do_ffc_scroll_layer(framebuf, -3, nearby_screens, new_region_offset_x, new_region_offset_y);
		
		if(!classic_draw) // -2 > -3
		{
			for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
				mapscr* base_scr = screen_handles[0].base_scr;
				if(XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)) do_layer(framebuf, 0, screen_handles[2], offx, offy);
			});
			if(XOR((newscr->flags7&fLAYER2BG) || (oldscr->flags7&fLAYER2BG), DMaps[cur_dmap].flags&dmfLAYER2BG))
				do_primitives(framebuf, 2);
			do_ffc_scroll_layer(framebuf, -2, nearby_screens, new_region_offset_x, new_region_offset_y);
			do_ffc_scroll_layer(framebuf, -1, nearby_screens, new_region_offset_x, new_region_offset_y);
		}
		
		
		combotile_add_y = is_unsmooth_vertical_scrolling ? -3 : 0;
		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			offy += playing_field_offset;
			if (lenscheck(screen_handles[0].scr, 0))
				putscr(screen_handles[0].scr, framebuf, offx, offy);
		});
		combotile_add_y = 0;

		if (lenscheck(newscr, 0))
			do_primitives(framebuf, 0);

		do_ffc_scroll_layer(framebuf, 0, nearby_screens, new_region_offset_x, new_region_offset_y);
		
		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			do_layer(framebuf, 0, screen_handles[1], offx, offy);
		});
		do_layer_primitives(framebuf, 1);
		
		do_ffc_scroll_layer(framebuf, 1, nearby_screens, new_region_offset_x, new_region_offset_y);

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			mapscr* base_scr = screen_handles[0].base_scr;
			if (!(XOR(base_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)))
				do_layer(framebuf, 0, screen_handles[2], offx, offy);
		});

		if (!(oldscr->flags7&fLAYER2BG) && !(XOR(origin_scr->flags7&fLAYER2BG, DMaps[cur_dmap].flags&dmfLAYER2BG)))
			do_layer_primitives(framebuf, 2);
		do_ffc_scroll_layer(framebuf, 2, nearby_screens, new_region_offset_x, new_region_offset_y);

		if (get_qr(qr_PUSHBLOCK_SPRITE_LAYER))
		{
			for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
				do_layer(framebuf, -2, screen_handles[0], offx, offy);
				if (get_qr(qr_PUSHBLOCK_LAYER_1_2))
				{
					do_layer(framebuf, -2, screen_handles[1], offx, offy);
					do_layer(framebuf, -2, screen_handles[2], offx, offy);
				}
			});

			do_primitives(framebuf, SPLAYER_PUSHBLOCK);
		}

		if (show_walkflags || show_effectflags)
		{
			for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
				do_walkflags(screen_handles, offx, offy); // show walkflags if the cheat is on
				do_effectflags(screen_handles[0].base_scr, offx, offy); // show effectflags if the cheat is on
			});

			do_walkflags(nx, ny);
		}

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			offy += playing_field_offset;
			putscrdoors(screen_handles[0].scr, framebuf, offx, offy);
		});

		if (!align_counter || scroll_counter) herostep();

		{
			auto prev_y = y;
			auto prev_yofs = yofs;

			if (is_unsmooth_vertical_scrolling) y += 3;
			yofs = playing_field_offset;

			if((z > 0 || fakez > 0) && (!get_qr(qr_SHADOWSFLICKER) || frame&1))
			{
				drawshadow(framebuf, get_qr(qr_TRANSSHADOWS) != 0);
			}

			if(!isdungeon() || get_qr(qr_FREEFORM))
			{
				for (int i = 0; i < decorations.Count(); i++)
				{
					auto sprite = decorations.spr(i);
					sprite->yofs += playing_field_offset - old_original_playing_field_offset;
				}

				draw_under(framebuf); //draw the ladder or raft
				decorations.draw2(framebuf, true);
				draw(framebuf); //Hero
				decorations.draw(framebuf,  true);

				for (int i = 0; i < decorations.Count(); i++)
				{
					auto sprite = decorations.spr(i);
					sprite->yofs -= playing_field_offset - old_original_playing_field_offset;
				}
			}

			y = prev_y;
			yofs = prev_yofs;
		}

		RESTORE_HERO_POS;

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			mapscr* base_scr = screen_handles[0].base_scr;
			if (!(XOR(base_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)))
				do_layer(framebuf, 0, screen_handles[3], offx, offy);
		});

		if (!(XOR(origin_scr->flags7&fLAYER3BG, DMaps[cur_dmap].flags&dmfLAYER3BG)))
			do_layer_primitives(framebuf, 3);
		do_ffc_scroll_layer(framebuf, 3, nearby_screens, new_region_offset_x, new_region_offset_y);

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			do_layer(framebuf, 0, screen_handles[4], offx, offy); //layer 4
		});

		do_layer_primitives(framebuf, 4);
		do_ffc_scroll_layer(framebuf, 4, nearby_screens, new_region_offset_x, new_region_offset_y);

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			do_layer(framebuf, -1, screen_handles[0], offx, offy); //overhead combos
			if (get_qr(qr_OVERHEAD_COMBOS_L1_L2))
			{
				do_layer(framebuf, -1, screen_handles[1], offx, offy); //overhead combos
				do_layer(framebuf, -1, screen_handles[2], offx, offy); //overhead combos
			}
		});

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			do_layer(framebuf, 0, screen_handles[5], offx, offy); //layer 5
		});

		do_layer_primitives(framebuf, 5);
		do_ffc_scroll_layer(framebuf, 5, nearby_screens, new_region_offset_x, new_region_offset_y);
		
		do_ffc_scroll_layer(framebuf, -1000, nearby_screens, new_region_offset_x, new_region_offset_y);

		for_every_nearby_screen_during_scroll(nearby_screens, [&](screen_handles_t screen_handles, int screen, int offx, int offy, bool is_new_screen) {
			do_layer(framebuf, 0, screen_handles[6], offx, offy); //layer 6
		});

		do_layer_primitives(framebuf, 6);
		do_ffc_scroll_layer(framebuf, 6, nearby_screens, new_region_offset_x, new_region_offset_y);

		if (draw_dark && get_qr(qr_NEW_DARKROOM) && get_qr(qr_NEWDARK_L6))
		{
			scrollscr_handle_dark(newscr, oldscr, nearby_screens);
		}

		put_passive_subscr(framebuf, 0, 0, game->should_show_time(), sspUP);

		if(get_qr(qr_SUBSCREENOVERSPRITES))
			do_primitives(framebuf, 7);
		do_ffc_scroll_layer(framebuf, 7, nearby_screens, new_region_offset_x, new_region_offset_y);
		
		if (draw_dark && get_qr(qr_NEW_DARKROOM) && !get_qr(qr_NEWDARK_L6))
		{
			scrollscr_handle_dark(newscr, oldscr, nearby_screens);
		}

		SAVE_HERO_POS;
		USE_COMPAT_HERO_POS;
		FFCore.runGenericPassiveEngine(SCR_TIMING_POST_DRAW);
		FFCore.runGenericPassiveEngine(SCR_TIMING_END_FRAME);

		RESTORE_HERO_POS;
		advanceframe(true/*,true,false*/);
		USE_COMPAT_HERO_POS;

		//Don't clear the last frame, unless 'fixed'
		if (scroll_counter > 0 || get_qr(qr_FIXSCRIPTSDURINGSCROLLING))
			script_drawing_commands.Clear();
		FFCore.runGenericPassiveEngine(SCR_TIMING_START_FRAME);
		actiontype lastaction = action;
		action=scrolling; FFCore.setHeroAction(scrolling);
		FFCore.runF6Engine();
		action=lastaction; FFCore.setHeroAction(lastaction);

		RESTORE_HERO_POS;
	}
	cur_dmap = old_dmap;

	// TODO(replays): Prior to z3 refactor, old scrolling code didn't clear the darkroom bitmaps at end of scroll,
	// so the first frame drawn had some lighting from previous screen... 
	// game_loop clears these bitmaps but that should be moved to advanceframe, and these other calls to `clear_darkroom_bitmaps`
	// deleted.
	if (draw_dark && !replay_is_active())
		clear_darkroom_bitmaps();

	clear_bitmap(msg_txt_display_buf);
	set_clip_state(msg_txt_display_buf, 1);
	clear_bitmap(msg_bg_display_buf);
	set_clip_state(msg_bg_display_buf, 1);
	clear_bitmap(msg_portrait_display_buf);
	set_clip_state(msg_portrait_display_buf, 1);

	viewport = new_viewport;
	playing_field_offset = new_playing_field_offset;
	x = new_hero_x;
	y = new_hero_y;
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
	if (lift_wpn)
		lift_wpn->yofs = playing_field_offset - 2;

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
	scrolling_using_new_region_coords = false;
	scrolling_destdmap = -1;
	memset(FFCore.ScrollingData, 0, sizeof(int32_t) * SZ_SCROLLDATA);
	FFCore.ScrollingData[SCROLLDATA_DIR] = -1;

	if (destdmap != -1)
	{
		bool changedlevel = false;
		bool changeddmap = false;
		if (cur_dmap != destdmap)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_DMAP);
			changeddmap = true;
		}
		if (DMaps[cur_dmap].level != DMaps[destdmap].level)
		{
			timeExitAllGenscript(GENSCR_ST_CHANGE_LEVEL);
			changedlevel = true;
		}
		dlevel = DMaps[destdmap].level;
		cur_dmap = destdmap;
		if (changeddmap)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_DMAP);
		}
		if (changedlevel)
		{
			throwGenScriptEvent(GENSCR_EVENT_CHANGE_LEVEL);
		}
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

	init_dmap();

	// Check for raft flags
	if((get_qr(qr_BROKEN_RAFT_SCROLL) || lastaction == rafting)
		&& action!=rafting && hopclk==0 && !walk_through_walls)
	{
		if(MAPFLAG(x,y)==mfRAFT||MAPCOMBOFLAG(x,y)==mfRAFT)
		{
			sfx(hero_scr->secretsfx);
			action=rafting; FFCore.setHeroAction(rafting);
			raftclk=0;
		}
		
		// Half a tile off?
		else if((dir==left || dir==right) && (MAPFLAG(x,y+8)==mfRAFT||MAPCOMBOFLAG(x,y+8)==mfRAFT))
		{
			sfx(hero_scr->secretsfx);
			action=rafting; FFCore.setHeroAction(rafting);
			raftclk=0;
		}
	}

	for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
		get_screen_state(scr->screen).open_doors = 0;
	});
	markBmap();

	if (isdungeon(current_screen))
	{
		switch(hero_scr->door[scrolldir^1])
		{
		case dNONE:
			dir = scrolldir;
			break;
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
				
			putdoor(hero_scr, scrollbuf, scrolldir^1, hero_scr->door[scrolldir^1]);
			get_screen_state(hero_scr->screen).open_doors = -4;
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
	
	if(newscr->flags2&fSECRET_SFX)
	{
		sfx(newscr->secretsfx);
	}
	
	if (updatemusic)
	{
		playLevelMusic();
		if (musicrevert)
			FFCore.music_update_cond = MUSIC_UPDATE_SCREEN;
	}

	// It happens that all decorations that are not drawn with an offset
	// are temporary sprites (like combo sprites) that should never carry over.
	for (int i = decorations.Count()-1; i >=0; i--)
	{
		auto sprite = (decoration*)decorations.spr(i);
		if (!sprite->is_drawn_with_offset())
			decorations.remove(sprite); // TODO: improve deletions by adding remove_by_index
		else if (old_region.region_id || new_region.region_id)
			sprite->yofs = playing_field_offset;
	}

	newscr_clk = frame;
	activated_timed_warp=false;
	loadside = scrolldir^1;
	FFCore.clear_combo_scripts();
	eventlog_mapflags();
	decorations.animate(); //continue to animate tall grass during scrolling
	if(get_qr(qr_FIXSCRIPTSDURINGSCROLLING))
	{
		if (old_dmap == new_dmap || (replay_version_check(0, 15)))
		{
			ZScriptVersion::RunScrollingScript(scrolldir, scroll_counter, sx, sy, end_frames, false); //Prewaitdraw
		}
	}

	// Bye!
	delete_temporary_screens(old_temporary_screens.data());
	FFCore.ScrollingScreensAll.clear();

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
			if(itemsbuf[itemid].flags & item_flag2)//"Divisor is Percentage Multiplier" flag
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
			if(itemsbuf[itemid].flags & item_flag2)//"Divisor is Percentage Multiplier" flag
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
			if(itemsbuf[itemid].flags & item_flag2)//"Divisor is Percentage Multiplier" flag
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
			if(itemsbuf[itemid].flags & item_flag2)//"Divisor is Percentage Multiplier" flag
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
    
    if(_walkflag(x+wx,y+wy,0,get_standing_z_state())) return true;
    
    if(dir==left || dir==right)
    {
        wx+=16;
        
        if(_walkflag(x+wx,y+wy,0,get_standing_z_state())) return true;
    }
    
    return false;
}

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
	return !Hero.BunnyClock() || (itemid > 0 && itemsbuf[itemid].flags&item_bunny_enabled);
}

bool usesSwordJinx(int32_t itemid)
{
	itemdata const& it = itemsbuf[itemid];
	bool ret = (it.type==itype_sword);
	if(it.flags & item_flip_jinx) return !ret;
	return ret;
}
bool checkitem_jinx(int32_t itemid)
{
	itemdata const& it = itemsbuf[itemid];
	if(it.flags & item_jinx_immune) return true;
	if (it.type == itype_shield){
		if(HeroShieldClk() != 0) return false;
		if(it.flags & !item_flag9) //Active Shields should also check sword/item jinx flags
		{
			if (usesSwordJinx(itemid)) return HeroSwordClk() == 0;
			return HeroItemClk() == 0;
		}
		return true;

	}
	if(usesSwordJinx(itemid)) return HeroSwordClk() == 0;
	return HeroItemClk() == 0;
}

void stopCaneOfByrna()
{
	for (int32_t i = 0; i < Lwpns.Count(); i++)
	{
		weapon* w = ((weapon*)Lwpns.spr(i));
		if (w->id == wCByrna && !w->weapon_dying_frame)
			w->dead = 1;
	}
}

//Check if there are no beams, kill sfx, and reset last_cane_of_byrna_item_id
void HeroClass::cleanupByrna()
{
	if ( last_cane_of_byrna_item_id > -1 )
	{
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
	if ((itype == getItemFamily(itemsbuf, Bwpn)) && getInput(btnB, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if ((itype == getItemFamily(itemsbuf, Awpn)) && getInput(btnA, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if ((itype == getItemFamily(itemsbuf, Xwpn)) && getInput(btnEx1, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if ((itype == getItemFamily(itemsbuf, Ywpn)) && getInput(btnEx2, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
    return false;
}

int32_t getWpnPressed(int32_t itype)
{
	if ((itype == getItemFamily(itemsbuf, Bwpn)) && getInput(btnB, INPUT_DRUNK | INPUT_HERO_ACTION)) return Bwpn & 0xFFF;
	if ((itype == getItemFamily(itemsbuf, Awpn)) && getInput(btnA, INPUT_DRUNK | INPUT_HERO_ACTION)) return Awpn & 0xFFF;
	if ((itype == getItemFamily(itemsbuf, Xwpn)) && getInput(btnEx1, INPUT_DRUNK | INPUT_HERO_ACTION)) return Xwpn & 0xFFF;
	if ((itype == getItemFamily(itemsbuf, Ywpn)) && getInput(btnEx2, INPUT_DRUNK | INPUT_HERO_ACTION)) return Ywpn & 0xFFF;
    
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
		if (getIntBtnInput(intbtn, INPUT_DRUNK | INPUT_PEEK))
			return jumpid; //not pressed
	}

	if ((itype_rocs == getItemFamily(itemsbuf, Bwpn)) && getInput(btnB, INPUT_DRUNK | INPUT_HERO_ACTION))
		return Bwpn;
	if ((itype_rocs == getItemFamily(itemsbuf, Awpn)) && getInput(btnA, INPUT_DRUNK | INPUT_HERO_ACTION))
		return Awpn;
	if ((itype_rocs == getItemFamily(itemsbuf, Xwpn)) && getInput(btnEx1, INPUT_DRUNK | INPUT_HERO_ACTION))
		return Xwpn;
	if ((itype_rocs == getItemFamily(itemsbuf, Ywpn)) && getInput(btnEx2, INPUT_DRUNK | INPUT_HERO_ACTION))
		return Ywpn;

	return -1;
}

bool isItmPressed(int32_t itmid)
{
	if (itmid == (NEG_OR_MASK(Bwpn, 0xFFF)) && getInput(btnB, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if (itmid == (NEG_OR_MASK(Awpn, 0xFFF)) && getInput(btnA, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if (itmid == (NEG_OR_MASK(Xwpn, 0xFFF)) && getInput(btnEx1, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
	if (itmid == (NEG_OR_MASK(Ywpn, 0xFFF)) && getInput(btnEx2, INPUT_DRUNK | INPUT_HERO_ACTION)) return true;
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

void dospecialmoney(mapscr* scr, int32_t index)
{
	if (cur_screen >= 128)
		scr = special_warp_return_scr;

    int32_t priceindex = ((item*)items.spr(index))->PriceIndex;

    switch (scr->room)
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
        donewmsg(scr, QMisc.info[scr->catchall].str[priceindex]);
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

        prices[0] = scr->catchall;
        if (!current_item_power(itype_wallet))
            game->change_drupy(prices[0]);
	//game->set_drupy(game->get_drupy()+price); may be needed everywhere

        putprices(false);
        setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
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
        if(game->get_spendable_rupies()<scr->catchall && !current_item_power(itype_wallet))
            return;
            
		int32_t price = -scr->catchall;
		int32_t wmedal = current_item_id(itype_wealthmedal);
		if(wmedal >= 0)
		{
			if(itemsbuf[wmedal].flags & item_flag1)
				price*=(itemsbuf[wmedal].misc1/100.0);
			else
				price+=itemsbuf[wmedal].misc1;
		}
		
		int32_t total = game->get_drupy()-price;
		total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
		game->set_drupy(game->get_drupy()-total);
        //game->set_drupy(game->get_drupy()+price);
        setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
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
            if(itemsbuf[i].type == itype_bomb && itemsbuf[i].level == 1)
                getitem(i, true, true);
        }
        
        ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        verifyBothWeapons();
        break;
	}
        
    case rARROWS:
	{
        if(game->get_spendable_rupies()<scr->catchall && !current_item_power(itype_wallet))
            return;
            
        int32_t price = -scr->catchall;
		int32_t wmedal = current_item_id(itype_wealthmedal);
		if(wmedal >= 0)
		{
			if(itemsbuf[wmedal].flags & item_flag1)
				price*=(itemsbuf[wmedal].misc1/100.0);
			else
				price+=itemsbuf[wmedal].misc1;
		}
		
	int32_t total = game->get_drupy()-price;
	total = vbound(total, 0, game->get_maxcounter(1)); //Never overflow! Overflow here causes subscreen bugs! -Z
	game->set_drupy(game->get_drupy()-total);

	//game->set_drupy(game->get_drupy()+price);
        setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
        game->change_maxarrows(10);
        game->set_arrows(game->get_maxarrows());
        ((item*)items.spr(index))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
        verifyBothWeapons();
        break;
	}
        
    case rSWINDLE:
        if(items.spr(index)->id==iRupy)
        {
            if(game->get_spendable_rupies()<scr->catchall && !current_item_power(itype_wallet))
                return;
	    int32_t tmpprice = -scr->catchall;
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
        
        setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
        ((item*)items.spr(0))->pickup=ipDUMMY+ipFADE;
        ((item*)items.spr(1))->pickup=ipDUMMY+ipFADE;
        fadeclk=66;
        dismissmsg();
        clear_bitmap(pricesdisplaybuf);
        set_clip_state(pricesdisplaybuf, 1);
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
			if((itemsbuf[id].flags & item_combine) && game->get_item(id))
				// Item upgrade routine.
			{
				
				for(int32_t i=0; i<MAXITEMS; i++)
				{
					// Find the item which is as close to this item's fam_type as possible.
					if(itemsbuf[i].type==itemsbuf[id].type && itemsbuf[i].level>itemsbuf[id].level
							&& (nextitem>-1 ? itemsbuf[i].level<=itemsbuf[nextitem].level : true))
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
		((idat.type == itype_triforcepiece) ? item_flag8 : item_gamedata);
	
	if(equipment)
	{
		// Fix boomerang sounds.
		int32_t itemid = current_item_id(idat.type);
		
		if(itemid>=0 && (idat.type == itype_brang || idat.type == itype_divineprotection
						 || idat.type == itype_hookshot || idat.type == itype_switchhook || idat.type == itype_cbyrna)
				&& sfx_allocated(itemsbuf[itemid].usesound)
				&& idat.usesound != itemsbuf[itemid].usesound)
		{
			stop_sfx(itemsbuf[itemid].usesound);
			cont_sfx(idat.usesound);
		}
		
		int32_t curitm = current_item_id(idat.type);
		if(!get_qr(qr_KEEPOLD_APPLIES_RETROACTIVELY)
			|| curitm < 0 || (itemsbuf[curitm].level <= idat.level)
			|| (itemsbuf[curitm].flags & item_keep_old))
		{
			game->set_item(id,true);
			passiveitem_script(id, doRunPassive);
		}
		
		if(!(idat.flags & item_keep_old))
		{
			if(!get_qr(qr_BROKEN_KEEPOLD_FLAG) || current_item(idat.type)<idat.level)
			{
				removeLowerLevelItemsOfFamily(game,itemsbuf,idat.type, idat.level);
			}
		}
		
		// NES consistency: replace all flying boomerangs with the current boomerang.
		if(idat.type==itype_brang)
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
			if(idat.type==itype_bombbag && idat.count==2 && (idat.flags&16)==0)
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
	
	if(idat.pickup_litems)
	{
		auto lvl = idat.pickup_litem_level;
		if(lvl < 0)
			lvl = dlevel;
		game->lvlitems[lvl] |= idat.pickup_litems;
	}
	
	if(idat.playsound&&!nosound)
	{
		sfx(idat.playsound);
	}
	
	//add lower-level items
	if(idat.flags&item_gain_old)
	{
		for(int32_t i=idat.level-1; i>0; i--)
		{
			int32_t potid = getItemID(itemsbuf, idat.type, i);
			
			if(potid != -1)
			{
				game->set_item(potid, true);
			}
		}
	}
	
	switch(idat.type)
	{
		case itype_itmbundle:
		{
			if(!itembundle_safe(id & 0xFF))
				break;
			int ids[10] = {idat.misc1, idat.misc2, idat.misc3, idat.misc4, idat.misc5,
				idat.misc6, idat.misc7, idat.misc8, idat.misc9, idat.misc10};
			bool pscript = (idat.flags & item_flag1);
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
			int32_t newid = get_progressive_item(id);
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
			if(idat.flags & item_flag1) //Active use, not passive
				break;
			if((idat.flags & item_flag2) && clockclk) //"Can't activate while clock active"
				break;
			setClock(watch=true);
			
			clock_zoras.clear();
				
			clockclk=idat.misc1;
			sfx(idat.usesound);
		}
		break;
		
		case itype_lkey:
		{
			auto lvl = dlevel;
			if(idat.flags & item_flag1) // custom level
				lvl = vbound(idat.misc1, 0, MAXLEVELS-1);
			if(game->lvlkeys[lvl]<255)
				game->lvlkeys[lvl]++;
			break;
		}
			
		case itype_ring:
		case itype_magicring:
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (cur_screen<128 || dlevel))
			{
				ringcolor(false);
			}
			break;
			
		case itype_whispring:
		{
			if(idat.flags & item_flag1)
			{
				if(HeroSwordClk()==-1) setSwordClk(150);  // Let's not bother applying the divisor.
				
				if(HeroItemClk()==-1) setItemClk(150);  // Let's not bother applying the divisor.

				if(HeroItemClk()== 1) setShieldClk(150);  // Let's not bother applying the divisor.
			}
			
			if(idat.power==0)
			{
				setSwordClk(0);
				setItemClk(0);
				setShieldClk(0);
			}
			
			break;
		}
		
		
		case itype_map:
			game->lvlitems[dlevel]|=(1 << li_map);
			break;
			
		case itype_compass:
			game->lvlitems[dlevel]|=(1 << li_compass);
			break;
			
		case itype_bosskey:
			game->lvlitems[dlevel]|=(1 << li_boss_key);
			break;
			
		case itype_fairy:
		
			game->set_life(zc_min(game->get_life()+(idat.flags&item_flag1 ?(int32_t)(game->get_maxlife()*(idat.misc1/100.0)):((idat.misc1*game->get_hp_per_heart()))),game->get_maxlife()));
			game->set_magic(zc_min(game->get_magic()+(idat.flags&item_flag2 ?(int32_t)(game->get_maxmagic()*(idat.misc2/100.0)):((idat.misc2*game->get_mp_per_block()))),game->get_maxmagic()));
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
			if(idat.flags & item_flag1) //Active use, not passive
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
    
    switch(itemsbuf[id&0xFF].type)
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
			if(!itembundle_safe(id))
				break;
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
			int32_t newid = get_progressive_item(id, true);
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
			game->lvlitems[dlevel]&=~(1 << li_map);
			break;
			
		case itype_compass:
			game->lvlitems[dlevel]&=~(1 << li_compass);
			break;
			
		case itype_bosskey:
			game->lvlitems[dlevel]&=~(1 << li_boss_key);
			break;
			
		case itype_lkey:
		{
			auto lvl = dlevel;
			if(idat.flags & item_flag1) // custom level
				lvl = vbound(idat.misc1, 0, MAXLEVELS-1);
			if(game->lvlkeys[lvl])
				game->lvlkeys[lvl]--;
			break;
		}
			
		case itype_ring:
			if((get_qr(qr_OVERWORLDTUNIC) != 0) || (cur_screen<128 || dlevel))
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

void HeroClass::handle_triforce(mapscr* scr, int32_t id)
{
	if(unsigned(id) >= MAXITEMS)
		return;
	itemdata const& itm = itemsbuf[id];
	switch(itm.type)
	{
		case itype_itmbundle:
		{
			if(!itembundle_safe(id, true)) // skip error because it will always have already printed earlier this frame in 'getitem'
				break;
			int ids[10] = {itm.misc1, itm.misc2, itm.misc3, itm.misc4, itm.misc5,
				itm.misc6, itm.misc7, itm.misc8, itm.misc9, itm.misc10};
			for(auto q = 0; q < 10; ++q)
			{
				if(unsigned(ids[q]) >= MAXITEMS) continue;
				handle_triforce(scr, ids[q]);
			}
		}
		break;
		case itype_triforcepiece:
		{
			if(itm.misc2>0)
				getTriforce(id); //small
			else getBigTri(scr, id); //big
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
		if(diagonalMovement || NO_GRIDLOCK)
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
	// `screen_spawned` is probably same as `heroscr`, but could not be if the item moved around.
	int32_t item_screen = ptr->screen_spawned;
	mapscr* item_scr = get_scr_maybe(cur_map, item_screen);

	// I haven't observed this happening, but there are crash reports showing that an
	// item sprite does not have a screen_spawned for the current screens.
	// Only guess is that `set_forcegrab` kept the item around from the previous screen.
	// https://zeldaclassic.sentry.io/share/issue/c13f5a3c5079463fb6901cfea26c695a/
	if (!item_scr)
	{
		item_screen = get_screen_for_world_xy(ptr->x, ptr->y);
		item_scr = get_scr(item_screen);
	}

	// For items grabbed while in a special screen.
	if (cur_screen >= 128)
		item_scr = special_warp_return_scr;

	if(ptr->fallclk > 0) return; //Don't pick up a falling item
	
	if(itemsbuf[id2].type == itype_progressive_itm)
	{
		int32_t newid = get_progressive_item(id2);
		if(newid > -1)
		{
			id2 = newid;
			holdid = newid;
			pstr = itemsbuf[newid].pstring;
			pstr_flags = itemsbuf[newid].pickup_string_flags;
		}
	}
	
	bool bottledummy = (pickup&ipCHECK) && item_scr->room == rBOTTLESHOP;
	
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
		
		for(int32_t i=0; i<items.Count(); i++)
		{
			if(((item*)items.spr(i))->PriceIndex >-1 && i!=index)
				((item*)items.spr(i))->pickup=ipDUMMY+ipFADE;
		}
		
		int32_t slot = game->fillBottle(QMisc.bottle_shop_types[item_scr->catchall].fill[PriceIndex]);
		id2 = find_bottle_for_slot(slot);
		ptr->id = id2;
		holdid = id2;
		pstr = 0;
		pickup |= ipHOLDUP;
	}
	else
	{
		if(itemsbuf[id2].type == itype_bottlefill && !game->canFillBottle())
			return; //No picking these up unless you have a bottle to fill!
		
		if(((pickup&ipTIMER) && (ptr->clk2 < game->get_item_spawn_flicker()))&& !(pickup & ipCANGRAB))
			if(ptr->id!=iFairyMoving)
				// wait for it to stop flashing, doesn't check for other items yet
				return;
				
		if(pickup&ipENEMY)                                        // item was being carried by enemy
			if(more_carried_items(item_screen)<=1)  // 1 includes this own item.
				screen_item_set_state(item_screen, ScreenItemState::None);
				
		if(pickup&ipDUMMY)                                        // dummy item (usually a rupee)
		{
			if(pickup&ipMONEY)
				dospecialmoney(item_scr, index);
				
			return;
		}
		
		if(get_qr(qr_HEARTSREQUIREDFIX) && !canget(id2))
			return;
		
		int32_t nextitem = -1;
		do
		{
			nextitem = -1;
			if((itemsbuf[id2].flags & item_combine) && game->get_item(id2))
				// Item upgrade routine.
			{
				
				for(int32_t i=0; i<MAXITEMS; i++)
				{
					// Find the item which is as close to this item's fam_type as possible.
					if(itemsbuf[i].type==itemsbuf[id2].type && itemsbuf[i].level>itemsbuf[id2].level
							&& (nextitem>-1 ? itemsbuf[i].level<=itemsbuf[nextitem].level : true))
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
			switch(item_scr->room)
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
					if(QMisc.shop[item_scr->catchall].hasitem[i] != 0)
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
		
		//EVENT
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
		}
		
		if(pickup&ipONETIME)    // set mITEM for one-time-only items
		{
			setmapflag(item_scr, mITEM);

			//Okay so having old source files is a godsend. You wanna know why?
			//Because the issue here was never to so with the wrong flag being set; no it's always been setting the right flag.
			//The problem here is that guy rooms were always checking for getmapflag, which used to have an internal check for the default.
			//The default would be mITEM if cur_screen was under 128 (AKA not in a cave), and mSPECIALITEM if in a cave.
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
				if(origin_scr->room==rGRUMBLE)
				{
					setmapflag(mSPECIALITEM);
				}
			}
			*/
		}
		else if(pickup&ipONETIME2)                                // set mSPECIALITEM flag for other one-time-only items
			setmapflag(item_scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
		
		if(exstate > -1 && exstate < 32)
		{
			setxmapflag(item_screen, 1<<exstate);
		}

		if(pickup&ipSECRETS)                                // Trigger secrets if this item has the secret pickup
		{
			if (item_scr->flags9&fITEMSECRETPERM) setmapflag(item_scr, mSECRET);
			trigger_secrets_for_screen(TriggerSource::ItemsSecret, item_scr, false);
		}

		collectitem_script(id2);
		getitem(id2, ptr->noSound, true);
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
		
		if(get_qr(qr_OLDPICKUP) || ((item_scr->room==rSP_ITEM || item_scr->room==rRP_HC || item_scr->room==rTAKEONE) && (pickup&ipONETIME2)) || 
		(get_qr(qr_SHOP_ITEMS_VANISH) && (item_scr->room==rBOTTLESHOP || item_scr->room==rSHOP) && (pickup&ipCHECK)))
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
				switch(item_scr->room)
				{
					case rSHOP:
						shop_pstr = QMisc.shop[item_scr->catchall].str[PriceIndex];
						break;
					case rBOTTLESHOP:
						shop_pstr = QMisc.bottle_shop_types[item_scr->catchall].str[PriceIndex];
						break;
				}
			}
			if ( (pstr > 0 && pstr < msg_count) || (shop_pstr > 0 && shop_pstr < msg_count) )
			{
				if ( (pstr > 0 && pstr < msg_count) && ( ( ( (pstr_flags&itemdataPSTRING_ALWAYS) || (pstr_flags&itemdataPSTRING_NOMARK) || ((pstr_flags&itemdataPSTRING_IP_HOLDUP) && replay_version_check(0, 44)) || (!(FFCore.GetItemMessagePlayed(id2)))  ) ) ) )
				{
					if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
				}
				else pstr = 0;
				if(shop_pstr)
				{
					donewmsg(item_scr, shop_pstr);
					enqueued_str = pstr;
				}
				else if(pstr)
				{
					donewmsg(item_scr, pstr);
				}
			}
			
		}
		
		if(itemsbuf[id2].type!=itype_triforcepiece || !(itemsbuf[id2].flags & item_gamedata))
		{
			if (!ptr->noHoldSound) sfx(item_scr->holdupsfx);
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
		int32_t shop_pstr = ( item_scr->room == rSHOP && PriceIndex>=0 && QMisc.shop[item_scr->catchall].str[PriceIndex] > 0 ) ? QMisc.shop[item_scr->catchall].str[PriceIndex] : 0;
		if ( (pstr > 0 && pstr < msg_count) || (shop_pstr > 0 && shop_pstr < msg_count) )
		{
			if ( (pstr > 0 && pstr < msg_count) && ( (!(pstr_flags&itemdataPSTRING_IP_HOLDUP)) && ( pstr_flags&itemdataPSTRING_NOMARK || pstr_flags&itemdataPSTRING_ALWAYS || (!(FFCore.GetItemMessagePlayed(id2))) ) ) )
			{
				if ( (!(pstr_flags&itemdataPSTRING_NOMARK)) ) FFCore.SetItemMessagePlayed(id2);
			}
			else pstr = 0;
			if(shop_pstr)
			{
				donewmsg(item_scr, shop_pstr);
				enqueued_str = pstr;
			}
			else if(pstr)
			{
				donewmsg(item_scr, pstr);
			}
		}
		
		
		clear_bitmap(pricesdisplaybuf);
		set_clip_state(pricesdisplaybuf, 1);
	}
	
	if(itemsbuf[id2].type==itype_triforcepiece
		&& itemsbuf[id2].misc2 <= 0 && linked_parent == eeGANON)
	{
		game->lvlitems[dlevel]|=(1 << li_boss_killed);
	}
	handle_triforce(item_scr, id2);
	if(!holdclk)
		post_item_collect();
}

void HeroClass::StartRefill(int32_t refillWhat)
{
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
		return;
	}

	if(!refilling)
	{
		refillclk=21;
		stop_sfx(QMisc.miscsfx[sfxLOWHEART]);
		sfx(WAV_REFILL,128,true);
		refilling=refillWhat;

		if(refill_why>=0) // Item index
		{
			if(itemsbuf[refill_why].type==itype_potion)
			{
				if(itemsbuf[refill_why].flags & item_flag3){swordclk=0;verifyAWpn();}
				if(itemsbuf[refill_why].flags & item_flag4)itemclk=0;
				if(itemsbuf[refill_why].flags & item_flag5)shieldjinxclk=0;
			}
			else if(itemsbuf[refill_why].type==itype_triforcepiece)
			{
				if(itemsbuf[refill_why].flags & item_flag3){swordclk=0;verifyAWpn();}
				if(itemsbuf[refill_why].flags & item_flag4)itemclk=0;
				if(itemsbuf[refill_why].flags & item_flag5)shieldjinxclk=0;
			}
		}
		else if(refill_why==REFILL_FAIRY)
		{
			if(!get_qr(qr_NONBUBBLEFAIRIES)){swordclk=0;verifyAWpn();}
			if(get_qr(qr_ITEMBUBBLE))itemclk=0;
			if(get_qr(qr_SHIELDBUBBLE))shieldjinxclk=0;
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

		// For ~2 years <2.55 quests were broken, never restoring anything on fairies.
		// https://discord.com/channels/876899628556091432/1275805052006563861
		if (replay_is_active() && replay_version_check(0, 35))
		{
			std::string qst = replay_get_meta_str("qst");
			if (qst == "first_quest_layered.qst" || qst == "nes_remastered.qst" || qst == "demosp253.qst")
				return;
		}

		if(refill_why>=0) // Item index
		{
			if((itemsbuf[refill_why].type==itype_potion)&&(!get_qr(qr_NONBUBBLEMEDICINE)))
			{
				swordclk=0;
				verifyAWpn();
				if(get_qr(qr_ITEMBUBBLE)) itemclk=0;
			}

			if((itemsbuf[refill_why].type==itype_triforcepiece)&&(!get_qr(qr_NONBUBBLETRIFORCE)))
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
    
    if(refill_why>=0 && itemsbuf[refill_why].type==itype_potion)
    {
        refill_heart_stop=zc_min(potion_life+(itemsbuf[refill_why].flags & item_flag1 ?int32_t(game->get_maxlife()*(itemsbuf[refill_why].misc1 /100.0)):((itemsbuf[refill_why].misc1 *game->get_hp_per_heart()))),game->get_maxlife());
        refill_magic_stop=zc_min(potion_magic+(itemsbuf[refill_why].flags & item_flag2 ?int32_t(game->get_maxmagic()*(itemsbuf[refill_why].misc2 /100.0)):((itemsbuf[refill_why].misc2 *game->get_mp_per_block()))),game->get_maxmagic());
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
					if ( q == (int32_t)hero_scr->oceansfx ) continue;
					if ( q == (int32_t)hero_scr->bosssfx ) continue;
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
					if ( q == (int32_t)hero_scr->oceansfx ) continue;
					if ( q == (int32_t)hero_scr->bosssfx ) continue;
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
					if ( q == (int32_t)hero_scr->oceansfx ) continue;
					if ( q == (int32_t)hero_scr->bosssfx ) continue;
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
		flash_pal[i] = get_qr(qr_FADE) ? _RGB(255,255,0) : _RGB(255,255,255); 
	}



	//get rid off all sprites but Hero
	guys.clear();
	items.clear();
	Ewpns.clear();
	Lwpns.clear();
	chainlinks.clear();
    
	kill_subscr_items();
    
	sfx(itemsbuf[id2].playsound);
	if ( !(itemsbuf[id2].flags & item_flag11) ) music_stop();
	
	//If item flag six is enabled, and a sound is set to attributes[2], play that sound.
	if ( (itemsbuf[id2].flags & item_flag14) )
	{
		uint8_t playwav = itemsbuf[id2].misc3;
		sfx(playwav);
		
	}
		
	//itemsbuf[id2].flags & item_flag9 : Don't dismiss Messages
	//itemsbuf[id2].flags & item_flag10 : Cutscene interrupts action script..
	//itemsbuf[id2].flags & item_flag11 : Don't change music.
	//itemsbuf[id2].flags & item_flag12 : Run Collect Script Script On Collection
	//itemsbuf[id2].flags & item_flag13 : Run Action Script On Collection
	//itemsbuf[id2].flags & item_flag14 : Play second sound (WAV) from Attributes[2] (misc2)
	//itemsbuf[id2].flags & item_flag15 : No MIDI
    
	if(!(itemsbuf[id2].flags & item_flag15)) //No MIDI flag
	{
		if(itemsbuf[id2].misc1)
			jukebox(itemsbuf[id2].misc1+ZC_MIDI_COUNT-1);
		else
			try_zcmusic("zelda.nsf",qstpath,5,ZC_MIDI_TRIFORCE,get_emusic_volume());
	}
	if(itemsbuf[id2].flags & item_gamedata)
	{
		game->lvlitems[dlevel]|=(1 << li_mcguffin);
	}
    
	int32_t f=0;
	int32_t x2=0;
	int32_t curtain_x=0;
	int32_t c=0;

	do
	{
		
		
		if ( (itemsbuf[id2].flags & item_flag13) ) //Run action script on collection.
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
					if ( !(itemsbuf[id2].flags & item_flag10) ) //Cutscene halts the script it resumes after cutscene.
						ZScriptVersion::RunScript(ScriptType::Item, itemsbuf[id2].script, id2); //if flag is off, run the script every frame of the cutscene.
				}
			}
		}
		//if ( itemsbuf[id2].misc2 == 2 ) //No cutscene; what if people used '2' on older quests?
		if ( (itemsbuf[id2].flags & item_flag12) ) //No cutscene
		{
			return;
		}

		if(!COOLSCROLL)
		{
			show_subscreen_items=false;
		}

		if(f==40)
		{
			actiontype oldaction = action;
			ALLOFF((!(itemsbuf[id2].flags & item_flag9)), false);
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
			    
			    if(cur_screen<128) loadlvlpal(DMaps[cur_dmap].color);
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
			    if(cur_screen<128) loadlvlpal(DMaps[cur_dmap].color);
			    else loadlvlpal(0xB);
			    
			    loadpalset(5,5);
			}
		    }
		}

	
		if(itemsbuf[id2].flags & item_gamedata)
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
	
		if(itemsbuf[id2].flags & item_flag1) // Warp out flag
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
				curtain_x=TRUNCATE_HALF_TILE(x2);
				draw_screen_clip_rect_x1=curtain_x;
				draw_screen_clip_rect_x2=255-curtain_x;
				draw_screen_clip_rect_y1=0;
				draw_screen_clip_rect_y2=231;
			}
		}
	
		draw_screen();
		
		//Run Triforce Script
		advanceframe(true);
		++f;
	}
	while
	(
		(f < ( (itemsbuf[id2].misc4 > 0) ? itemsbuf[id2].misc4 : 408)) 
		|| (!(itemsbuf[id2].flags & item_flag15) /*&& !(itemsbuf[id2].flags & item_flag11)*/ && (midi_pos > 0 && !replay_is_active())) 
		|| (/*!(itemsbuf[id2].flags & item_flag15) &&*/ !(itemsbuf[id2].flags & item_flag11) && (zcmusic!=NULL) && (zcmusic->position<800 && !replay_is_active())
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
	draw_screen_clip_rect_y2=231;
	show_subscreen_items=true;
    
	//Warp Hero out of item cellars, in 2.10 and earlier quests. -Z ( 16th January, 2019 )
	//Added a QR for this, to Other->2, as `Triforce in Cellar Warps Hero Out`. -Z 15th March, 2019 
	if((itemsbuf[id2].flags & item_flag1) && ( get_qr(qr_SIDEVIEWTRIFORCECELLAR) ? ( cur_screen < MAPSCRS192b136 ) : (cur_screen < MAPSCRSNORMAL) ) )
	{
		sdir=dir;
		dowarp(hero_scr, 1, 0); //side warp
	}
	else
	{
		if ( !(itemsbuf[id2].flags & item_flag11) ) playLevelMusic();
	}
}

void red_shift()
{
    int32_t tnum=176;
    
    // set up the new palette
    for(int32_t i=CSET(2); i < CSET(4); i++)
    {
        int32_t r = (i-CSET(2)) << 1;
        RAMpal[i+tnum].r = _rgb_scale_6[r];
        RAMpal[i+tnum].g = _rgb_scale_6[r >> 3];
        RAMpal[i+tnum].b = _rgb_scale_6[r >> 4];
    }
    
    // color scale the game screen
    for(int32_t y=0; y<viewport.visible_height(show_bottom_8px); y++)
    {
        for(int32_t x=0; x<framebuf->w; x++)
        {
            int c = framebuf->line[y+original_playing_field_offset][x];
			int r = RAMpal[c].r / 4;
			int g = RAMpal[c].g / 4;
			int b = RAMpal[c].b / 4;
            int v = zc_min(int32_t(r*0.4 + g*0.6 + b*0.4)>>1,31);
			putpixel(framebuf, x, y + original_playing_field_offset, c ? (v + tnum+CSET(2)) : 0);
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
				scriptcoldet = true;
			    
				for(int32_t i=0; i<32; i++) miscellaneous[i] = 0;
			    
				
				playing_field_offset = original_playing_field_offset; // otherwise, red_shift() may go past the bottom of the screen
				quakeclk=wavy=0;
			    
				//in original Z1, Hero marker vanishes at death.
				//code in subscr.cpp, put_passive_subscr checks the following value.
				//color 255 is a GUI color, so quest makers shouldn't be using this value.
				//Also, subscreen is static after death in Z1.
				int32_t tmp_hero_dot = QMisc.colors.hero_dot;
				QMisc.colors.hero_dot = 255;
				
				put_passive_subscr(subscrbmp, 0, 0, game->should_show_time(), sspUP);
				//Don't forget passive subscreen scripts!
				if(get_qr(qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN))
				{
					script_drawing_commands.Clear(); //We only want draws from this script
					if(DMaps[cur_dmap].passive_sub_script != 0)
						ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
					if (FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) && DMaps[cur_dmap].passive_sub_script != 0 && FFCore.doscript(ScriptType::ScriptedPassiveSubscreen))
					{
						ZScriptVersion::RunScript(ScriptType::ScriptedPassiveSubscreen, DMaps[cur_dmap].passive_sub_script, cur_dmap);
						FFCore.waitdraw(ScriptType::ScriptedPassiveSubscreen) = false;
					}
					BITMAP* tmp = framebuf;
					framebuf = subscrbmp; //Hack; force draws to subscrbmp
					do_script_draws(framebuf, origin_scr, 0, playing_field_offset); //Draw the script draws
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
				for_every_base_screen_in_region([&](mapscr* scr, unsigned int region_scr_x, unsigned int region_scr_y) {
					if(f==58)
					{
						for(int32_t i = 0; i < 96; i++)
							scr->cset[i] = 3;
							
						for(int32_t j=1; j<=6; j++)
							if(scr->layermap[j-1]>0)
							{
								mapscr* lyr_scr = get_scr_layer(scr->screen, j);
								for(int32_t i=0; i<96; i++)
									lyr_scr->cset[i] = 3;
							}
					}
					
					if(f==59)
					{
						for(int32_t i = 96; i < 176; i++)
							scr->cset[i] = 3;
							
						for(int32_t j=1; j<=6; j++)
							if(scr->layermap[j-1]>0)
							{
								mapscr* lyr_scr = get_scr_layer(scr->screen, j);
								for(int32_t i=96; i<176; i++)
									lyr_scr->cset[i] = 3;
							}
					}
					
					if(f==60)
					{
						for(int32_t i=0; i<176; i++)
						{
							scr->cset[i] = 2;
						}
						
						for(int32_t j=1; j<=6; j++)
							if(scr->layermap[j-1]>0)
							{
								mapscr* lyr_scr = get_scr_layer(scr->screen, j);
								for(int32_t i=0; i<176; i++)
									lyr_scr->cset[i] = 2;
							}
					}
				});

				if(f==60)
				{
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
					rectfill(framebuf,0,playing_field_offset,framebuf->w,framebuf->h,0);
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
				clear_info_bmp();
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
			sfx(getHurtSFX(),pan(x));
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
	mapscr* scr = hero_scr;
	int screen = scr->screen;
	auto [offx, offy] = translate_screen_coordinates_to_world(screen);

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
    loaditem(scr, offx, offy);
    
    if(game->lvlitems[dlevel]&(1 << li_boss_killed))
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
                if(guysbuf[i].flags&guy_ganon)
                {
                    Id=i;
                    break;
                }
            }
            
            if(current_item(itype_ring))
            {
                addenemy(screen,offx+160,offy+96,Id,0);
            }
            else
            {
                addenemy(screen,offx+80,offy+32,Id,0);
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

    if((!getmapflag(screen, mSPECIALITEM) || (scr->flags9&fBELOWRETURN)) && (tunes[MAXMIDIS-1].data))
        jukebox(MAXMIDIS-1);
    else
        playLevelMusic();
        
    currcset=DMaps[cur_dmap].color;
    if (get_qr(qr_GANONINTRO) ) 
    {
	dointro();
	//Yes, I checked. This is literally in 2.10 (minus this if statement of course).
	//I have no clue why it's here; Literally the only difference between dointro in 2.10 and dointro in this version is an 'else' that sets introclk and intropos to 74.
	//I have no idea what was going through the original devs heads and I'm extremely worried I'm missing something, cause at first glance this looks like 
	//a hack solution to an underlying bug, but no! There's just a fucking dointro() call in older versions and I don't know *why*. -Deedee
    }

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
    y = (isdungeon() && cur_screen<128) ? 75 : 73;
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

void HeroClass::set_dive(int32_t newdive)
{
	if (newdive > 0)
	{
		auto flippers_id = current_item_id(itype_flippers);
		zfix targ_z = -1;
		if (flippers_id >= 0)
			targ_z = -(itemsbuf[flippers_id].level);
		if (standing_on_z > targ_z)
			standing_on_z = targ_z;
	}
	else
	{
		if (standing_on_z < 0)
			standing_on_z = 0;
	}
	diveclk = newdive;
}


bool HeroClass::can_deploy_ladder()
{
    bool ladderallowed = ((!get_qr(qr_LADDERANYWHERE) && (hero_scr->flags&fLADDER)) || isdungeon()
                          || (get_qr(qr_LADDERANYWHERE) && !(hero_scr->flags&fLADDER)));
    return (current_item_id(itype_ladder)>-1 && ladderallowed && !ilswim && z==0 && fakez==0 && !hoverclk &&
            (!isSideViewHero() || on_sideview_solid_oldpos(this)));
}

void HeroClass::reset_ladder()
{
	if (ladderx || laddery)
	{
		ladderx = laddery = 0;

		if (is_autowalking())
		{
			if (check_pitslide() != -1)
				try_hover();
		}
	}
}

bool is_conveyor(int32_t type);
int32_t get_conveyor(int32_t x, int32_t y);

void HeroClass::check_conveyor()
{
	++newconveyorclk;

	if (is_conveyor_stunned > 0)
		--is_conveyor_stunned;
	if (is_on_conveyor > 0)
	{
		if(!--is_on_conveyor)
			conv_forcedir = -1;
	}
	
	if((!get_qr(qr_BROKEN_CONVEYORS) && action==rafting) || action==casting||action==sideswimcasting||action==drowning || action==sidedrowning||action==lavadrowning||inlikelike||pull_hero||is_autowalking())
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
		if (conveyclk <= 0)
		{
			if(is_on_conveyor < 0)
			{
				is_on_conveyor = 0;
				conv_forcedir = -1;
				is_conveyor_stunned = 0;
			}
		}
		return;
	}
	newcombo const* cmb = &combobuf[cmbid];
	rpos_t rpos = COMBOPOS_REGION(x+7,y+(bigHitbox?8:12));
	bool custom_spd = (cmb->usrflags&cflag2);
	if((z>0||fakez>0) && !((hero_scr->flags2&fAIRCOMBOS)||(cmb->usrflags&cflag7)))
	{
		is_on_conveyor = 0;
		conv_forcedir = -1;
		is_conveyor_stunned = 0;
		return;
	}
	if(custom_spd || conveyclk<=0) //!DIMITODO: let player be on multiple conveyors at once
	{
		int32_t ctype=cmb->type;
		auto rate = custom_spd ? zc_max(cmb->attribytes[0], 1) : 3;
		if(custom_spd && (newconveyorclk % rate)) return;
		if((cmb->usrflags&cflag5) && HasHeavyBoots())
			return;
		is_on_conveyor = 0;
		conv_forcedir = -1;
		is_conveyor_stunned = 0;
		
		deltax=combo_class_buf[ctype].conveyor_x_speed;
		deltay=combo_class_buf[ctype].conveyor_y_speed;
		
		if (is_conveyor(ctype) && custom_spd)
		{
			deltax = zslongToFix(cmb->attributes[0]);
			deltay = zslongToFix(cmb->attributes[1]);
		}
		
		if((deltax==0&&deltay==0)&&(isSideViewHero() && on_sideview_solid_oldpos(this)))
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
			is_on_conveyor = custom_spd ? rate : -1;
		}
		else return;
		
		bool forcewalk = (cmb->usrflags&cflag6) && get_qr(qr_NEW_HERO_MOVEMENT2);
		if(forcewalk)
		{
			is_conveyor_stunned = rate;
			if(cmb->usrflags&cflag3)
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
						
						if ((getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != left && dir != right && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != left && dir != right && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != up && dir != down && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != up && dir != down && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != left && dir != right && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnRight, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnLeft, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != left && dir != right && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != up && dir != down && !(diagonalMovement || NO_GRIDLOCK))
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
						
						if ((getInput(btnUp, INPUT_DRUNK | INPUT_HERO_ACTION) || getInput(btnDown, INPUT_DRUNK | INPUT_HERO_ACTION)) && dir != up && dir != down && !(diagonalMovement || NO_GRIDLOCK))
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
				if(cmb->usrflags&cflag3)
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
        // TODO(crash): check that .add succeeds.
        decorations.add(new dDivineProtectionShield(HeroX(), HeroY(), dDIVINEPROTECTIONSHIELD, 0));
        decorations.spr(decorations.Count()-1)->misc=0;
        decorations.add(new dDivineProtectionShield(HeroX(), HeroY(), dDIVINEPROTECTIONSHIELD, 0));
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
	if (down)
	{
		if (check_new_slope(x, y + 0.0001_zf, 16, 16, old_x, old_y, true))
		{
			return false;
		}
	}
	if(down && get_qr(qr_DOWN_DOESNT_GRAB_LADDERS))
	{
		bool onSolid = on_sideview_solid_oldpos(this,true);
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
		bool onSolid = on_sideview_solid_oldpos(this,true);
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
                            particles.add(new pTwilight(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 6, 0, 0, (zc_oldrand()%8)+i*4));
                            int32_t k=particles.Count()-1;
                            particle *p = (particles.at(k));
                            p->step=3;
                        }
                        else if(type ==1)  // Sands of Hours
                        {
                            particles.add(new pTwilight(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 6, 1, 2, (zc_oldrand()%16)+i*2));
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
                            particles.add(new pDivineEscapeDust(Hero.getX()+j, Hero.getY()-Hero.getZ()+i, 6, 6, herotilebuf[i*16+j], zc_oldrand()%96));
                            
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
	return isSideViewHero() && (moveflags & move_obeys_grav) && !walk_through_walls;
}
bool HeroClass::is_unpushable() const
{
	return walk_through_walls;
}

HeroClass Hero;
