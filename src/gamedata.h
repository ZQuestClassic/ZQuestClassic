#ifndef GAMEDATA_H_
#define GAMEDATA_H_

#include "base/general.h"
#include "base/ints.h"
#include "base/zc_array.h"
#include "base/containers.h"
#include "user_object.h"

#define DIDCHEAT_BIT 0x80
#define NUM_GSWITCHES 256
#define MAX_MI (MAXDMAPS*MAPSCRSNORMAL)

#define MAX_SAVED_PORTALS 10000
struct savedportal
{
	bool operator==(const savedportal&) const = default;

	int16_t destdmap = -1;
	int16_t srcdmap = -1;
	byte srcscr;
	byte destscr;
	int32_t x;
	int32_t y;
	byte sfx;
	int32_t warpfx;
	int16_t spr;
	bool deleting;
	
	int32_t getUID(){return uid;}
	void clearUID(){uid = 0;}
	
	savedportal();
	void clear()
	{
		*this = savedportal();
	}
	
private:
	int32_t uid;
	inline static int32_t nextuid = 1;
};

// Everything needed by the title screen.
struct gamedata_header
{
	bool operator==(const gamedata_header&) const = default;

	std::string qstpath;
	std::string replay_file;
	std::string name;
	std::string title;
	byte quest;
	int deaths;
	word life;
	word maxlife;
	byte hp_per_heart_container;
	bool has_played;
	bool time_valid;
	bool did_cheat;
	dword time;
	byte icon[128];
	byte pal[48];
};

struct gamedata
{
	bool operator==(const gamedata&) const = default;

	gamedata_header header;
	byte _cheat;
	bool item[MAXITEMS];
	byte items_off[MAXITEMS];
	word _maxcounter[MAX_COUNTERS];
	word _counter[MAX_COUNTERS];
	int16_t _dcounter[MAX_COUNTERS];
	
	char version[17];
	bounded_vec<word,byte> lvlitems {MAXLEVELS, 0};
	bounded_vec<word,byte> lvlkeys {MAXLEVELS, 0};
	bounded_vec<word,dword> lvlswitches {MAXLEVELS, 0};
	byte _continue_scrn;
	word _continue_dmap;
	bounded_vec<word,int32_t> _generic {genMAX, 0}; // Generic gamedata. See enum above this struct for indexes.
	byte visited[MAXDMAPS];
	bounded_vec<dword,byte> bmaps {MAX_MI, 0}; // the dungeon progress maps
	bounded_vec<dword,word> maps {MAXSCRSNORMAL, 0}; // info on map changes, items taken, etc.
	bounded_vec<dword,byte> guys {MAXSCRSNORMAL, 0}; // guy counts (enemy kill progress)
	bool item_messages_played[MAXITEMS]; //Each field is set when an item pickup message plays the first time per session
	bounded_map<dword,bounded_vec<byte,int32_t>> screen_d {MAX_MI, {8, 0}}; // script-controlled screen variables
	int32_t global_d[MAX_SCRIPT_REGISTERS]; // script-controlled global variables
	script_object_type global_d_types[MAX_SCRIPT_REGISTERS];
	
	word awpn = 255, bwpn = 255, xwpn = 255, ywpn = 255;
	int16_t abtn_itm = -1, bbtn_itm = -1, xbtn_itm = -1, ybtn_itm = -1;
	int16_t forced_awpn = -1, forced_bwpn = -1, forced_xwpn = -1, forced_ywpn = -1;
	
	byte bottleSlots[NUM_BOTTLE_SLOTS];
	
	savedportal saved_mirror_portal;
	
	byte swim_mult = 1, swim_div = 1;
	
	bitstring gen_doscript;
	bounded_map<word,word> gen_exitState {NUMSCRIPTSGENERIC, 0};
	bounded_map<word,word> gen_reloadState {NUMSCRIPTSGENERIC, 0};
	bounded_map<word,bounded_vec<byte,int32_t>> gen_initd {NUMSCRIPTSGENERIC, {8, 0}};
	bounded_map<word,uint32_t> gen_eventstate {NUMSCRIPTSGENERIC, 0};
	bounded_map<word,bounded_map<dword,int32_t>> gen_data {NUMSCRIPTSGENERIC, {0, 0}};
	
