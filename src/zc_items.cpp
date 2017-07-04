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

/**********************************/
/**********  Item Class  **********/
/**********************************/

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include "zelda.h"
#include "guys.h"
#include "zdefs.h"
#include "maps.h"
#include <stdio.h>
#include "items.h"

extern sprite_list  guys;
extern sprite_list  items;

/*
  void movefairy(fix &x,fix &y,int misc) {
  return;
  }

  void killfairy(int misc) {
  return;
  }
  */
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

ItemDefinitionRef select_dropitem(int item_set, int x, int y)
{
    int total_chance=0;
    
    for(int k=0; k<11; ++k)
    {
        int current_chance=item_drop_sets[item_set].chance[k];
        
        if(k>0)
        {
            ItemDefinitionRef current_item=item_drop_sets[item_set].item[k-1];
            if (curQuest->isValid(current_item))
            {
                if ((!get_bit(quest_rules, qr_ENABLEMAGIC) || (game->get_maxmagic() <= 0)) && (curQuest->getItemDefinition(current_item).family == itype_magic))
                {
                    current_chance = 0;
                }

                if ((!get_bit(quest_rules, qr_TRUEARROWS)) && (curQuest->getItemDefinition(current_item).family == itype_arrowammo))
                {
                    current_chance = 0;
                }
            }
        }
        
        total_chance+=current_chance;
    }
    
    if(total_chance==0)
        return ItemDefinitionRef();
        
    int item_chance=(rand()%total_chance)+1;
    
    ItemDefinitionRef drop_item;
    
    for(int k=10; k>=0; --k)
    {
    
        int current_chance=item_drop_sets[item_set].chance[k];
        ItemDefinitionRef current_item=(k==0 ? ItemDefinitionRef() : item_drop_sets[item_set].item[k-1]);
        
        if((!get_bit(quest_rules,qr_ENABLEMAGIC)||(game->get_maxmagic()<=0)) && 
            (curQuest->isValid(current_item) && curQuest->getItemDefinition(current_item).family == itype_magic))
        {
            current_chance=0;
        }
        
        if((!get_bit(quest_rules,qr_TRUEARROWS)) &&
            (curQuest->isValid(current_item) && curQuest->getItemDefinition(current_item).family == itype_arrowammo))
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
    
    if(curQuest->isValid(drop_item) && curQuest->getItemDefinition(drop_item).family==itype_fairy)
    {
        for(int j=0; j<items.Count(); ++j)
        {
            if((curQuest->getItemDefinition(((item *)items.spr(j))->itemDefinition).family==itype_fairy)
                && ((abs(items.spr(j)->x-x)<32)||(abs(items.spr(j)->y-y)<32)))
            {
                drop_item=ItemDefinitionRef();
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

