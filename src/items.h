#ifndef ITEMS_H_
#define ITEMS_H_

#include "base/zdefs.h"
#include "base/flags.h"
#include "base/initdata.h"
#include "base/zfix.h"
#include "sfx.h"
#include "sprite.h"

namespace flags {
// TODO: these generically named flags need to be renamed / documented.
enum item_flags : uint32_t
{
	item_none                = 0,
	item_gamedata            = F(0), // Whether this item sets the corresponding gamedata value or not
	item_edible              = F(1), // can be eaten by Like Like
	item_combine             = F(2), // blue potion + blue potion = red potion
	item_downgrade           = F(3),
	item_flag1               = F(4),
	item_flag2               = F(5),
	item_keep_old            = F(6),
	item_rupee_magic         = F(7),
	item_unused              = F(8),
	item_gain_old            = F(9),
	item_flag3               = F(10),
	item_flag4               = F(11),
	item_flag5               = F(12),
	item_flag6               = F(13),
	item_flag7               = F(14),
	item_flag8               = F(15),
	item_flag9               = F(16),
	item_flag10              = F(17),
	item_flag11              = F(18),
	item_flag12              = F(19),
	item_flag13              = F(20),
	item_flag14              = F(21),
	item_flag15              = F(22),
	item_passive_script      = F(23),
	item_validate_only       = F(24),
	item_sideswim_disabled   = F(25),
	item_bunny_enabled       = F(26),
	item_validate_only_2     = F(27),
	item_jinx_immune         = F(28),
	item_flip_jinx           = F(29),
	item_burning_sprites     = F(30),
};
} // ends namespace

class item;
struct itemdata;
extern char *item_string[MAXITEMS];

extern int32_t fairy_cnt;
bool addfairy(zfix x, zfix y, int32_t misc3, int32_t id);
bool addfairynew(zfix x, zfix y, int32_t misc3, item &itemfairy);
bool can_drop(zfix x, zfix y);
void item_fall(zfix& x, zfix& y, zfix& fall);
int32_t item_pits(zfix& x, zfix& y, int32_t& fallclk);
int32_t select_dropitem(int32_t item_set);
int32_t select_dropitem(int32_t item_set, int32_t x, int32_t y);
int32_t get_progressive_item(int32_t itmid, bool lastOwned = false);
bool itembundle_safe(int32_t itmid, bool skipError = false);
bool checkmagiccost(int32_t itemid, bool checkTime = false);
void paymagiccost(int32_t itemid, bool ignoreTimer = false, bool onlyTimer = false);

class item : public sprite
{
public:
    int32_t pickup,clk2;
    int32_t aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
    bool dummy;
    int32_t o_speed, o_delay, frames;
    int32_t fairyUID;
    word pstring; //pickup string
    word pickup_string_flags;
    int32_t family;
    byte lvl;
    int32_t linked_parent;
	bool is_dragged;
	int16_t from_dropset;
	int8_t pickupexstate;
	bool noSound;
	bool noHoldSound;

	void load_gfx(itemdata const& itm);

    item(zfix X,zfix Y,zfix Z,int32_t i,int32_t p,int32_t c, bool isDummy = false);
    virtual ~item();
    virtual bool animate(int32_t index);
    virtual void draw(BITMAP *dest);
	virtual int32_t run_script(int32_t mode);
	virtual optional<ScriptType> get_scrtype() const {return ScriptType::ItemSprite;}
	virtual ALLEGRO_COLOR hitboxColor(byte opacity = 255) const;
	virtual void draw_hitbox();
	void set_forcegrab(bool val)
	{
		if(force_grab && !val)
			--ignore_delete;
		else if(!force_grab && val)
			++ignore_delete;
		force_grab = val;
		if(val)
		{
			fallclk = 0;
			drownclk = 0;
		}
	}
	
	bool get_forcegrab() {return force_grab;}
	
private:
	bool force_grab;
};

// easy way to draw an item
void putitem(BITMAP *dest,int32_t x,int32_t y,int32_t item_id);
void putitem2(BITMAP *dest,int32_t x,int32_t y,int32_t item_id, int32_t &aclk, int32_t &aframe, int32_t flash);
void putitem3(BITMAP *dest,int32_t x,int32_t y,int32_t item_id, int32_t clk);
void dummyitem_animate(item* dummy, int32_t clk);

enum
{
	ITM_ADVP_NAME,
	ITM_ADVP_DISP_NAME,
	ITM_ADVP_ITMCLASS,
	ITM_ADVP_EQUIPMENTITM,
	ITM_ADVP_ATTRIBS,
	ITM_ADVP_GENFLAGS,
	ITM_ADVP_TYPEFLAGS,
	ITM_ADVP_USECOSTS,
	ITM_ADVP_USESFX,
	ITM_ADVP_PICKUP,
	ITM_ADVP_PICKUPSTRS,
	ITM_ADVP_PICKUPFLAGS,
	ITM_ADVP_WEAPONDATA,
	ITM_ADVP_GRAPHICS,
	ITM_ADVP_TILEMOD,
	ITM_ADVP_SPRITES,
	ITM_ADVP_ITEMSIZE,
	ITM_ADVP_WEAPONSIZE,
	ITM_ADVP_ITEMSCRIPTS,
	ITM_ADVP_WEAPONSCRIPT,
};

//Now itemdata lives here too!


