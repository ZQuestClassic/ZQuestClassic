#ifndef _INITDATA_H_
#define _INITDATA_H_

#include "base/headers.h"
#include "base/containers.h"
#include <vector>

struct zinitdata
{
	byte items[MAXITEMS/8];
	
	byte map[MAXLEVELS/8];
	byte compass[MAXLEVELS/8];
	byte boss_key[MAXLEVELS/8];
	byte mcguffin[MAXLEVELS/8];
	byte level_keys[MAXLEVELS];
	
	bool get_item(size_t ind) const {return get_bit(items,ind);}
	void set_item(size_t ind, bool st) {set_bit(items,ind,st);}
	
	word counter[MAX_COUNTERS];
	word mcounter[MAX_COUNTERS];
	
	byte bomb_ratio = 4; // ratio of super bombs to bombs
	byte hcp, hcp_per_hc = 4;
	word cont_heart; // continue health
	
public:
	
	byte misc[16];
	
	byte last_map; //last map worked on
	
	byte last_screen; //last screen worked on
	byte msg_more_x, msg_more_y, msg_more_is_offset;
	byte subscreen;
	word start_dmap;
	byte heroAnimationStyle;
	
	int32_t ss_grid_x = 8;
	int32_t ss_grid_y = 8;
	int32_t ss_grid_xofs;
	int32_t ss_grid_yofs;
	int32_t ss_grid_color = 8;
	int32_t ss_bbox_1_color = 15;
	int32_t ss_bbox_2_color = 7;
	int32_t ss_flags;
	byte subscreen_style;
	byte usecustomsfx;
	byte gravity = 16; //Deprecated!
	int32_t gravity2 = 1600; //Bumping this up to an int32_t.
	word terminalv = 320;
	byte msg_speed = 5;
	byte transition_type; // Can't edit, yet.
	byte jump_hero_layer_threshold = 255; // Hero is drawn above layer 3 if z > this.
	byte hero_swim_speed;
	
	word heroStep, subscrSpeed = 1, heroSideswimUpStep, heroSideswimSideStep, heroSideswimDownStep;
	
	int32_t exitWaterJump;

	byte hp_per_heart, magic_per_block, hero_damage_multiplier, ene_damage_multiplier;
	
	int32_t swimgravity;
	
	byte dither_type, dither_arg, dither_percent, def_lightrad, transdark_percent, darkcol;
	
	int32_t bunny_ltm;
	byte switchhookstyle;
	
	byte magicdrainrate;
	
	byte hero_swim_mult = 2, hero_swim_div = 3;
	
	bool gen_doscript[NUMSCRIPTSGENERIC];
	word gen_exitState[NUMSCRIPTSGENERIC];
	word gen_reloadState[NUMSCRIPTSGENERIC];
	int32_t gen_initd[NUMSCRIPTSGENERIC][8];
	uint32_t gen_eventstate[NUMSCRIPTSGENERIC];
	
	bounded_vec<word,bounded_vec<uint32_t,int32_t>> gen_data {NUMSCRIPTSGENERIC, {0}};
	bounded_vec<uint32_t,bounded_vec<uint32_t,int32_t>> screen_data {MAXSCRS, {0}};
	
	zfix shove_offset = 6.5_zf;
	
	void clear_genscript();
	
	void clear();
	void copy(zinitdata const& other);
	
	zinitdata() = default;
};
extern zinitdata zinit;

#endif

