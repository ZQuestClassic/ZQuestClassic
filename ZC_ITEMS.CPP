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
//-------------------------------------------------------+
// This program is free software; you can redistribute it and/or modify it under the terms of the
// modified version 3 of the GNU General Public License. See License.txt for details.


/**********************************/
/**********  Item Class  **********/
/**********************************/

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first
#include "items.h"

#include "guys.h"
#include "link.h"
#include "maps.h"
#include "zdefs.h"
#include "zelda.h"
#include "zc_sys.h"
#include <stdio.h>

extern sprite_list  guys;
extern sprite_list  items;


inline bool hasEnoughHearts(int id)
{
    return game->get_maxlife()>=itemsbuf[id].pickup_hearts*HP_PER_HEART;
}

bool item::canPickUp(LinkClass* link)
{
    if((pickup&ipTIMER)!=0 && clk2<32 && id!=iFairyMoving)
        // Recently dropped non-fairy; still flashing
        return false;
    
    if((pickup&ipENEMY)!=0) // item was being carried by enemy
        if(more_carried_items()<=1) // I don't know what's going on here...
            hasitem&=~2;
    
    if((pickup&ipDUMMY)!=0) // Dummy item; can't be picked up
    {
        // But it may still do something.
        // This should be moved, but it'll have to wait...
        if((pickup&ipMONEY)!=0)
        {
           // if(onGetDummyMoney)
           //     onGetDummyMoney();
        }
        
        return false;
    }
    
    const itemdata& data=itemsbuf[id];
    
    if(data.pickup_hearts>0)
    {
        if((get_bit(quest_rules,qr_HEARTSREQUIREDFIX) || (pickup&ipSPECIAL)) &&
          !hasEnoughHearts(id))
            return false;
    }
    
    if(game->get_spendable_rupies()<price)
        // Doesn't check for infinite wallet; is that correct?
        return false;
    
    // There's still more to be done here...
    
    return true;
}

int item::getUpgradeResult() const
{
    const itemdata& data=itemsbuf[id];
    int nextItem=-1;
    
    if((data.flags&ITEM_COMBINE)!=0 && current_item(data.family)==data.fam_type)
    {
        for(int i=0; i<MAXITEMS; i++)
        {
            // Find the item in the same family with the least greater fam_type
            if(itemsbuf[i].family==data.family &&
              itemsbuf[i].fam_type>data.fam_type)
            {
                if(nextItem==-1 ||
                  itemsbuf[i].fam_type<=itemsbuf[nextItem].fam_type)
                    nextItem=i;
            }
        }
    }
    
    return nextItem;
}

bool addfairy(fix x, fix y, int misc3, int id)
{
    addenemy(x,y,eITEMFAIRY,id);
    ((enemy*)guys.spr(guys.Count()-1))->dstep=misc3;
    ((enemy*)guys.spr(guys.Count()-1))->step=(misc3/100.0);
    movefairy(x,y,id);
    return true;
}

bool can_drop(fix x, fix y)
{
    return !_walkflag(x,y+16,0);
}

int select_dropitem(int item_set, int x, int y)
{
    int total_chance=0;
    
    for(int k=0; k<11; ++k)
    {
        int current_chance=item_drop_sets[item_set].chance[k];
        
        if(k>0)
        {
            int current_item=item_drop_sets[item_set].item[k-1];
            
            if((!get_bit(quest_rules,qr_ENABLEMAGIC)||(game->get_maxmagic()<=0))&&(itemsbuf[current_item].family == itype_magic))
            {
                current_chance=0;
            }
            
            if((!get_bit(quest_rules,qr_TRUEARROWS))&&(itemsbuf[current_item].family == itype_arrowammo))
            {
                current_chance=0;
            }
        }
        
        total_chance+=current_chance;
    }
    
    if(total_chance==0)
        return -1;
        
    int item_chance=(rand()%total_chance)+1;
    
    int drop_item=-1;
    
    for(int k=10; k>=0; --k)
    {
    
        int current_chance=item_drop_sets[item_set].chance[k];
        int current_item=(k==0 ? -1 : item_drop_sets[item_set].item[k-1]);
        
        if((!get_bit(quest_rules,qr_ENABLEMAGIC)||(game->get_maxmagic()<=0))&&(current_item>=0&&itemsbuf[current_item].family == itype_magic))
        {
            current_chance=0;
        }
        
        if((!get_bit(quest_rules,qr_TRUEARROWS))&&(current_item>=0&&itemsbuf[current_item].family == itype_arrowammo))
        {
            current_chance=0;
        }
        
        if(current_chance>0&&item_chance<=current_chance)
        {
            drop_item=current_item;
            break;
        }
        else
        {
            item_chance-=current_chance;
        }
    }
    
    if(drop_item>=0 && itemsbuf[drop_item].family==itype_fairy)
    {
        for(int j=0; j<items.Count(); ++j)
        {
            if((itemsbuf[items.spr(j)->id].family==itype_fairy)&&((abs(items.spr(j)->x-x)<32)||(abs(items.spr(j)->y-y)<32)))
            {
                drop_item=-1;
                break;
            }
        }
    }
    
    return drop_item;
}

bool is_side_view()
{
    return (tmpscr->flags7&fSIDEVIEW)!=0;
}
/*** end of sprite.cc ***/

