//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include <stdio.h>

#include "base/gui.h"
#include "init.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "jwin.h"
#include "base/zsys.h"
#include "gamedata.h"
#include "hero.h"
#include "zc_init.h"
#include "cheats.h"
#include <fmt/format.h>

int32_t onCheatConsole()
{
    zinitdata *zinit2 = copyIntoZinit(game);
	zinitdata base = *zinit2;
    int32_t rval = doInit(zinit2, true);
	std::string delta = serialize_init_data_delta(&base, zinit2);
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

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }

#define LIST_PROPS \
	PROP(arrows); \
	PROP(bomb_ratio); \
	PROP(bombs); \
	PROP(bunny_ltm); \
	PROP(cont_heart); \
	PROP(darkcol); \
	PROP(def_lightrad); \
	PROP(dither_arg); \
	PROP(dither_percent); \
	PROP(dither_type); \
	PROP(ene_damage_multiplier); \
	PROP(exitWaterJump); \
	PROP(gravity); \
	PROP(gravity2); \
	PROP(hc); \
	PROP(hcp_per_hc); \
	PROP(hcp); \
	PROP(hero_damage_multiplier); \
	PROP(heroSideswimDownStep); \
	PROP(heroSideswimSideStep); \
	PROP(heroSideswimUpStep); \
	PROP(heroStep); \
	PROP(hp_per_heart); \
	PROP(jump_hero_layer_threshold); \
	PROP(keys); \
	PROP(magic_per_block); \
	PROP(magic); \
	PROP(magicdrainrate); \
	PROP(max_arrows); \
	PROP(max_bombs); \
	PROP(max_keys); \
	PROP(max_magic); \
	PROP(max_rupees); \
	PROP(max_sbombs); \
	PROP(rupies); \
	PROP(start_heart); \
	PROP(subscrSpeed); \
	PROP(super_bombs); \
	PROP(swimgravity); \
	PROP(switchhookstyle); \
	PROP(terminalv); \
	PROP(transdark_percent); \
	PROP(triforce) \
	PROP(hero_swim_mult) \
	PROP(hero_swim_div)

#define LIST_ARRAY_PROPS \
	ARRAY_PROP(boss_key); \
	ARRAY_PROP(compass); \
	ARRAY_PROP(items); \
	ARRAY_PROP(level_keys); \
	ARRAY_PROP(map); \
	ARRAY_PROP(misc); \
	ARRAY_PROP(scrcnt); \
	ARRAY_PROP(scrmaxcnt)

std::string serialize_init_data_delta(zinitdata *base, zinitdata *changed)
{
	std::vector<std::string> tokens;
	
	#define PROP(name) if (base->name != changed->name) \
		tokens.push_back(fmt::format("{}={}", #name, (int)changed->name));
	LIST_PROPS;
	#undef PROP

	#define ARRAY_PROP(name) \
		for (int i = 0; i < countof(base->name); i++) \
			if (base->name[i] != changed->name[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)changed->name[i]))
	LIST_ARRAY_PROPS;
	#undef ARRAY_PROP

	return fmt::format("{}", fmt::join(tokens, " "));
}

// https://stackoverflow.com/a/5888676/2788187
static size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

zinitdata *apply_init_data_delta(zinitdata *base, std::string delta)
{
	zinitdata *result = new zinitdata(*base);

	std::vector<std::string> tokens;
	split(delta, tokens, ' ');
	
	#define FAIL_IF(x) if (x) { delete result; return nullptr; }

	for (std::string token : tokens)
	{
		std::vector<std::string> kv;
		split(token, kv, '=');
		FAIL_IF(kv.size() != 2);

		errno = 0;
		int as_int = std::strtol(kv[1].data(), nullptr, 10);
		FAIL_IF(errno);

		if (kv[0].find('[') != kv[0].npos)
		{
			std::vector<std::string> name_index;
			split(kv[0], name_index, '[');

			errno = 0;
			int index = std::strtol(name_index[1].data(), nullptr, 10);
			FAIL_IF(errno);

			#define ARRAY_PROP(name) if (name_index[0] == #name) \
			{ \
				FAIL_IF(index >= countof(result->name)); \
				result->name[index] = as_int; \
			} else
			LIST_ARRAY_PROPS;
			#undef LIST_ARRAY_PROPS
	
			continue;
		}

		#define PROP(name) if (kv[0] == #name) \
			result->name = as_int; else
		LIST_PROPS;
		#undef PROP
	}

	return result;
}

