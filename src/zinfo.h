#ifndef _ZINFO_H
#define _ZINFO_H
#include "base/mapscr.h"

void assignchar(char** p, char const* str);

struct zinfo
{
	void clear_ic_help();
	void clear_ic_name();
	void clear_ctype_name();
	void clear_ctype_help();
	void clear_mf_name();
	void clear_mf_help();
	void clear_ctr_name();
	void clear_weap_name();
	void clear();
	zinfo();

	//ZQ Only
	char *ic_help_string[itype_max];
	char *ctype_name[cMAX];
	char *ctype_help_string[cMAX];
	char *mf_name[mfMAX];
	char *mf_help_string[mfMAX];
	char *weap_name[wMax];

	//Shared
	char *ic_name[itype_max];
	char *ctr_name[MAX_COUNTERS];
	
	bool isUsableItemclass(size_t q);
	bool isUsableComboType(size_t q);
	bool isUsableMapFlag(size_t q);
	bool isUsableWeap(size_t q);
	bool isUsableCtr(int32_t q);
	char const* getItemClassName(size_t q);
	char const* getItemClassHelp(size_t q);
	char const* getComboTypeName(size_t q);
	char const* getComboTypeHelp(size_t q);
	char const* getMapFlagName(size_t q);
	char const* getWeapName(size_t q);
	char const* getMapFlagHelp(size_t q);
	char const* getCtrName(int32_t q);
	
	void copyFrom(zinfo const& other);
	bool isNull();
};

extern zinfo ZI;

int32_t writezinfo(PACKFILE *f, zinfo const& z);
int32_t readzinfo(PACKFILE *f, zinfo& z, zquestheader const& hdr);

//old jank module shit
struct zcmodule
{
	char module_name[2048]; //filepath for current zcmodule file
	char quests[10][255]; //first five quests, filenames
	char skipnames[10][255]; //name entry passwords
	char datafiles[5][255]; //qst.dat, zquest.dat, fonts.dat, sfx.dat, zelda.dat

	byte old_quest_serial_flow; //Do we go from 3rd to 5th,
	//and from 5th to 4th, or just 1->2->3->4->5
	//If this is 0, we do quests in strict order.
	//if it is 1, then we use the old hardcoded quest flow.
	
	byte max_quest_files;
	word startingdmap[10];
	word startingscreen[10];
	int32_t title_track, tf_track, gameover_track, ending_track, dungeon_track, overworld_track, lastlevel_track;
	
	char enem_type_names[eeMAX][255];
	char enem_anim_type_names[aMAX][255];
	char roomtype_names[rMAX][255];
	char walkmisc7_names[e7tEATHURT+1][255];
	char walkmisc9_names[e9tARMOS+1][255];
	char guy_type_names[gDUMMY1][255];
	char enemy_weapon_names[wMax-wEnemyWeapons][255];
	char enemy_scriptweaponweapon_names[10][255];
	char player_weapon_names[wIce+1][255];
	
	char base_NSF_file[1024];
	char copyright_strings[3][2048];
	int32_t copyright_string_vars[10*3]; //font, 104,136,13,-1
	char animate_NES_title;
	char delete_quest_data_on_wingame[20]; //Do we purge items, scripts, and other data when moving to the next quest?
	
	int32_t select_screen_tiles[sels_tile_LAST];
	char select_screen_tile_csets[sels_tile_cset_LAST];
	byte refresh_title_screen;
	
	//to add, and init
	//word startingdmap, startingscreen;
	//char enemy_script_weapon_names[10][255];
	
	char moduletitle[255];
	byte modver_1, modver_2, modver_3, modver_4, modbuild, modbeta;
	byte modmonth, modday, modhour, modminute;
	word modyear;
	char moduleauthor[255];
	char moduleinfo0[255];
	char moduleinfo1[255];
	char moduleinfo2[255];
	char moduleinfo3[255];
	char moduleinfo4[255];
	char moduletimezone[7]; //supports fiveb char abbreviations, and UTC+ or UTC- nn.
	//char module_base_nsf[255];
	
}; //zcmodule

#endif

