#ifndef _MISCTYPES_H_
#define _MISCTYPES_H_

#include "base/ints.h"
#include "base/general.h"
enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
enum {df_walktrans=0};

struct DoorComboSet
{
	char name[21];
	//byte padding;
	//22
	word doorcombo_u[9][4];                                   //[door type][door combo]
	byte doorcset_u[9][4];                                    //[door type][door combo]
	word doorcombo_d[9][4];                                   //[door type][door combo]
	byte doorcset_d[9][4];                                    //[door type][door combo]
	//238 (216)
	word doorcombo_l[9][6];                                   //[door type][door combo]
	byte doorcset_l[9][6];                                    //[door type][door combo]
	word doorcombo_r[9][6];                                   //[door type][door combo]
	byte doorcset_r[9][6];                                    //[door type][door combo]
	//562 (324)
	word bombdoorcombo_u[2];                                  //rubble
	byte bombdoorcset_u[2];                                   //rubble
	word bombdoorcombo_d[2];                                  //rubble
	byte bombdoorcset_d[2];                                   //rubble
	//574 (12)
	word bombdoorcombo_l[3];                                  //rubble
	byte bombdoorcset_l[3];                                   //rubble
	//byte padding;
	word bombdoorcombo_r[3];                                  //rubble
	byte bombdoorcset_r[3];                                   //rubble
	//byte padding;
	//594 (18)
	word walkthroughcombo[4];                                 //[n, s, e, w]
	byte walkthroughcset[4];                                  //[n, s, e, w]
	
	byte flags[2];
	byte expansion[30];
	//638 (44)
};

struct shoptype
{
	char name[33];
	byte item[3];
	byte hasitem[3];
	word price[3];
	word str[3];  //item info string
	//10
};

struct bottleshoptype
{
	char name[33];
	byte fill[3];
	word comb[3];
	byte cset[3];
	word price[3];
	word str[3];
	void clear()
	{
		memset(name, 0, sizeof(name));
		memset(fill, 0, sizeof(fill));
		memset(comb, 0, sizeof(comb));
		memset(cset, 0, sizeof(cset));
		memset(price, 0, sizeof(price));
		memset(str, 0, sizeof(str));
	}
};

struct bottletype
{
	char name[33];
	char counter[3];
	word amount[3];
	byte flags;
	byte next_type;
#define BTFLAG_PERC0       0x01
#define BTFLAG_PERC1       0x02
#define BTFLAG_PERC2       0x04
#define BTFLAG_CURESWJINX  0x10
#define BTFLAG_CUREITJINX  0x20
#define BTFLAG_ALLOWIFFULL 0x40
#define BTFLAG_AUTOONDEATH 0x80
	void clear()
	{
		memset(name, 0, sizeof(name));
		memset(counter, -1, sizeof(counter));
		memset(amount, 0, sizeof(amount));
		flags = 0;
		next_type = 0;
	}
	bool is_blank() const
	{
		if(name[0])
			return false;
		for(int q = 0; q < 3; ++q)
		{
			if(counter[q] > -1 && amount[q] > 0)
				return false;
			if(next_type)
				return false;
		}
		return true;
	}
};

struct pondtype
{
	byte olditem[3];
	byte newitem[3];
	byte fairytile;
	byte aframes;
	byte aspeed;
	word msg[15];
	byte foo[32];
};

struct infotype
{
	char name[33];
	word str[3];
	word price[3];
};

struct warpring
{
	word dmap[9];
	byte scr[9];
	byte size;
};

struct zcolors
{
	byte text, caption;
	byte overw_bg, dngn_bg;
	byte dngn_fg, cave_fg;
	byte bs_dk, bs_goal;
	byte compass_lt, compass_dk;
	//10
	byte subscr_bg, subscr_shadow, triframe_color;
	byte bmap_bg,bmap_fg;
	byte hero_dot;
	//15
	byte triforce_cset;
	byte triframe_cset;
	byte overworld_map_cset;
	byte dungeon_map_cset;
	byte blueframe_cset;
	//20
	int32_t triforce_tile;
	int32_t triframe_tile;
	int32_t overworld_map_tile;
	int32_t dungeon_map_tile;
	int32_t blueframe_tile;
	//40
	int32_t HCpieces_tile;
	byte HCpieces_cset;
	byte msgtext;
	
	byte foo[6];
	//52
	byte foo2[256];
	
	
	//308 bytes
};

struct palcycle
{
	byte first,count,speed;
	//3
};


enum miscsprite
{
	sprFALL,
	sprDROWN,
	sprLAVADROWN,
	sprSWITCHPOOF,
	sprMAX = 256
};
enum miscsfx
{
	sfxBUSHGRASS,
	sfxSWITCHED,
	sfxLOWHEART,
	sfxERROR,
	sfxHURTPLAYER,
	sfxHAMMERPOUND,
	sfxSUBSCR_ITEM_ASSIGN,
	sfxSUBSCR_CURSOR_MOVE,
	sfxREFILL,
	sfxDRAIN,
	sfxTAP,
	sfxTAP_HOLLOW,
	sfxMAX = 256
};

struct miscQdata
{
	shoptype shop[NUM_SHOPS];
	//160 (160=10*16)
	infotype info[NUM_INFOS];
	//384 (224=14*16)
	warpring warp[NUM_WARP_RINGS];
	//528 (144=18*8)
	palcycle cycles[NUM_PAL_CYCLES][3];
	//2832 (2304=3*256*3)
	//2850 (18=2*2)
	byte     triforce[8];                                     // positions of triforce pieces on subscreen
	//2858 (8)
	zcolors  colors;
	//3154 (296)
	int32_t     icons[4];
	//3162 (8=2*4)
	//pondtype pond[16];
	//4314 (1152=72*16)
	word endstring;
	//  byte dummy;  // left over from a word
	//word expansion[98];
	//4512
	int32_t questmisc[32]; //Misc init values for the user. Used by scripts.
	char questmisc_strings[32][128]; //needs to be memset then data allocated from IntiData
	//We probably want a way to access these in ZScript by their string, or to get the strings stored.
	int32_t zscript_last_compiled_version;
	byte sprites[sprMAX];
	
	bottletype bottle_types[NUM_BOTTLE_TYPES];
	bottleshoptype bottle_shop_types[NUM_BOTTLE_SHOPS];
	
	byte miscsfx[sfxMAX];
};

extern DoorComboSet *DoorComboSets;
extern miscQdata QMisc;

#endif

