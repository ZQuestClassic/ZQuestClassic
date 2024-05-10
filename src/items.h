#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "sprite.h"
#include "base/zdefs.h"
#include "base/initdata.h"
#include "base/zfix.h"

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
int32_t get_progressive_item(itemdata const& itm, bool lastOwned = false);
bool checkmagiccost(int32_t itemid, bool checkTime = false);
void paymagiccost(int32_t itemid, bool ignoreTimer = false, bool onlyTimer = false);

class item : public sprite
{
public:
    int32_t pickup,clk2;
    int32_t aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
    int32_t screen_spawned;
    bool dummy;
    int32_t o_tile,o_cset, o_speed, o_delay, frames;
    int32_t fairyUID;
    word pstring; //pickup string
    word pickup_string_flags;
    //int32_t script_UID;
    int32_t family;
    byte lvl;
    int32_t linked_parent;
	bool is_dragged;
	int16_t from_dropset;
	int8_t pickupexstate;
	bool noSound;
	bool noHoldSound;

//Linker errors because this is shared with zquest. :( -Z
    #ifndef IS_EDITOR
    int32_t script_UID;
    int32_t getScriptUID();
    void setScriptUID(int32_t new_id);
    #endif
//
	void load_gfx(itemdata const& itm);

    item(zfix X,zfix Y,zfix Z,int32_t i,int32_t p,int32_t c, bool isDummy = false);
    virtual ~item();
    virtual bool animate(int32_t index);
    virtual void draw(BITMAP *dest);
#ifndef IS_EDITOR
	virtual int32_t run_script(int32_t mode);
#endif
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

enum
{
	BURNSPR_NONE,
	BURNSPR_ANY,
	BURNSPR_STRONG,
	BURNSPR_MAGIC,
	BURNSPR_DIVINE,
	BURNSPR_MAX
};
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
    int32_t flags;
#define ITEM_GAMEDATA           0x00000001  // Whether this item sets the corresponding gamedata value or not
#define ITEM_EDIBLE             0x00000002  // can be eaten by Like Like
#define ITEM_COMBINE            0x00000004  // blue potion + blue potion = red potion
#define ITEM_DOWNGRADE          0x00000008
#define ITEM_FLAG1              0x00000010
#define ITEM_FLAG2              0x00000020
#define ITEM_KEEPOLD            0x00000040
#define ITEM_RUPEE_MAGIC        0x00000080
#define ITEM_UNUSED             0x00000100
#define ITEM_GAINOLD            0x00000200
#define ITEM_FLAG3              0x00000400
#define ITEM_FLAG4              0x00000800
#define ITEM_FLAG5              0x00001000
#define ITEM_FLAG6              0x00002000
#define ITEM_FLAG7              0x00004000
#define ITEM_FLAG8              0x00008000
#define ITEM_FLAG9              0x00010000
#define ITEM_FLAG10             0x00020000
#define ITEM_FLAG11             0x00040000
#define ITEM_FLAG12             0x00080000
#define ITEM_FLAG13             0x00100000
#define ITEM_FLAG14             0x00200000
#define ITEM_FLAG15             0x00400000
#define ITEM_PASSIVESCRIPT      0x00800000
#define ITEM_VALIDATEONLY       0x01000000
#define ITEM_SIDESWIM_DISABLED  0x02000000
#define ITEM_BUNNY_ENABLED      0x04000000
#define ITEM_VALIDATEONLY2      0x08000000
#define ITEM_JINX_IMMUNE        0x10000000
#define ITEM_FLIP_JINX          0x20000000
#define ITEM_BURNING_SPRITES    0x40000000
    word script;												// Which script the item is using
    char count;
    word amount;
    int16_t setmax;
    word max;
    byte playsound;
    word collect_script;
//  byte exp[10];                                             // not used
    int32_t initiald[INITIAL_D];
    byte initiala[INITIAL_A];
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
	byte burnsprs[BURNSPR_MAX];
	byte light_rads[BURNSPR_MAX];
    byte useweapon; //lweapon id type -Z
    byte usedefence; //default defence type -Z
    int32_t weap_pattern[ITEM_MOVEMENT_PATTERNS]; //formation, arg1, arg2 -Z
    int32_t weaprange; //default range -Z
    int32_t weapduration; //default duration, 0 = infinite. 
 
    
    //To implement next;
    int32_t duplicates; //Number of duplicate weapons generated.
    int32_t wpn_misc_d[FFSCRIPT_MISC]; //THe initial Misc[d] that will be assiged to the weapon, 
    
    int32_t weap_initiald[INITIAL_D];
    byte weap_initiala[INITIAL_A];
    
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
    
    
  
//Guydata Enemy Editor Size Panel FLags

#define itemdataOVERRIDE_TILEWIDTH      0x00000001
#define itemdataOVERRIDE_TILEHEIGHT     0x00000002
#define itemdataOVERRIDE_HIT_WIDTH      0x00000004
#define itemdataOVERRIDE_HIT_HEIGHT     0x00000008
#define itemdataOVERRIDE_HIT_Z_HEIGHT   0x00000010
#define itemdataOVERRIDE_HIT_X_OFFSET   0x00000020
#define itemdataOVERRIDE_HIT_Y_OFFSET   0x00000040
#define itemdataOVERRIDE_DRAW_X_OFFSET  0x00000080
#define itemdataOVERRIDE_DRAW_Y_OFFSET  0x00000100
#define itemdataOVERRIDE_DRAW_Z_OFFSET  0x00000200

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
    byte sprite_initiala[INITIAL_A];
    word sprite_script;
	
	char display_name[256];
	
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