struct itemdata
{
    int32_t tile;
    byte misc_flags;                                                // 0000vhtf (vh:flipping, t:two hands, f:flash)
    byte csets;                                               // ffffcccc (f:flash cset, c:cset)
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte delay;                                               // extra delay factor (-1) for first frame
    int32_t ltm;                                                 // Hero Tile Modifier
    int32_t family;												// What family the item is in
    byte fam_type;	//level										// What type in this family the item is
    int32_t power;	// Damage, height, etc. //changed from byte to int32_t in V_ITEMS 31
    item_flags flags;
    word script;												// Which script the item is using
    char count;
    word amount;
    int16_t setmax;
    word max;
    byte playsound = WAV_SCALE;
    word collect_script;
//  byte exp[10];                                             // not used
    int32_t initiald[INITIAL_D];
    byte wpn;
    byte wpn2;
    byte wpn3;
    byte wpn4;
    byte wpn5;
    byte wpn6;
    byte wpn7;
    byte wpn8;
    byte wpn9;
    byte wpn10;
    byte pickup_hearts;
    int32_t misc1;
    int32_t misc2;
    int32_t misc3;
    int32_t misc4;
    int32_t misc5;
    int32_t misc6;
    int32_t misc7;
    int32_t misc8;
    int32_t misc9;
    int32_t misc10;
	int16_t cost_amount[2]; // Magic usage!
    byte usesound, usesound2;
	byte burnsprs[WPNSPR_MAX];
	byte light_rads[WPNSPR_MAX];
    byte useweapon; //lweapon id type -Z
    byte usedefense; //default defense type -Z
    
    //To implement next;
    int32_t weap_pattern[ITEM_MOVEMENT_PATTERNS]; //formation, arg1, arg2 -Z
    int32_t weaprange; //default range -Z
    int32_t weapduration; //default duration, 0 = infinite. 
    int32_t duplicates; //Number of duplicate weapons generated.
    int32_t wpn_misc_d[FFSCRIPT_MISC]; //THe initial Misc[d] that will be assiged to the weapon,     
    int32_t weap_initiald[INITIAL_D];
    
    byte drawlayer;
    int32_t collectflags;
    int32_t hxofs, hyofs, hxsz, hysz, hzsz, xofs, yofs; //item
    int32_t weap_hxofs, weap_hyofs, weap_hxsz, weap_hysz, weap_hzsz, weap_xofs, weap_yofs; //weapon
    int32_t tilew, tileh, weap_tilew, weap_tileh; //New for 2.54
    int32_t pickup; byte pickupflag;
    
#define itemdataPSTRING_ALWAYS		0x00000001
#define itemdataPSTRING_IP_HOLDUP	0x00000002
#define itemdataPSTRING_NOMARK		0x00000004
    word pstring;
    word pickup_string_flags;
    int32_t overrideFLAGS; //Override flags.
    int32_t weapoverrideFLAGS; 
    
    word weaponscript; //If only. -Z This would link an item to a weapon script in the item editor.
    int32_t wpnsprite; //enemy weapon sprite. 
    int32_t magiccosttimer[2]; 
    char cost_counter[2];
    
    char initD_label[8][65];
    char weapon_initD_label[8][65];
    char sprite_initD_label[8][65];
    
    int32_t sprite_initiald[INITIAL_D];
    word sprite_script;
	
	char display_name[256];
	
	byte pickup_litems;
	int16_t pickup_litem_level = -1;
	
	std::string get_name(bool init = false, bool plain = false) const;
	//helper functions because stupid shit
	int32_t misc(size_t ind) const
	{
		switch(ind)
		{
			case 0: return misc1;
			case 1: return misc2;
			case 2: return misc3;
			case 3: return misc4;
			case 4: return misc5;
			case 5: return misc6;
			case 6: return misc7;
			case 7: return misc8;
			case 8: return misc9;
			case 9: return misc10;
		}
		return 0;
	}
	void misc(size_t ind, int32_t val)
	{
		switch(ind)
		{
			case 0: misc1 = val; break;
			case 1: misc2 = val; break;
			case 2: misc3 = val; break;
			case 3: misc4 = val; break;
			case 4: misc5 = val; break;
			case 5: misc6 = val; break;
			case 6: misc7 = val; break;
			case 7: misc8 = val; break;
			case 8: misc9 = val; break;
			case 9: misc10 = val; break;
		}
		return;
	}
	void advpaste(itemdata const& other, bitstring const& pasteflags);
};

//some methods for dealing with items
int32_t getItemFamily(itemdata *items, int32_t item);
void removeItemsOfFamily(gamedata *g, itemdata *items, int32_t family);
void removeItemsOfFamily(zinitdata *i, itemdata *items, int32_t family);
void removeLowerLevelItemsOfFamily(gamedata *g, itemdata *items, int32_t family, int32_t level);
int32_t getHighestLevelOfFamily(zinitdata *source, itemdata *items, int32_t family);
int32_t getHighestLevelOfFamily(gamedata *source, itemdata *items, int32_t family, bool checkenabled = false);
int32_t getHighestLevelEvenUnowned(itemdata *items, int32_t family);
int32_t getItemID(itemdata *items, int32_t family, int32_t level);
int32_t getCanonicalItemID(itemdata *items, int32_t family);
int32_t getItemIDPower(itemdata *items, int32_t family, int32_t power);
void addOldStyleFamily(zinitdata *dest, itemdata *items, int32_t family, char levels);

std::string bottle_name(size_t type);
std::string bottle_slot_name(size_t slot, std::string const& emptystr);
#endif