	bounded_map<dword,uint32_t> xstates {MAXSCRSNORMAL, 0};
	bounded_map<dword,std::array<byte,4>> xdoors {MAXSCRSNORMAL, {0}};
	
	bounded_map<word,int32_t> gswitch_timers {NUM_GSWITCHES, 0};
	bounded_map<word,int16_t> OverrideItems {itype_max, -2};

	std::vector<user_object> script_objects;
	std::vector<script_array> script_arrays;
	std::vector<savedportal> user_portals;

	// Replaced by script_arrays for newer saves.
	std::vector<ZScriptArray> globalRAM;
	// Replaced by script_objects for newer saves.
	std::vector<saved_user_object> compat_saved_user_objects;

	bounded_map<dword,bounded_map<dword,int32_t>> screen_data {MAXSCRS, {0, 0}};
	
	size_t scriptDataSize(int32_t indx) const
	{
		if(unsigned(indx) >= MAXMAPS*MAPSCRS)
			return 0;
		return screen_data[indx].size();
	}
	void scriptDataResize(int32_t indx, int32_t sz)
	{
		if(unsigned(indx) >= MAXMAPS*MAPSCRS)
			return;
		sz = vbound(sz, 0, 214748);
		screen_data[indx].resize(sz);
	}
	
	void Clear();
	void Copy(const gamedata& g);
	void clear_genscript();

	void save_objects(bool gc_array);
	void load_objects(bool gc_array);

private:
	void save_user_objects();
	void save_script_objects();
	void load_user_objects();
	void load_script_objects();

public:

	const char *get_qstpath() const;
	void set_qstpath(std::string qstpath);

	const char *get_name() const;
	void set_name(std::string n);
	
	byte get_quest() const;
	void set_quest(byte q);
	void change_quest(int16_t q);
	
	word get_counter(byte c) const;
	void set_counter(word change, byte c);
	void change_counter(int16_t change, byte c);
	
	word get_maxcounter(byte c) const;
	void set_maxcounter(word change, byte c);
	void change_maxcounter(int16_t change, byte c);
	
	int16_t get_dcounter(byte c) const;
	void set_dcounter(int16_t change, byte c);
	void change_dcounter(int16_t change, byte c);
	
	word get_life() const;
	void set_life(word l);
	void change_life(int16_t l);
	
	word get_maxlife() const;
	void set_maxlife(word m);
	void change_maxlife(int16_t m);
	
	int16_t get_drupy();
	void set_drupy(int16_t d);
	void change_drupy(int16_t d);
	
	word get_rupies();
	word get_spendable_rupies();
	void set_rupies(word r);
	void change_rupies(int16_t r);
	
	word get_maxarrows();
	void set_maxarrows(word a);
	void change_maxarrows(int16_t a);
	
	word get_arrows();
	void set_arrows(word a);
	void change_arrows(int16_t a);
	
	word get_deaths() const;
	void set_deaths(word d);
	void change_deaths(int16_t d);
	
	word get_keys();
	void set_keys(word k);
	void change_keys(int16_t k);
	
	word get_bombs();
	void set_bombs(word k);
	void change_bombs(int16_t k);
	
	word get_maxbombs();
	void set_maxbombs(word b, bool setSuperBombs=true);
	void change_maxbombs(int16_t b);
	
	word get_sbombs();
	void set_sbombs(word k);
	void change_sbombs(int16_t k);
	
	word get_wlevel();
	void set_wlevel(word l);
	void change_wlevel(int16_t l);
	
	byte get_cheat() const;
	void set_cheat(byte c);
	void did_cheat(bool set);
	bool did_cheat() const;
	
	byte get_hasplayed() const;
	void set_hasplayed(byte p);
	
	dword get_time() const;
	void set_time(dword t);
	void change_time(int64_t t);
	
	byte get_timevalid() const;
	void set_timevalid(byte t);
	
	byte get_HCpieces();
	void set_HCpieces(byte p);
	void change_HCpieces(int16_t p);
	
	byte get_hcp_per_hc();
	void set_hcp_per_hc(byte val);
	
	byte get_cont_hearts();
	void set_cont_hearts(byte val);
	
	bool get_cont_percent();
	void set_cont_percent(bool ispercent);
	
	byte get_hp_per_heart() const;
	void set_hp_per_heart(byte val);
	
	byte get_mp_per_block();
	void set_mp_per_block(byte val);
	
