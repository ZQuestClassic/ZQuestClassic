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
#include "zdefs.h"
#include "zfix.h"


extern char *item_string[ITEMCNT];

extern int fairy_cnt;
void movefairy(zfix &x,zfix &y,int misc);
void killfairy(int misc);
int addenemy(int x,int y,int id,int clk);
bool addfairy(zfix x, zfix y, int misc3, int id);
bool can_drop(zfix x, zfix y);
void item_fall(zfix& x, zfix& y, zfix& fall);
int select_dropitem(int item_set);
int select_dropitem(int item_set, int x, int y);

class item : public sprite
{
public:
    int pickup,clk2;
    int aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
    int o_tile,o_cset, o_speed, o_delay, frames;
    word pstring; //pickup string
    word pickup_string_flags;
    //int script_UID;
    int overrideFLAGS; //Override flags.
    long family;
    byte lvl;
    int linked_parent;
    //int weapoverrideFLAGS; 
    
    //word weaponscript; //If only. -Z This would link an item to a weapon script in the item editor.
//Linker errors because this is shared with zquest. :( -Z
    #ifndef IS_ZQUEST
    int script_UID;
    int getScriptUID();
    void setScriptUID(int new_id);
    #endif
//
    item(zfix X,zfix Y,zfix Z,int i,int p,int c, bool isDummy = false);
    virtual ~item();
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
	virtual int run_script(int mode);
};

// easy way to draw an item
void putitem(BITMAP *dest,int x,int y,int item_id);
void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash);
#endif
/*** end of sprite.cc ***/

