#include "core/qrs.h"
#include "zc/zelda.h"
#include "zc/guys.h"
#include "core/zdefs.h"
#include "zc/maps.h"
#include "items.h"
#include "zc/zscriptversion.h"
#include <stdio.h>
#include "core/misctypes.h"
#include "zinfo.h"

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

bool can_drop(item const* itm)
{
	if (itm->hardcoded_sideview_hitbox())
	{
		return !(_walkflag(itm->x,itm->y+16,0) ||
			((!get_qr(qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS) && int32_t(itm->y)%16==0) &&
			((checkSVLadderPlatform(itm->x+4,itm->y+16)) || (checkSVLadderPlatform(itm->x+12,itm->y+16)))));
	}
	return !itm->on_sideview_solid();
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
			auto const& itm = get_item_data(current_item);
            
            if((!get_qr(qr_ENABLEMAGIC) || game->get_maxmagic() <= 0) && itm.type == itype_magic)
                current_chance=0;
            
            if(!get_qr(qr_TRUEARROWS) && itm.type == itype_arrowammo)
                current_chance=0;
			
			if(get_qr(qr_SMARTDROPS))
				if(itm.amount > 0 && game->get_maxcounter(itm.count) == 0)
					current_chance = 0;
			if(get_qr(qr_SMARTER_DROPS))
				if(itm.amount > 0 && game->get_counter(itm.count) >= game->get_maxcounter(itm.count))
					current_chance = 0;
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
		auto const& itm = get_item_data(current_item);
        
        if((!get_qr(qr_ENABLEMAGIC) || game->get_maxmagic() <= 0) && itm.type == itype_magic)
            current_chance=0;
        
        if(!get_qr(qr_TRUEARROWS) && itm.type == itype_arrowammo)
            current_chance=0;
        
		if(get_qr(qr_SMARTDROPS))
			if(itm.amount > 0 && game->get_maxcounter(itm.count) == 0)
				current_chance = 0;
		
		if(get_qr(qr_SMARTER_DROPS))
			if(itm.amount > 0 && game->get_counter(itm.count) >= game->get_maxcounter(itm.count))
				current_chance = 0;
		
        if(current_chance > 0 && item_chance <= current_chance)
        {
            drop_item = current_item;
            break;
        }
        else
        {
            item_chance -= current_chance;
        }
    }
    
    return drop_item;
}
int32_t select_dropitem(int32_t item_set, int32_t x, int32_t y)
{
	int32_t drop_item = select_dropitem(item_set);
	
    if(valid_item_id(drop_item) && itemsbuf.get(drop_item).type==itype_fairy && !get_qr(qr_OLD_FAIRY_LIMIT))
    {
        for(int32_t j=0; j<items.Count(); ++j)
        {
            if((itemsbuf.get(items.spr(j)->id).type==itype_fairy)&&((abs(items.spr(j)->x-x)<32)||(abs(items.spr(j)->y-y)<32)))
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
	if (script <= 0 || script >= NUMSCRIPTSITEMSPRITE || FFCore.getQuestHeaderInfo(vZelda) < 0x255 || FFCore.system_suspend[susptITEMSPRITESCRIPTS])
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

bool handle_ambush_item(int id, int screen, zfix x, zfix y)
{
	itemdata const* data = &get_item_data(id);
	if (data->type == itype_progressive_itm)
	{
		int32_t id2 = get_progressive_item(id);
		if (valid_item_id(id2) && id2 != id)
		{
			data = &get_item_data(id2);
			id = id2;
		}
	}
	if (data->type != itype_enemy_ambush)
		return false;
	
	int count = data->misc1;
	vector<int> choices = {
		data->misc6, data->misc7, data->misc8,
		data->misc9, data->misc10
	};
	for (auto it = choices.begin(); it != choices.end();)
	{
		if (!*it || unsigned(*it) >= MAXGUYS)
			it = choices.erase(it);
		else ++it;
	}
	string s = fmt::format("{} item '{}' ({})", ZI.getItemClassName(itype_enemy_ambush), id, data->name);
	if (count <= 0)
		Z_error("%s had invalid enemy count!", s.c_str());
	else if (choices.empty())
		Z_error("%s had no valid enemies to spawn!", s.c_str());
	else
	{
		int actual_count = 0;
		for(int q = 0; q < count; ++q)
		{
			int choice;
			if (data->flags & item_flag2)
				choice = q % choices.size();
			else
				choice = zc_rand(choices.size()-1);
			if (addenemy(screen, x, y, choices[choice], -10))
				++actual_count;
		}
		Z_eventlog("%s spawned %d enemies!\n", s.c_str(), actual_count);
	}
	return true;
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
