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
#include <boost/bind.hpp>

class LinkClass;

extern char *item_string[ITEMCNT];

extern int fairy_cnt;
void movefairy(fix &x,fix &y,int misc);
void killfairy(int misc);
int addenemy(int x,int y,int id,int clk);
bool addfairy(fix x, fix y, int misc3, int id);
bool can_drop(fix x, fix y);
int select_dropitem(int item_set, int x, int y);
bool is_side_view();

class item : public sprite
{
public:
    int pickup,clk2;
    int aclk,aframe;
    char PriceIndex;
    bool flash,twohand,anim, subscreenItem;
    int o_tile,o_cset, o_speed, o_delay, frames;
    
    bool alsoHaveBow; // Used in subscreens
    
    item(int id);
    item(fix X,fix Y,fix Z,int i,int p,int c, bool isDummy = false);
    virtual ~item();

    void setPickupFlags(int newPickup);
    virtual bool animate(int index);
    virtual void draw(BITMAP *dest);
    
    /// Determines whether Link is able to collect an item.
    /** \note ZC only
     */
    bool canPickUp(LinkClass* link);
    
    /// Finds the item that will be added when an upgrading item is collected.
    /** \return The ID of the resulting item, or -1 if this is not
     *          an upgrade item or no suitable upgrade could be found.
     *  \note ZC only
     */
    int getUpgradeResult() const;
    
	//TODO: These are broken. There should be a less clever way to do this.
    // public for now...
	void* onGetDummyMoney; // ..WHY??
    void* onPickUp;
    
    inline void setPrice(int newPrice)
    {
        price=newPrice;
    }
    
protected:
	void init(fix X,fix Y,fix Z,int i,int p,int c, bool isDummy = false);

private:
    int price;
};

// easy way to draw an item
void putitem(BITMAP *dest,int x,int y,int item_id);
void putitem2(BITMAP *dest,int x,int y,int item_id, int &aclk, int &aframe, int flash);
#endif
/*** end of sprite.cc ***/

