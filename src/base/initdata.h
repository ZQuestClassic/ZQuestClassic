#ifndef INITDATA_H_
#define INITDATA_H_

#include "base/headers.h"
#include "base/containers.h"
#include <vector>

enum
{
	INIT_FL_CONTPERCENT,
	INIT_FL_CANSLASH,
	INIT_FL_MAX
};
enum
{
	// Mark all screens as visited when loading a region.
	REGION_MAPPING_FULL,
	// Mark only screens as visited when the player steps into it (see update_heroscr).
	REGION_MAPPING_PHYSICAL,
	REGION_MAPPING_MAX
};
enum
{
	SPRITE_THRESHOLD_GROUND,
	SPRITE_THRESHOLD_3,
	SPRITE_THRESHOLD_4,
	SPRITE_THRESHOLD_OVERHEAD,
	SPRITE_THRESHOLD_5,
	SPRITE_THRESHOLD_MAX
};
struct zinitdata
{
	byte items[MAXITEMS/8];
	
	word litems[MAXLEVELS] = {0};
	bounded_vec<word,dword> lvlswitches {MAXLEVELS, 0};
	bounded_vec<word,byte> level_keys {MAXLEVELS, 0};
	
	word counter[MAX_COUNTERS];
	word mcounter[MAX_COUNTERS] = {0, 255, 0, 0, 0, 255}; // crMONEY/crKEYS = 255
	
	byte bomb_ratio = 4; // ratio of super bombs to bombs
	byte hcp, hcp_per_hc = 4;
	word cont_heart; // continue health

	byte hp_per_heart, magic_per_block, hero_damage_multiplier, ene_damage_multiplier;
	byte dither_type, dither_arg, dither_percent, def_lightrad, transdark_percent, darkcol;
	word light_wave_rate, light_wave_size;
	
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
	zfix air_drag = 0.0100_zf;
	
	byte region_mapping;
	
	word item_spawn_flicker = 32, item_timeout_dur = 512, item_timeout_flicker = 0;
	byte item_flicker_speed = 2;
	
	bitstring gen_doscript;
	bounded_map<word,word> gen_exitState {NUMSCRIPTSGENERIC};
	bounded_map<word,word> gen_reloadState {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_vec<byte,int32_t>> gen_initd {NUMSCRIPTSGENERIC, {8}};
	bounded_map<word,uint32_t> gen_eventstate {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_map<dword,int32_t>> gen_data {NUMSCRIPTSGENERIC, {0}};
	bounded_map<dword,bounded_map<dword,int32_t>> screen_data {MAXSCRS, {0}};
	
	byte bottle_slot[NUM_BOTTLE_SLOTS];
	word sprite_z_thresholds[SPRITE_THRESHOLD_MAX];
	
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

