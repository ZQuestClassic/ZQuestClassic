#ifndef _GAMEDATA_H_
#define _GAMEDATA_H_

#include "base/general.h"
#include "base/ints.h"
#include "base/zc_array.h"
#include "user_object.h"

#define DIDCHEAT_BIT 0x80
#define NUM_GSWITCHES 256
#define MAX_MI (MAXDMAPS*MAPSCRSNORMAL)

enum
{
	crNONE = -1,
	crLIFE, crMONEY, crBOMBS, crARROWS, crMAGIC,
	crKEYS, crSBOMBS, crCUSTOM1, crCUSTOM2, crCUSTOM3,
	crCUSTOM4, crCUSTOM5, crCUSTOM6, crCUSTOM7, crCUSTOM8,
	crCUSTOM9, crCUSTOM10, crCUSTOM11, crCUSTOM12, crCUSTOM13,
	crCUSTOM14, crCUSTOM15, crCUSTOM16, crCUSTOM17, crCUSTOM18,
	crCUSTOM19, crCUSTOM20, crCUSTOM21, crCUSTOM22, crCUSTOM23,
	crCUSTOM24, crCUSTOM25, crCUSTOM26, crCUSTOM27, crCUSTOM28,
	crCUSTOM29, crCUSTOM30, crCUSTOM31, crCUSTOM32, crCUSTOM33,
	crCUSTOM34, crCUSTOM35, crCUSTOM36, crCUSTOM37, crCUSTOM38,
	crCUSTOM39, crCUSTOM40, crCUSTOM41, crCUSTOM42, crCUSTOM43,
	crCUSTOM44, crCUSTOM45, crCUSTOM46, crCUSTOM47, crCUSTOM48,
	crCUSTOM49, crCUSTOM50, crCUSTOM51, crCUSTOM52, crCUSTOM53,
	crCUSTOM54, crCUSTOM55, crCUSTOM56, crCUSTOM57, crCUSTOM58,
	crCUSTOM59, crCUSTOM60, crCUSTOM61, crCUSTOM62, crCUSTOM63,
	crCUSTOM64, crCUSTOM65, crCUSTOM66, crCUSTOM67, crCUSTOM68,
	crCUSTOM69, crCUSTOM70, crCUSTOM71, crCUSTOM72, crCUSTOM73,
	crCUSTOM74, crCUSTOM75, crCUSTOM76, crCUSTOM77, crCUSTOM78,
	crCUSTOM79, crCUSTOM80, crCUSTOM81, crCUSTOM82, crCUSTOM83,
	crCUSTOM84, crCUSTOM85, crCUSTOM86, crCUSTOM87, crCUSTOM88,
	crCUSTOM89, crCUSTOM90, crCUSTOM91, crCUSTOM92, crCUSTOM93,
	crCUSTOM94, crCUSTOM95, crCUSTOM96, crCUSTOM97, crCUSTOM98,
	crCUSTOM99, crCUSTOM100, MAX_COUNTERS
};
enum generic_ind
{
	genHCP, genMDRAINRATE, genCANSLASH, genWLEVEL,
	genHCP_PER_HC, genCONTHP, genCONTHP_IS_PERC, genHP_PER_HEART,
	genMP_PER_BLOCK, genHERO_DMG_MULT, genENE_DMG_MULT,
	genDITH_TYPE, genDITH_ARG, genDITH_PERC, genLIGHT_RAD,genTDARK_PERC,genDARK_COL,
	genWATER_GRAV, genSIDESWIM_UP, genSIDESWIM_SIDE, genSIDESWIM_DOWN, genSIDESWIM_JUMP,
	genBUNNY_LTM, genSWITCHSTYLE, genLAST,
	genMAX = 256
};

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
	byte  /*_wlevel,*/_cheat;
	bool  item[MAXITEMS];
	byte  items_off[MAXITEMS];
	word _maxcounter[MAX_COUNTERS];	// 0 - life, 1 - rupees, 2 - bombs, 3 - arrows, 4 - magic, 5 - keys, 6-super bombs
	word _counter[MAX_COUNTERS];
	int16_t _dcounter[MAX_COUNTERS];
	
	char  version[17];
	byte  lvlitems[MAXLEVELS];
	byte  lvlkeys[MAXLEVELS];
	dword lvlswitches[MAXLEVELS];
	byte  _continue_scrn;
	word  _continue_dmap;
	int32_t _generic[genMAX];	// Generic gamedata. See enum above this struct for indexes.
	byte  visited[MAXDMAPS];
	byte  bmaps[MAXDMAPS*MAPSCRSNORMAL];                      // the dungeon progress maps
	word  maps[MAXMAPS*MAPSCRSNORMAL];                       // info on map changes, items taken, etc.
	byte  guys[MAXMAPS*MAPSCRSNORMAL];                       // guy counts (though dungeon guys are reset on entry)
	bool item_messages_played[MAXITEMS];  //Each field is set when an item pickup message plays the first time per session
	int32_t  screen_d[MAX_MI][8];                // script-controlled screen variables
	int32_t  global_d[MAX_SCRIPT_REGISTERS];                                      // script-controlled global variables
	std::vector< ZCArray <int32_t> > globalRAM;
	
	word awpn = 255, bwpn = 255, xwpn = 255, ywpn = 255;
	int16_t abtn_itm = -1, bbtn_itm = -1, xbtn_itm = -1, ybtn_itm = -1;
	int16_t forced_awpn = -1, forced_bwpn = -1, forced_xwpn = -1, forced_ywpn = -1;
	bool isclearing; // The gamedata is being cleared
	//115456 (260)
	byte bottleSlots[256];
	
	savedportal saved_mirror_portal;
	
	byte swim_mult = 1, swim_div = 1;
	
	bool gen_doscript[NUMSCRIPTSGENERIC];
	word gen_exitState[NUMSCRIPTSGENERIC];
	word gen_reloadState[NUMSCRIPTSGENERIC];
	int32_t gen_initd[NUMSCRIPTSGENERIC][8];
	int32_t gen_dataSize[NUMSCRIPTSGENERIC];
	std::vector<int32_t> gen_data[NUMSCRIPTSGENERIC];
	uint32_t gen_eventstate[NUMSCRIPTSGENERIC];
	
	uint32_t xstates[MAXMAPS*MAPSCRSNORMAL];
	
	int32_t gswitch_timers[NUM_GSWITCHES];
	int16_t OverrideItems[itype_max] = {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,
		-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2};

	std::vector<saved_user_object> user_objects;
	std::vector<savedportal> user_portals;
	
	std::vector<int32_t> screen_data[MAXMAPS*MAPSCRS];
	
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
		if(screen_data[indx].size() == size_t(sz))
			return;
		screen_data[indx].resize(sz, 0);
	}
	
	void Clear();
	void Copy(const gamedata& g);
	void clear_genscript();
	
	void save_user_objects();
	void load_user_objects();

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
		if(slot > 255) return 0;
		return bottleSlots[slot];
	}
	void set_bottle_slot(dword slot, byte val)
	{
		if(slot > 255) return;
		if(val > 64) val = 0;
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
};

extern gamedata *game;

#endif

