#include "combo.h"
#include "general.h"
#include "mapscr.h"

std::vector<newcombo> combobuf;

bool combo_trigger::is_blank() const
{
	return *this == combo_trigger();
}

void combo_trigger::clear()
{
	*this = combo_trigger();
}

void newcombo::set_tile(int32_t newtile)
{
	o_tile = newtile;
	tile = newtile;
}

void newcombo::clear()
{
	*this = newcombo();
}
	
bool newcombo::is_blank(bool ignoreEff) const
{
	if(tile) return false;
	if(flip) return false;
	if(walk&0xF) return false;
	if(!ignoreEff && (walk&0xF0)!=0xF0) return false;
	if(type) return false;
	if(csets) return false;
	if(frames) return false;
	if(speed) return false;
	if(nextcombo) return false;
	if(nextcset) return false;
	if(flag) return false;
	if(skipanim) return false;
	if(nexttimer) return false;
	if(skipanimy) return false;
	if(animflags) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
		if(attributes[q]) return false;
	if(usrflags) return false;
	if(genflags) return false;
	if(!label.empty()) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
		if(attribytes[q]) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
		if(attrishorts[q]) return false;
	if(script) return false;
	for(auto q = 0; q < 8; ++q)
		if(initd[q]) return false;
	if(o_tile) return false;
	if(cur_frame) return false;
	if(aclk) return false;
	
	if(liftcmb) return false;
	if(liftundercmb) return false;
	if(liftcs) return false;
	if(liftundercs) return false;
	if(liftdmg) return false;
	if(liftlvl) return false;
	if(liftitm) return false;
	if(liftflags) return false;
	if(liftgfx) return false;
	if(liftsprite) return false;
	if(liftsfx) return false;
	if(liftbreaksprite != -1) return false;
	if(liftbreaksfx) return false;
	if(lifthei != 8) return false;
	if(lifttime != 16) return false;
	if(lift_parent_item) return false;
	
	if(speed_mult != 1) return false;
	if(speed_div != 1) return false;
	if(speed_add) return false;
	
	if(sfx_appear) return false;
	if(sfx_disappear) return false;
	if(sfx_loop) return false;
	if(sfx_walking) return false;
	if(sfx_standing) return false;
	if(sfx_tap) return false;
	if(sfx_landing) return false;
	
	if(spr_appear) return false;
	if(spr_disappear) return false;
	if(spr_walking) return false;
	if(spr_standing) return false;
	if(spr_falling) return false;
	if(spr_drowning) return false;
	if(spr_lava_drowning) return false;
	if(sfx_falling) return false;
	if(sfx_drowning) return false;
	if(sfx_lava_drowning) return false;
	
	if(!triggers.empty()) return false;
	if(!misc_weap_data.is_blank()) return false;
	if(!lift_weap_data.is_blank()) return false;
	return true;
}

bool newcombo::can_cycle() const
{
	if(nextcombo != 0)
		return true;
	if(animflags & AF_CYCLEUNDERCOMBO)
		return true;
	return false;
}

int newcombo::each_tile(std::function<bool(int32_t)> proc) const
{
	int tile = o_tile;
	int frame = 0;
	do
	{
		if(proc(tile))
			return frame;
		if(++frame >= frames)
			break;
		tile += ((1+skipanim)*frame);
		if(int rowoffset = TILEROW(tile)-TILEROW(o_tile))
			tile += skipanimy * rowoffset * TILES_PER_ROW;
	}
	while(true);
	return -1;
}