	byte get_hero_dmgmult();
	void set_hero_dmgmult(byte val);
	
	byte get_ene_dmgmult();
	void set_ene_dmgmult(byte val);
	
	byte get_dither_type();
	void set_dither_type(byte val);
	
	byte get_dither_arg();
	void set_dither_arg(byte val);

	byte get_dither_perc();
	void set_dither_perc(byte val);

	byte get_light_rad();
	void set_light_rad(byte val);
	
	byte get_transdark_perc();
	void set_transdark_perc(byte val);
	
	byte get_darkscr_color();
	void set_darkscr_color(byte val);
	
	word get_light_wave_rate();
	void set_light_wave_rate(word val);
	
	word get_light_wave_size();
	void set_light_wave_size(word val);
	
	int32_t get_watergrav();
	void set_watergrav(int32_t val);
	
	int32_t get_sideswim_up();
	void set_sideswim_up(int32_t val);
	
	int32_t get_sideswim_side();
	void set_sideswim_side(int32_t val);
	
	int32_t get_sideswim_down();
	void set_sideswim_down(int32_t val);
	
	int32_t get_sideswim_jump();
	void set_sideswim_jump(int32_t val);
	
	int32_t get_bunny_ltm();
	void set_bunny_ltm(int32_t val);
	
	byte get_switchhookstyle();
	void set_switchhookstyle(byte val);
	
	byte get_spriteflickerspeed();
	void set_spriteflickerspeed(byte val);

	byte get_spriteflickercolor();
	void set_spriteflickercolor(byte val);

	byte get_spriteflickertransp();
	void set_spriteflickertransp(byte val);
	
	byte get_regionmapping();
	void set_regionmapping(byte val);

	byte get_continue_scrn() const;
	void set_continue_scrn(byte s);
	void change_continue_scrn(int16_t s);
	
	word get_continue_dmap() const;
	void set_continue_dmap(word d);
	void change_continue_dmap(int16_t d);
	
	word get_maxmagic();
	void set_maxmagic(word m);
	void change_maxmagic(int16_t m);
	
	word get_magic();
	void set_magic(word m);
	void change_magic(int16_t m);
	
	int16_t get_dmagic();
	void set_dmagic(int16_t d);
	void change_dmagic(int16_t d);
	
	byte get_magicdrainrate();
	void set_magicdrainrate(byte r);
	void change_magicdrainrate(int16_t r);
	
	byte get_canslash();
	void set_canslash(byte s);
	void change_canslash(int16_t s);
	
	int32_t get_generic(byte c) const;
	void set_generic(int32_t change, byte c);
	void change_generic(int32_t change, byte c);
	
	byte get_lkeys();
	
	void set_item(int32_t id, bool value);
	void set_item_no_flush(int32_t id, bool value);
	inline bool get_item(int32_t id) const
	{
		if ( ((unsigned)id) >= MAXITEMS ) return false;
			return item[id];
	}
	
	byte get_bottle_slot(dword slot)
	{
		if(slot >= NUM_BOTTLE_SLOTS) return 0;
		return bottleSlots[slot];
	}
	void set_bottle_slot(dword slot, byte val)
	{
		if(slot >= NUM_BOTTLE_SLOTS) return;
		if(val > NUM_BOTTLE_TYPES) val = 0;
		bottleSlots[slot] = val;
	}
	
	int32_t fillBottle(byte val);
	bool canFillBottle();
	
	void set_portal(int16_t destdmap, int16_t srcdmap, byte scr, int32_t x, int32_t y, byte sfx, int32_t weffect, int16_t psprite);
	void load_portal();
	void clear_portal(int32_t);
	
	void load_portals();
	savedportal* getSavedPortal(int32_t uid);

	bool should_show_time();
	
	void normalize()
	{
		lvlitems.normalize();
		lvlkeys.normalize();
		lvlswitches.normalize();
		_generic.normalize();
		bmaps.normalize();
		maps.normalize();
		guys.normalize();
		screen_d.normalize();
		gen_doscript.normalize();
		gen_exitState.normalize();
		gen_reloadState.normalize();
		gen_initd.normalize();
		gen_eventstate.normalize();
		gen_data.normalize();
		xstates.normalize();
		xdoors.normalize();
		gswitch_timers.normalize();
		OverrideItems.normalize();
		screen_data.normalize();
	}
};

extern gamedata *game;

#endif

