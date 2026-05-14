#include "base/qrs.h"
#include "zc/zelda.h"
#include "zc/replay_compat.h"
#include "zc/guys.h"
#include "base/zdefs.h"
#include "zc/maps.h"
#include "items.h"
#include "zscriptversion.h"
#include <stdio.h>
#include "base/misctypes.h"

extern FFScript FFCore;
extern sprite_list  guys;
extern sprite_list  items;

bool addfairy(zfix x, zfix y, int32_t misc3, int32_t id)
{
    addenemy(x,y,eITEMFAIRY,id);
    ((enemy*)guys.spr(guys.Count()-1))->dstep=misc3;
    ((enemy*)guys.spr(guys.Count()-1))->step=(misc3/100.0);
    movefairy(x,y,id);
    return true;
}

bool addfairynew(zfix x, zfix y, int32_t misc3, item &itemfairy)
{
    addenemy(x,y,eITEMFAIRY,0);
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

void item_fall(zfix& x, zfix& y, zfix& fall)
{
	if(!get_qr(qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS) && checkSVLadderPlatform(x+4,y+(fall/100)+15))
	{
		y+=fall/100;
		y-=int32_t(y)%16; //Fix to top of ladder
		fall = 0;
	}
	else
	{
		y+=fall/100;
		
		if((fall/100)==0 && fall>0)
			fall*=(fall>0 ? 2 : 0.5); // That oughta do something about the floatiness.
			
		if(fall <= (int32_t)zinit.terminalv)
		{
			fall += (zinit.gravity / 100);
		}
	}
}

int select_dropitem(int item_set)
{
	auto const& dropset = item_drop_sets[item_set];
	int chances[11] = {0};
	int total_chance = 0;

	for(int k = 0; k < 11; ++k)
	{
		int current_chance = dropset.chance[k];
		if (!current_chance) continue;

		if (k > 0) // 0 is nothing chance
		{
			int current_item = dropset.item[k-1];
			auto const& itm = itemsbuf[current_item];
			bool remove = false;

			// Remove magic if magic disabled
			if ((!get_qr(qr_ENABLEMAGIC) || game->get_maxmagic() <= 0) && itm.family == itype_magic)
				remove = true;
			// Remove arrows if arrows disabled
			else if (!get_qr(qr_TRUEARROWS) && itm.family == itype_arrowammo)
				remove = true;
			// remove items that increment a counter with 0 max
			else if (get_qr(qr_SMARTDROPS)
				&& itm.amount > 0 && game->get_maxcounter(itm.count) == 0)
					remove = true;
			// remove items that increment an already full counter
			else if (get_qr(qr_SMARTER_DROPS)
				&& itm.amount > 0 && game->get_counter(itm.count) >= game->get_maxcounter(itm.count))
					remove = true;

			if (remove)
			{
				// removed items become 'Nothing'
				if (get_qr(qr_SMARTDROPS_NOTHING))
				{
					// Add their chance to the nothing chance
					// before clearing the chance to 0
					chances[0] += current_chance;
					total_chance += current_chance;
				}
				current_chance = 0;
			}
		}

		chances[k] = current_chance;
		total_chance += current_chance;
	}

	if (replay_compat_dropset_reroll_rng_bug())
	{
		// older replays: still consume an rng roll below even when only 'Nothing'
		// remains, so only early-exit when there's no chance at all
		if (total_chance == 0)
			return -1;
	}
	// only nothing chance, so early exit with nothing
	else if (total_chance <= chances[0])
		return -1;

	int rolled_chance = zc_oldrand() % total_chance; // 0 to n-1

	for (int k = 10; k > 0; --k)
	{
		if (chances[k])
		{
			if (rolled_chance < chances[k])
				return dropset.item[k-1];
			else
				rolled_chance -= chances[k];
		}
	}

	return -1;
}
int32_t select_dropitem(int32_t item_set, int32_t x, int32_t y)
{
	int32_t drop_item = select_dropitem(item_set);
	
    if(drop_item>=0 && itemsbuf[drop_item].family==itype_fairy && !get_qr(qr_OLD_FAIRY_LIMIT))
    {
        for(int32_t j=0; j<items.Count(); ++j)
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