void newcombo::advpaste(newcombo const& other, bitstring const& flags)
{
	if(flags.get(CMB_ADVP_TILE))
	{
		tile = other.tile;
		o_tile = other.o_tile;
		flip = other.flip;
	}
	if(flags.get(CMB_ADVP_CSET2))
		csets = other.csets;
	if(flags.get(CMB_ADVP_SOLIDITY))
		walk = (walk&0xF0) | (other.walk&0x0F);
	if(flags.get(CMB_ADVP_ANIM))
	{
		frames = other.frames;
		speed = other.speed;
		nextcombo = other.nextcombo;
		nextcset = other.nextcset;
		skipanim = other.skipanim;
		nexttimer = other.nexttimer;
		skipanimy = other.skipanimy;
		animflags = other.animflags;
	}
	if(flags.get(CMB_ADVP_TYPE))
		type = other.type;
	if(flags.get(CMB_ADVP_INHFLAG))
		flag = other.flag;
	if(flags.get(CMB_ADVP_ATTRIBUTE))
	{
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
			attribytes[q] = other.attribytes[q];
		for(int32_t q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
			attrishorts[q] = other.attrishorts[q];
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
			attributes[q] = other.attributes[q];
	}
	if(flags.get(CMB_ADVP_FLAGS))
		usrflags = other.usrflags;
	if(flags.get(CMB_ADVP_GENFLAGS))
		genflags = other.genflags;
	if(flags.get(CMB_ADVP_LABEL))
		label = other.label;
	if(flags.get(CMB_ADVP_SCRIPT))
	{
		script = other.script;
		for(int32_t q = 0; q < 8; ++q)
			initd[q] = other.initd[q];
	}
	if(flags.get(CMB_ADVP_EFFECT))
		walk = (walk&0x0F) | (other.walk&0xF0);
	if(flags.get(CMB_ADVP_TRIGGERS))
		triggers = other.triggers;
	if(flags.get(CMB_ADVP_LIFTING))
	{
		liftcmb = other.liftcmb;
		liftundercmb = other.liftundercmb;
		liftcs = other.liftcs;
		liftundercs = other.liftundercs;
		liftdmg = other.liftdmg;
		liftlvl = other.liftlvl;
		liftitm = other.liftitm;
		liftflags = other.liftflags;
		liftgfx = other.liftgfx;
		liftsprite = other.liftsprite;
		liftsfx = other.liftsfx;
		liftbreaksprite = other.liftbreaksprite;
		liftbreaksfx = other.liftbreaksfx;
		lifthei = other.lifthei;
		lifttime = other.lifttime;
		lift_parent_item = other.lift_parent_item;
		lift_weap_data = other.lift_weap_data;
	}
	if(flags.get(CMB_ADVP_GEN_MOVESPEED))
	{
		speed_mult = other.speed_mult;
		speed_div = other.speed_div;
		speed_add = other.speed_add;
	}
	if(flags.get(CMB_ADVP_GEN_SFX))
	{
		sfx_appear = other.sfx_appear;
		sfx_disappear = other.sfx_disappear;
		sfx_loop = other.sfx_loop;
		sfx_walking = other.sfx_walking;
		sfx_standing = other.sfx_standing;
		sfx_tap = other.sfx_tap;
		sfx_landing = other.sfx_landing;
	}
	if(flags.get(CMB_ADVP_GEN_SPRITES))
	{
		spr_appear = other.spr_appear;
		spr_disappear = other.spr_disappear;
		spr_walking = other.spr_walking;
		spr_standing = other.spr_standing;
		spr_falling = other.spr_falling;
		spr_drowning = other.spr_drowning;
		spr_lava_drowning = other.spr_lava_drowning;
		sfx_falling = other.sfx_falling;
		sfx_drowning = other.sfx_drowning;
		sfx_lava_drowning = other.sfx_lava_drowning;
	}
	if(flags.get(CMB_ADVP_MISC_WEAP_DATA))
		misc_weap_data = other.misc_weap_data;
}

bool is_push_flag(int flag, optional<int> dir)
{
	switch(flag)
	{
		case mfPUSHUD: case mfPUSHUDNS: case mfPUSHUDINS:
			return !dir || *dir <= down;
		case mfPUSHLR: case mfPUSHLRNS: case mfPUSHLRINS:
			return !dir || *dir >= left;
		case mfPUSHU: case mfPUSHUNS: case mfPUSHUINS:
			return !dir || *dir==up;
		case mfPUSHD: case mfPUSHDNS: case mfPUSHDINS:
			return !dir || *dir==down;
		case mfPUSHL: case mfPUSHLNS: case mfPUSHLINS:
			return !dir || *dir==left;
		case mfPUSHR: case mfPUSHRNS: case mfPUSHRINS:
			return !dir || *dir==right;
		case mfPUSH4: case mfPUSH4NS: case mfPUSH4INS:
			return true;
	}
	return false;
}
