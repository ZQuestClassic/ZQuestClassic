#ifndef CORE_INITDATA_H_
#define CORE_INITDATA_H_

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
	bitstring items = {};
	
	word litems[MAXLEVELS] = {0};
	bounded_vec<word,dword> lvlswitches {MAXLEVELS, 0};
	bounded_vec<word,byte> level_keys {MAXLEVELS, 0};
	
	word counter[MAX_COUNTERS] = {};
	word mcounter[MAX_COUNTERS] = {0, 255, 0, 0, 0, 255}; // crMONEY/crKEYS = 255
	
	byte bomb_ratio = 4; // ratio of super bombs to bombs
	byte hcp = 0, hcp_per_hc = 4;
	word cont_heart = 100; // continue health

	byte hp_per_heart = 16, magic_per_block = 32, hero_damage_multiplier = 2, ene_damage_multiplier = 4;
	byte dither_type = 0, dither_arg = 0, dither_percent = 20, def_lightrad = 24, transdark_percent = 0, darkcol = BLACK;
	word light_wave_rate = 0, light_wave_size = 0;
	
	int32_t ss_grid_x = 8;
	int32_t ss_grid_y = 8;
	int32_t ss_grid_xofs = 0;
	int32_t ss_grid_yofs = 0;
	int32_t ss_grid_color = 8;
	int32_t ss_bbox_1_color = 15;
	int32_t ss_bbox_2_color = 7;
	int32_t ss_flags = 0;
	
	bitstring flags = {};
	
	byte last_map = 0, last_screen = 0; //last editor map/screen
	byte msg_speed = 5, msg_advance_delay = 50;
	
	int32_t gravity = 1600, swimgravity = 5;
	word terminalv = 320;
	byte hero_swim_speed = 67; //old movement still needs
	byte hero_swim_mult = 2, hero_swim_div = 3; //new movement
	
	word heroSideswimUpStep = 150, heroSideswimSideStep = 100, heroSideswimDownStep = 75;
	int32_t exitWaterJump = 0;
	
	word heroStep = 150;
	byte heroAnimationStyle = 0;
	byte jump_hero_layer_threshold = 255; // Hero is drawn above layer 3 if z > this.
	int32_t bunny_ltm = 0;
	
	word start_dmap = 0;
	word subscrSpeed = 1;
	byte switchhookstyle = 0, magicdrainrate = 2;
	byte spriteflickerspeed = 1, spriteflickercolor = 0x00, spriteflickertransp = 0;
	
	zfix shove_offset = 6.5_zf;
	zfix air_drag = 0.0100_zf;
	
	byte region_mapping = REGION_MAPPING_FULL;
	
	word item_spawn_flicker = 32, item_timeout_dur = 512, item_timeout_flicker = 0;
	byte item_flicker_speed = 2;
	
	int8_t hero_itembox_xofs = 0, hero_itembox_yofs = 8;
	byte hero_itembox_width = 16, hero_itembox_height = 8;
	
	bitstring gen_doscript = {};
	bounded_map<word,word> gen_exitState {NUMSCRIPTSGENERIC};
	bounded_map<word,word> gen_reloadState {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_vec<byte,int32_t>> gen_initd {NUMSCRIPTSGENERIC, {8}};
	bounded_map<word,std::map<word,int>> gen_inst_init {NUMSCRIPTSGENERIC, {}};
	bounded_map<word,uint32_t> gen_eventstate {NUMSCRIPTSGENERIC};
	bounded_map<word,bounded_map<dword,int32_t>> gen_data {NUMSCRIPTSGENERIC, {0}};
	bounded_map<dword,bounded_map<dword,int32_t>> screen_data {MAXSCRS, {0}};
	
	byte bottle_slot[NUM_BOTTLE_SLOTS] = {};
	word sprite_z_thresholds[SPRITE_THRESHOLD_MAX] = {};
	
	std::array<script_config_nosavescript, NUMSCRIPTGLOBAL> global_scrconfig;
	std::array<script_config_nosavescript, NUMSCRIPTHERO> hero_scrconfig;

	bool get_item(size_t ind) const {return items.get(ind);}
	void set_item(size_t ind, bool st) {items.set(ind,st);}
	
	void normalize()
	{
		level_keys.normalize();
		flags.normalize();
		gen_doscript.normalize();
		gen_exitState.normalize();
		gen_reloadState.normalize();
		gen_initd.normalize();
		gen_inst_init.normalize();
		gen_eventstate.normalize();
		gen_data.normalize();
		screen_data.normalize();
	}
	
	void clear_genscript();
	
	void clear();
	void copy(zinitdata const& other);
	
	zinitdata();
};
extern zinitdata zinit;

#endif

