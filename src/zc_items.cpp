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
#include "items.h"
#include "zscriptversion.h"
#include <stdio.h>

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
    return !(_walkflag(x,y+16,0) ||
		((!get_bit(quest_rules, qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS) && int(y)%16==0) &&
		((checkSVLadderPlatform(x+4,y+16)) || (checkSVLadderPlatform(x+12,y+16)))));
}

void item_fall(fix& x, fix& y, fix& fall)
{
	if(!get_bit(quest_rules, qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS) && checkSVLadderPlatform(x+4,y+(fall/100)+15))
	{
		y+=fall/100;
		y-=int(y)%16; //Fix to top of ladder
		fall = 0;
	}
	else
	{
		y+=fall/100;
		
		if((fall/100)==0 && fall>0)
			fall*=(fall>0 ? 2 : 0.5); // That oughta do something about the floatiness.
			
		if(fall <= (int)zinit.terminalv)
		{
			fall += zinit.gravity;
		}
	}
}

int select_dropitem(int item_set)
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
			
			if(get_bit(quest_rules, qr_SMARTDROPS))
			{
				if(itemsbuf[current_item].amount > 0 && game->get_maxcounter(itemsbuf[current_item].count) == 0)
				{
					current_chance = 0;
				}
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
        
		if(get_bit(quest_rules, qr_SMARTDROPS))
		{
			if(itemsbuf[current_item].amount > 0 && game->get_maxcounter(itemsbuf[current_item].count) == 0)
			{
				current_chance = 0;
			}
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
    
    return drop_item;
}
int select_dropitem(int item_set, int x, int y)
{
	int drop_item = select_dropitem(item_set);
	
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
int item::run_script(int mode)
{
	if (script <= 0 || !doscript || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[susptITEMSPRITESCRIPTS])
		return RUNSCRIPT_OK;
	int ret = RUNSCRIPT_OK;
	switch(mode)
	{
		case MODE_NORMAL:
			return ZScriptVersion::RunScript(SCRIPT_ITEMSPRITE, script, getUID());
		case MODE_WAITDRAW:
			if(waitdraw)
			{
				ret = ZScriptVersion::RunScript(SCRIPT_ITEMSPRITE, script, getUID());
				waitdraw = false;
			}
			break;
	}
    return ret;
}
/*** end of sprite.cc ***/

