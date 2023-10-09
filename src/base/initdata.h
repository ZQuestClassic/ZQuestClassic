#ifndef _INITDATA_H_
#define _INITDATA_H_

#include "base/general.h"
#include "base/ints.h"
#include <vector>

struct zinitdata
{
	bool items[256];
	
	byte hc;
	word start_heart, cont_heart;
	byte hcp, hcp_per_hc = 4, keys;
	word rupies;
	byte triforce; // bit flags
	byte map[64];
	byte compass[64];
	byte boss_key[64];
	byte misc[16];
	
	byte last_map; //last map worked on
	
	byte last_screen; //last screen worked on
	word max_magic;
	word magic;
	byte bomb_ratio = 4; // ratio of super bombs to bombs
	byte msg_more_x, msg_more_y, msg_more_is_offset;
	byte subscreen;
	word start_dmap;
	byte heroAnimationStyle;
	
	byte level_keys[MAXLEVELS];
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
	word max_rupees = 255, max_keys = 255;
	byte gravity = 16; //Deprecated!
	int32_t gravity2 = 1600; //Bumping this up to an int32_t.
	word terminalv = 320;
	byte msg_speed = 5;
	byte transition_type; // Can't edit, yet.
	byte jump_hero_layer_threshold = 255; // Hero is drawn above layer 3 if z > this.
	byte hero_swim_speed;
	
	word bombs, super_bombs, max_bombs, max_sbombs, arrows, max_arrows, heroStep, subscrSpeed = 1, heroSideswimUpStep, heroSideswimSideStep, heroSideswimDownStep;
	
	int32_t exitWaterJump;

	byte hp_per_heart, magic_per_block, hero_damage_multiplier, ene_damage_multiplier;
	
	word scrcnt[25], scrmaxcnt[25]; //Script counter start/max -Em
	
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
	int32_t gen_dataSize[NUMSCRIPTSGENERIC];
	std::vector<int32_t> gen_data[NUMSCRIPTSGENERIC];
	uint32_t gen_eventstate[NUMSCRIPTSGENERIC];
	
	void clear_genscript();
	
	void clear();
	void copy(zinitdata const& other);
	
	zinitdata() = default;
};
extern zinitdata zinit;

#endif

