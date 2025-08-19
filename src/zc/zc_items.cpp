#include "base/qrs.h"
#include "zc/zelda.h"
#include "zc/guys.h"
#include "base/zdefs.h"
#include "zc/maps.h"
#include "items.h"
#include "zscriptversion.h"
#include <stdio.h>
#include "base/misctypes.h"

extern sprite_list  guys;
extern sprite_list  items;

bool addfairy(zfix x, zfix y, int32_t misc3, int32_t id)
{
    addenemy(cur_screen,x,y,eITEMFAIRY,id);
    ((enemy*)guys.spr(guys.Count()-1))->dstep=misc3;
    ((enemy*)guys.spr(guys.Count()-1))->step=(misc3/100.0);
    movefairy(x,y,id);
    return true;
}

bool addfairynew(zfix x, zfix y, int32_t misc3, item &itemfairy)
{
    addenemy(cur_screen,x,y,eITEMFAIRY,0);
    enemy *ptr = ((enemy*)guys.spr(guys.Count()-1));
    ptr->dstep=misc3;
    ptr->step=(misc3/100.0);
    itemfairy.fairyUID = ptr->getUID();
    if (get_qr(qr_FAIRYDIR)) ptr->dir = zc_rand(7);
    movefairynew(x,y,itemfairy);
    return true;
}

bool can_drop(zfix x, zfix y)
{
    return !(_walkflag(x,y+16,0) ||
		((!get_qr(qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS) && int32_t(y)%16==0) &&
		((checkSVLadderPlatform(x+4,y+16)) || (checkSVLadderPlatform(x+12,y+16)))));
}

int32_t select_dropitem(int32_t item_set)
{
    int32_t total_chance=0;
    
    for(int32_t k=0; k<11; ++k)
    {
        int32_t current_chance=item_drop_sets[item_set].chance[k];
        
        if(k>0)
        {
            int32_t current_item=item_drop_sets[item_set].item[k-1];
            
            if((!get_qr(qr_ENABLEMAGIC)||(game->get_maxmagic()<=0))&&(itemsbuf[current_item].type == itype_magic))
            {
                current_chance=0;
            }
            
            if((!get_qr(qr_TRUEARROWS))&&(itemsbuf[current_item].type == itype_arrowammo))
            {
                current_chance=0;
            }
			
			if(get_qr(qr_SMARTDROPS))
			{
				if(itemsbuf[current_item].amount > 0 && game->get_maxcounter(itemsbuf[current_item].count) == 0)
				{
					current_chance = 0;
				}
			}
			if(get_qr(qr_SMARTER_DROPS))
			{
				if(itemsbuf[current_item].amount > 0 && game->get_counter(itemsbuf[current_item].count) >= game->get_maxcounter(itemsbuf[current_item].count))
				{
					current_chance = 0;
				}
			}
        }
        
        total_chance+=current_chance;
    }
    
    if(total_chance==0)
        return -1;
        
    int32_t item_chance=(zc_oldrand()%total_chance)+1;
    
    int32_t drop_item=-1;
    
    for(int32_t k=10; k>=0; --k)
    {
    
        int32_t current_chance=item_drop_sets[item_set].chance[k];
        int32_t current_item=(k==0 ? -1 : item_drop_sets[item_set].item[k-1]);
        
        if((!get_qr(qr_ENABLEMAGIC)||(game->get_maxmagic()<=0))&&(current_item>=0&&itemsbuf[current_item].type == itype_magic))
        {
            current_chance=0;
        }
        
        if((!get_qr(qr_TRUEARROWS))&&(current_item>=0&&itemsbuf[current_item].type == itype_arrowammo))
        {
            current_chance=0;
        }
        
		if(get_qr(qr_SMARTDROPS))
		{
			if(itemsbuf[current_item].amount > 0 && game->get_maxcounter(itemsbuf[current_item].count) == 0)
			{
				current_chance = 0;
			}
		}
		
		if(get_qr(qr_SMARTER_DROPS)) //OH SHIT EMILY
		{											//DEEDEE 'BOUT TO DAB ON YOU
			if(itemsbuf[current_item].amount > 0 && game->get_counter(itemsbuf[current_item].count) >= game->get_maxcounter(itemsbuf[current_item].count))
			{
				current_chance = 0;	//Item droprate being set to 0 faster than I can chug an entire coffee (read: fast)
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
int32_t select_dropitem(int32_t item_set, int32_t x, int32_t y)
{
	int32_t drop_item = select_dropitem(item_set);
	
    if(drop_item>=0 && itemsbuf[drop_item].type==itype_fairy && !get_qr(qr_OLD_FAIRY_LIMIT))
    {
        for(int32_t j=0; j<items.Count(); ++j)
        {
            if((itemsbuf[items.spr(j)->id].type==itype_fairy)&&((abs(items.spr(j)->x-x)<32)||(abs(items.spr(j)->y-y)<32)))
            {
                drop_item=-1;
                break;
            }
        }
    }
	
	return drop_item;
}
int32_t item::run_script(int32_t mode)
{
	if(switch_hooked && !get_qr(qr_SWITCHOBJ_RUN_SCRIPT)) return RUNSCRIPT_OK;
	if (script <= 0 || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[susptITEMSPRITESCRIPTS])
		return RUNSCRIPT_OK;
	auto scrty = *get_scrtype();
	auto uid = getUID();
	if(!FFCore.doscript(scrty,uid))
		return RUNSCRIPT_OK;
	int32_t ret = RUNSCRIPT_OK;
	bool& waitdraw = FFCore.waitdraw(scrty, uid);
	switch(mode)
	{
		case MODE_NORMAL:
			return ZScriptVersion::RunScript(ScriptType::ItemSprite, script, uid);
		case MODE_WAITDRAW:
			if(waitdraw)
			{
				ret = ZScriptVersion::RunScript(ScriptType::ItemSprite, script, uid);
				waitdraw = false;
			}
			break;
	}
    return ret;
}


std::string bottle_name(size_t type)
{
	return std::string(QMisc.bottle_types[type-1].name);
}
std::string bottle_slot_name(size_t slot, std::string const& emptystr)
{
	size_t bind = game ? game->get_bottle_slot(slot) : 0;
	if(!bind)
		return emptystr;
	return bottle_name(bind);
}
