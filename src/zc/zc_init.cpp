#include <stdio.h>

#include "base/gui.h"
#include "init.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "jwin.h"
#include "base/zsys.h"
#include "gamedata.h"
#include "zc/hero.h"
#include "zc/zc_init.h"
#include "zc/cheats.h"

int32_t onCheatConsole()
{
    zinitdata *zinit2 = copyIntoZinit(game);
	zinitdata base = *zinit2;
    int32_t rval = doInit(zinit2, true);
	std::string delta = serialize_init_data_delta(&base, zinit2);
	if (delta.size())
		cheats_enqueue(Cheat::PlayerData, -1, -1, delta);
    // resetItems(game, zinit2, false);
    delete zinit2;
    // ringcolor(false);
    return rval;
}

void onInitOK()
{
}

// copyIntoZinit: worst kludge in all of ZC history? I've seen worse. ;) -Gleeok
zinitdata *copyIntoZinit(gamedata *gdata)
{
    zinitdata *zinit2 = new zinitdata;
    //populate it
    zinit2->gravity=zinit.gravity;
    zinit2->gravity2=zinit.gravity2;
    zinit2->terminalv=zinit.terminalv;
    zinit2->jump_hero_layer_threshold=zinit.jump_hero_layer_threshold;
    zinit2->heroStep=zinit.heroStep;
    zinit2->shove_offset=zinit.shove_offset;
    zinit2->subscrSpeed=zinit.subscrSpeed;
    zinit2->hc = gdata->get_maxlife()/gdata->get_hp_per_heart();
    zinit2->bombs = gdata->get_bombs();
    zinit2->keys = gdata->get_keys();
    zinit2->max_bombs = gdata->get_maxbombs();
    zinit2->super_bombs = gdata->get_sbombs();
    zinit2->max_sbombs = gdata->get_sbombs();
    zinit2->bomb_ratio = zinit.bomb_ratio;
    zinit2->hcp = gdata->get_HCpieces();
    zinit2->rupies = gdata->get_rupies();
    zinit2->max_bombs = gdata->get_maxbombs();
    zinit2->arrows = gdata->get_arrows();
    zinit2->max_arrows = gdata->get_maxarrows();
	
	zinit2->hp_per_heart = gdata->get_hp_per_heart();
	zinit2->magic_per_block = gdata->get_mp_per_block();
	zinit2->hero_damage_multiplier = gdata->get_hero_dmgmult();
	zinit2->ene_damage_multiplier = gdata->get_ene_dmgmult();
	zinit2->dither_type = gdata->get_dither_type();
	zinit2->dither_arg = gdata->get_dither_arg();
	zinit2->dither_percent = gdata->get_dither_perc();
	zinit2->def_lightrad = gdata->get_light_rad();
	zinit2->transdark_percent = gdata->get_transdark_perc();
	zinit2->darkcol = gdata->get_darkscr_color();
	zinit2->swimgravity = gdata->get_watergrav();
	zinit2->heroSideswimUpStep = gdata->get_sideswim_up();
	zinit2->heroSideswimSideStep = gdata->get_sideswim_side();
	zinit2->heroSideswimDownStep = gdata->get_sideswim_down();
	zinit2->exitWaterJump = gdata->get_sideswim_jump();
	zinit2->bunny_ltm = gdata->get_bunny_ltm();
	zinit2->switchhookstyle = gdata->get_switchhookstyle();
	
	for(int32_t q = 0; q < 25; ++q)
	{
		zinit2->scrcnt[q] = gdata->get_counter(q+7);
		zinit2->scrmaxcnt[q] = gdata->get_maxcounter(q+7);
	}
    
    for(int32_t i=0; i<MAXLEVELS; i++)
    {
        set_bit(zinit2->map, i, (gdata->lvlitems[i] & liMAP) ? 1 : 0);
        set_bit(zinit2->compass, i, (gdata->lvlitems[i] & liCOMPASS) ? 1 : 0);
        set_bit(zinit2->boss_key, i, (gdata->lvlitems[i] & liBOSSKEY) ? 1 : 0);
        zinit2->level_keys[i] = gdata->lvlkeys[i];
    }
    
    for(int32_t i=0; i<8; i++)
    {
        set_bit(&zinit2->triforce,i,(gdata->lvlitems[i+1]&liTRIFORCE) ? 1 : 0);
    }
    
    zinit2->max_magic = gdata->get_maxmagic();
    zinit2->magic = gdata->get_magic();
    
	zinit2->magicdrainrate = vbound(gdata->get_magicdrainrate(), 0, 255);
    set_bit(zinit2->misc, idM_CANSLASH, gdata->get_canslash());
    
    zinit2->arrows = gdata->get_arrows();
    zinit2->max_arrows = gdata->get_maxarrows();
    
    zinit2->max_rupees = gdata->get_maxcounter(1);
    zinit2->max_keys = gdata->get_maxcounter(5);
    
    zinit2->start_heart = gdata->get_life()/gdata->get_hp_per_heart();
    zinit2->cont_heart = gdata->get_cont_hearts();
    zinit2->hcp_per_hc = gdata->get_hcp_per_hc();
    set_bit(zinit2->misc,idM_CONTPERCENT,gdata->get_cont_percent() ? 1 : 0);
    
    //now set up the items!
    for(int32_t i=0; i<MAXITEMS; i++)
    {
        zinit2->items[i] = gdata->get_item(i);
    }
	
	zinit2->hero_swim_mult = gdata->swim_mult;
	zinit2->hero_swim_div = gdata->swim_div;
    
    return zinit2;
}
