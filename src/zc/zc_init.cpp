#include <stdio.h>

#include "base/gui.h"
#include "init.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "gui/jwin.h"
#include "base/zsys.h"
#include "gamedata.h"
#include "dialog/info.h"
#include "zc/hero.h"
#include "zc/zc_init.h"
#include "zc/cheats.h"

static std::string cheat_delta;

void zc_init_apply_cheat_delta()
{
	if (cheat_delta.size())
	{
		cheats_enqueue(Cheat::HeroData, -1, -1, cheat_delta);
		cheat_delta.clear();
	}
}

int32_t onCheatConsole()
{
	zinitdata *base = copyIntoZinit(game);
	zinitdata *zinit2;

	if (cheat_delta.size())
	{
		std::string error;
		zinit2 = apply_init_data_delta(base, cheat_delta, error);
		if (!zinit2)
			InfoDialog("Error applying init data delta", error).show();
	}
	else
	{
		zinit2 = new zinitdata;
		*zinit2 = *base;
	}

    int32_t rval = doInit(zinit2, true);
	std::string delta = serialize_init_data_delta(base, zinit2);
	if (delta.size())
		cheat_delta = delta;

	delete base;
	delete zinit2;

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
	zinit2->terminalv=zinit.terminalv;
	zinit2->jump_hero_layer_threshold=zinit.jump_hero_layer_threshold;
	for(int q = 0; q < SPRITE_THRESHOLD_MAX; ++q)
		zinit2->sprite_z_thresholds[q] = zinit.sprite_z_thresholds[q];
	zinit2->heroStep=zinit.heroStep;
	zinit2->shove_offset=zinit.shove_offset;
	zinit2->air_drag=zinit.air_drag;
	zinit2->subscrSpeed=zinit.subscrSpeed;
	zinit2->bomb_ratio = zinit.bomb_ratio;
	zinit2->hcp = gdata->get_HCpieces();
	
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
	zinit2->light_wave_rate = gdata->get_light_wave_rate();
	zinit2->light_wave_size = gdata->get_light_wave_size();
	zinit2->swimgravity = gdata->get_watergrav();
	zinit2->heroSideswimUpStep = gdata->get_sideswim_up();
	zinit2->heroSideswimSideStep = gdata->get_sideswim_side();
	zinit2->heroSideswimDownStep = gdata->get_sideswim_down();
	zinit2->exitWaterJump = gdata->get_sideswim_jump();
	zinit2->bunny_ltm = gdata->get_bunny_ltm();
	zinit2->switchhookstyle = gdata->get_switchhookstyle();
	zinit2->spriteflickerspeed = gdata->get_spriteflickerspeed();
	zinit2->spriteflickercolor = gdata->get_spriteflickercolor();
	zinit2->spriteflickertransp = gdata->get_spriteflickertransp();
	zinit2->region_mapping = gdata->get_regionmapping();
	zinit2->item_spawn_flicker = gdata->get_item_spawn_flicker();
	zinit2->item_timeout_dur = gdata->get_item_timeout_dur();
	zinit2->item_timeout_flicker = gdata->get_item_timeout_flicker();
	zinit2->item_flicker_speed = gdata->get_item_flicker_speed();
	
	for(int32_t q = 0; q < MAX_COUNTERS; ++q)
	{
		zinit2->counter[q] = gdata->get_counter(q);
		zinit2->mcounter[q] = gdata->get_maxcounter(q);
	}
	
	for(int32_t i=0; i<MAXLEVELS; i++)
		zinit2->litems[i] = gdata->lvlitems[i];
	zinit2->lvlswitches = gdata->lvlswitches;
	zinit2->level_keys = gdata->lvlkeys;
	for(uint q = 0; q < NUM_BOTTLE_SLOTS; ++q)
		zinit2->bottle_slot[q] = gdata->bottleSlots[q];
	
	zinit2->magicdrainrate = vbound(gdata->get_magicdrainrate(), 0, 255);
	zinit2->flags.set(INIT_FL_CANSLASH,gdata->get_canslash());
	
	zinit2->cont_heart = gdata->get_cont_hearts();
	zinit2->hcp_per_hc = gdata->get_hcp_per_hc();
	zinit2->flags.set(INIT_FL_CONTPERCENT,gdata->get_cont_percent());
	
	//now set up the items!
	for(int32_t i=0; i<MAXITEMS; i++)
	{
		zinit2->set_item(i, gdata->get_item(i));
	}
	
	zinit2->hero_swim_mult = gdata->swim_mult;
	zinit2->hero_swim_div = gdata->swim_div;
	
	zinit2->normalize();
	return zinit2;
}
