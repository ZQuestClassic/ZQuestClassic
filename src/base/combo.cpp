#include "combo.h"
#include "general.h"
#include "mapscr.h"

std::vector<newcombo> combobuf;

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
	for(auto q = 0; q < 6; ++q)
		if(expansion[q]) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
		if(attributes[q]) return false;
	if(usrflags) return false;
	if(genflags) return false;
	for(auto q = 0; q < 6; ++q)
		if(triggerflags[q]) return false;
	if(triggerlevel) return false;
	if(triggerbtn) return false;
	if(triggeritem) return false;
	if(trigtimer) return false;
	if(trigsfx) return false;
	if(trigchange) return false;
	if(trigprox) return false;
	if(trigctr) return false;
	if(trigctramnt) return false;
	if(triglbeam) return false;
	if(trigcschange) return false;
	if(spawnitem) return false;
	if(spawnenemy) return false;
	if(exstate > -1) return false;
	if(exdoor_dir > -1) return false;
	if(spawnip) return false;
	if(trigcopycat) return false;
	if(trigcooldown) return false;
	if(trig_lstate) return false;
	if(trig_gstate) return false;
	if(trig_statetime) return false;
	if(trig_genscr) return false;
	if(trig_group) return false;
	if(trig_group_val) return false;
	if(trig_levelitems) return false;
	if(trigdmlevel > -1) return false;
	for(int q = 0; q < 3; ++q)
		if(trigtint[q]) return false;
	if(triglvlpalette > -1) return false;
	if(trigbosspalette > -1) return false;
	if(trigquaketime > -1) return false;
	if(trigwavytime > -1) return false;
	if(trig_swjinxtime > -2) return false;
	if(trig_itmjinxtime > -2) return false;
	if(trig_shieldjinxtime > -2) return false;
	if(trig_stuntime > -2) return false;
	if(trig_bunnytime > -2) return false;
	if(trig_pushtime != 8) return false;
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
	if(liftlightrad) return false;
	if(liftlightshape) return false;
	if(prompt_cid) return false;
	if(prompt_cs) return false;
	if(prompt_x != 12) return false;
	if(prompt_y != -8) return false;
	
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
	if(flags.get(CMB_ADVP_ATTRIBYTE))
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
			attribytes[q] = other.attribytes[q];
	if(flags.get(CMB_ADVP_ATTRISHORT))
		for(int32_t q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
			attrishorts[q] = other.attrishorts[q];
	if(flags.get(CMB_ADVP_ATTRIBUTE))
		for(int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
			attributes[q] = other.attributes[q];
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
	{
		for(int32_t q = 0; q < 6; ++q)
			triggerflags[q] = other.triggerflags[q];
		triggerlevel = other.triggerlevel;
		triggerbtn = other.triggerbtn;
		prompt_cid = other.prompt_cid;
		prompt_cs = other.prompt_cs;
		prompt_x = other.prompt_x;
		prompt_y = other.prompt_y;
		triggeritem = other.triggeritem;
		trigtimer = other.trigtimer;
		trigsfx = other.trigsfx;
		trigchange = other.trigchange;
		trigprox = other.trigprox;
		trigctr = other.trigctr;
		trigctramnt = other.trigctramnt;
		triglbeam = other.triglbeam;
		trigcschange = other.trigcschange;
		spawnitem = other.spawnitem;
		spawnenemy = other.spawnenemy;
		exstate = other.exstate;
		exdoor_dir = other.exdoor_dir;
		exdoor_ind = other.exdoor_ind;
		spawnip = other.spawnip;
		trigcopycat = other.trigcopycat;
		trigcooldown = other.trigcooldown;
		trig_lstate = other.trig_lstate;
		trig_gstate = other.trig_gstate;
		trig_statetime = other.trig_statetime;
		trig_genscr = other.trig_genscr;
		trig_group = other.trig_group;
		trig_group_val = other.trig_group_val;
		trig_levelitems = other.trig_levelitems;
		trigdmlevel = other.trigdmlevel;
		for(int q = 0; q < 3; ++q)
			trigtint[q] = other.trigtint[q];
		triglvlpalette = other.triglvlpalette;
		trigbosspalette = other.trigbosspalette;
		trigquaketime = other.trigquaketime;
		trigwavytime = other.trigwavytime;
		trig_swjinxtime = other.trig_swjinxtime;
		trig_itmjinxtime = other.trig_itmjinxtime;
		trig_shieldjinxtime = other.trig_shieldjinxtime;
		trig_stuntime = other.trig_stuntime;
		trig_bunnytime = other.trig_bunnytime;
		trig_pushtime = other.trig_pushtime;
	}
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
		liftlightrad = other.liftlightrad;
		liftlightshape = other.liftlightshape;
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
