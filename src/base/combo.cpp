#include "combo.h"
#include "sizes.h"

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
	
bool newcombo::is_blank(bool ignoreEff)
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
	if(spawnip) return false;
	if(trigcopycat) return false;
	if(trigcooldown) return false;
	if(trig_lstate) return false;
	if(trig_gstate) return false;
	if(trig_statetime) return false;
	if(trig_genscr) return false;
	if(trig_group) return false;
	if(trig_group_val) return false;
	if(!label.empty()) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRIBYTES; ++q)
		if(attribytes[q]) return false;
	for(auto q = 0; q < NUM_COMBO_ATTRISHORTS; ++q)
		if(attrishorts[q]) return false;
	if(script) return false;
	for(auto q = 0; q < 2; ++q)
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
	if(spr_appear) return false;
	if(spr_disappear) return false;
	if(spr_walking) return false;
	if(spr_standing) return false;
	return true;
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

