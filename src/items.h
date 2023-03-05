//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "sprite.h"
#include "base/zdefs.h"
#include "zfix.h"

class item;
extern char *item_string[ITEMCNT];

extern int32_t fairy_cnt;
void movefairy(zfix &x,zfix &y,int32_t misc);
void movefairynew(zfix &x,zfix &y,item const &itemfairy);
void killfairy(int32_t misc);
void killfairynew(item const &itemfairy);
int32_t addenemy(int32_t x,int32_t y,int32_t id,int32_t clk);
bool addfairy(zfix x, zfix y, int32_t misc3, int32_t id);
bool addfairynew(zfix x, zfix y, int32_t misc3, item &itemfairy);
bool can_drop(zfix x, zfix y);
void item_fall(zfix& x, zfix& y, zfix& fall);
int32_t item_pits(zfix& x, zfix& y, int32_t& fallclk);
int32_t select_dropitem(int32_t item_set);
int32_t select_dropitem(int32_t item_set, int32_t x, int32_t y);
int32_t get_progressive_item(itemdata const& itm, bool lastOwned = false);

class item : public sprite
{
public:
    int32_t pickup,clk2;
    int32_t aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
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
    //int32_t weapoverrideFLAGS; 
    
    //word weaponscript; //If only. -Z This would link an item to a weapon script in the item editor.
//Linker errors because this is shared with zquest. :( -Z
    #ifndef IS_ZQUEST
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
	virtual int32_t run_script(int32_t mode);
	virtual ALLEGRO_COLOR hitboxColor(byte opacity = 255) const;
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
#endif
/*** end of sprite.cc ***/

