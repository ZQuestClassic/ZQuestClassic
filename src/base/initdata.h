#ifndef _INITDATA_H_
#define _INITDATA_H_

#include "base/headers.h"
#include "base/containers.h"
#include <vector>

enum
{
	INIT_FL_CONTPERCENT,
	INIT_FL_CANSLASH,
	INIT_FL_MAX
};
struct zinitdata
{
	byte items[MAXITEMS/8];
	
	byte map[MAXLEVELS/8];
	byte compass[MAXLEVELS/8];
	byte boss_key[MAXLEVELS/8];
	byte mcguffin[MAXLEVELS/8];
	bounded_vec<word,byte> level_keys {MAXLEVELS};
	
	word counter[MAX_COUNTERS];
	word mcounter[MAX_COUNTERS] = {0, 255, 0, 0, 0, 255}; // crMONEY/crKEYS = 255
	
	byte bomb_ratio = 4; // ratio of super bombs to bombs
	byte hcp, hcp_per_hc = 4;
	word cont_heart; // continue health

	byte hp_per_heart, magic_per_block, hero_damage_multiplier, ene_damage_multiplier;
	byte dither_type, dither_arg, dither_percent, def_lightrad, transdark_percent, darkcol;
	
	int32_t ss_grid_x = 8;
	int32_t ss_grid_y = 8;
	int32_t ss_grid_xofs;
	int32_t ss_grid_yofs;
	int32_t ss_grid_color = 8;
	int32_t ss_bbox_1_color = 15;
	int32_t ss_bbox_2_color = 7;
	int32_t ss_flags;
	
	bitstring flags;
	
	byte last_map, last_screen; //last editor map/screen
	byte msg_more_x, msg_more_y, msg_more_is_offset, msg_speed = 5;
	
	int32_t gravity = 1600, swimgravity = 5;
	word terminalv = 320;
	byte hero_swim_speed; //old movement still needs
	byte hero_swim_mult = 2, hero_swim_div = 3; //new movement
	
	word heroSideswimUpStep, heroSideswimSideStep, heroSideswimDownStep;
	int32_t exitWaterJump;
	
	word heroStep;
	byte heroAnimationStyle;
	byte jump_hero_layer_threshold = 255; // Hero is drawn above layer 3 if z > this.
	int32_t bunny_ltm;
	
	word start_dmap;
	word subscrSpeed = 1;
	byte switchhookstyle, magicdrainrate;
	byte spriteflickerspeed = 1, spriteflickercolor = 0x00, spriteflickertransp = 0;
	
	zfix shove_offset = 6.5_zf;
	
	bitstring gen_doscript;
	bounded_map<word,word> gen_exitState {NUMSCRIPTSGENERIC};
	bounded_map<word,word> gen_reloadState {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_vec<byte,int32_t>> gen_initd {NUMSCRIPTSGENERIC, {8}};
	bounded_map<word,uint32_t> gen_eventstate {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_map<dword,int32_t>> gen_data {NUMSCRIPTSGENERIC, {0}};
	bounded_map<dword,bounded_map<dword,int32_t>> screen_data {MAXSCRS, {0}};
	
	bool get_item(size_t ind) const {return get_bit(items,ind);}
	void set_item(size_t ind, bool st) {set_bit(items,ind,st);}
	
	void normalize()
	{
		level_keys.normalize();
		flags.normalize();
		gen_doscript.normalize();
		gen_exitState.normalize();
		gen_reloadState.normalize();
		gen_initd.normalize();
		gen_eventstate.normalize();
		gen_data.normalize();
		screen_data.normalize();
	}
	
	void clear_genscript();
	
	void clear();
	void copy(zinitdata const& other);
	
	zinitdata() = default;
};
extern zinitdata zinit;

#endif

