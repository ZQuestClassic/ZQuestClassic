#include <map>
#include <vector>
#include <algorithm>
#include <string>

#include <stdio.h>

#include "base/zc_alleg.h"
#include "gui/jwin.h"
#include "base/zdefs.h"
#include "base/dmap.h"
#include "base/zsys.h"
#include "base/gui.h"
#include "init.h"
#include "items.h"
#include "base/qrs.h"
#include "zc/zelda.h"
#include "zinfo.h"
#include <fmt/format.h>
#include <fmt/ranges.h>

void getitem(int32_t id, bool nosound, bool doRunPassive);

//InitData

#include "dialog/init_data.h"
int32_t doInit(zinitdata *local_zinit, bool isZC)
{
	call_init_dlg(*local_zinit, isZC);
    return D_O_K;
}

// NOTE: This method has been severely hacked to fix an annoying problem at game start:
// items (ie the Small Wallet) which modify max counter values need to be processed after
// the values for those counters specified in init data, as the author expects these items
// to modify the max counter. BUT the counter value should NOT be updated, ie, starting with
// the bomb item does not give 8 free bombs at quest start.
// I don't like this solution one bit, but can't come up with anything better -DD
void resetItems(gamedata *game2, zinitdata *zinit2, bool freshquest)
{
	game2->set_maxlife(zinit2->mcounter[crLIFE]);
	game2->set_maxbombs(zinit2->mcounter[crBOMBS]);
	game2->set_maxcounter(zinit2->mcounter[crBOMBS]/zc_max(1,zinit2->bomb_ratio), crSBOMBS);
	game2->set_maxmagic(zinit2->mcounter[crMAGIC]);
	game2->set_maxarrows(zinit2->mcounter[crARROWS]);
	game2->set_maxcounter(zinit2->mcounter[crMONEY], crMONEY);
	game2->set_maxcounter(zinit2->mcounter[crKEYS], crKEYS);
	
	for(int32_t q = 0; q < 100; ++q)
		game2->set_maxcounter(zinit2->mcounter[q+crCUSTOM1], q+crCUSTOM1);
	
	//set up the items
	game2->items_off.clear();
	game2->items_owned.normalize();
	size_t sz = zc_max(zinit2->items.length(), game2->items_owned.length());
	for(size_t q = 0; q < sz; ++q)
	{
		if(zinit2->get_item(q) && (get_item_data(q).flags & item_gamedata))
		{
#ifndef IS_EDITOR
			if (!game2->get_item(q))
				getitem(q,true,false);
#else
			game2->set_item_no_flush(q, true);
#endif
		}
		else
			game2->set_item_no_flush(q,false);
	}
	// Fix them DMap items
	// Since resetItems() gets called before AND after init_dmap()...
	if (get_currdmap() > -1)
		game2->items_off = DMaps[get_currdmap()].disabled_items;
	
	flushItemCache();
	
	//Then set up the counters
	game2->set_life(zc_max(1,zinit2->counter[crLIFE]));
	game2->set_bombs(zinit2->counter[crBOMBS]);
	
	if(zinit2->counter[crBOMBS] > 0 && zinit2->mcounter[crBOMBS] > 0) game2->set_item(iBombs, true);
	
	game2->set_keys(zinit2->counter[crKEYS]);
	game2->set_sbombs(zinit2->counter[crSBOMBS]);
	
	if(zinit2->counter[crSBOMBS] > 0 && (zinit2->mcounter[crBOMBS] /zc_max(1,zinit2->bomb_ratio)) > 0) game2->set_item(iSBomb, true);
	
	game2->set_HCpieces(zinit2->hcp);
	game2->set_rupies(zinit2->counter[crMONEY]);
	game2->set_hcp_per_hc(zinit2->hcp_per_hc);
	game2->set_cont_hearts(zinit2->cont_heart);
	game2->set_cont_percent(zinit2->flags.get(INIT_FL_CONTPERCENT));
	game2->set_hp_per_heart(zinit2->hp_per_heart);
	game2->set_mp_per_block(zinit2->magic_per_block);
	game2->set_hero_dmgmult(zinit2->hero_damage_multiplier);
	game2->set_regionmapping(zinit2->region_mapping);
	game2->set_item_spawn_flicker(zinit2->item_spawn_flicker);
	game2->set_item_timeout_dur(zinit2->item_timeout_dur);
	game2->set_item_timeout_flicker(zinit2->item_timeout_flicker);
	game2->set_item_flicker_speed(zinit2->item_flicker_speed);
	game2->set_ene_dmgmult(zinit2->ene_damage_multiplier);
	game2->set_dither_type(zinit2->dither_type);
	game2->set_dither_arg(zinit2->dither_arg);
	game2->set_dither_perc(zinit2->dither_percent);
	game2->set_light_rad(zinit2->def_lightrad);
	game2->set_transdark_perc(zinit2->transdark_percent);
	game2->set_darkscr_color(zinit2->darkcol);
	game2->set_light_wave_rate(zinit2->light_wave_rate);
	game2->set_light_wave_size(zinit2->light_wave_size);
	game2->set_watergrav(zinit2->swimgravity);
	game2->set_sideswim_up(zinit2->heroSideswimUpStep);
	game2->set_sideswim_side(zinit2->heroSideswimSideStep);
	game2->set_sideswim_down(zinit2->heroSideswimDownStep);
	game2->set_sideswim_jump(zinit2->exitWaterJump);
	game2->set_bunny_ltm(zinit2->bunny_ltm);
	game2->set_switchhookstyle(zinit2->switchhookstyle);
	game2->set_spriteflickerspeed(zinit2->spriteflickerspeed);
	game2->set_spriteflickercolor(zinit2->spriteflickercolor);
	game2->set_spriteflickertransp(zinit2->spriteflickertransp);
	
	for(int32_t i=0; i<MAXLEVELS; i++)
		game2->lvlitems[i] = zinit2->litems[i];
	game2->lvlswitches = zinit2->lvlswitches;
	game2->lvlkeys = zinit2->level_keys;
	for(uint q = 0; q < NUM_BOTTLE_SLOTS; ++q)
		game2->bottleSlots[q] = zinit2->bottle_slot[q];
	
	game2->set_magic(zc_min(zinit2->counter[crMAGIC],zinit2->mcounter[crMAGIC]));
	game2->set_magicdrainrate(zinit2->magicdrainrate);
	game2->set_canslash(zinit2->flags.get(INIT_FL_CANSLASH)?1:0);
	
	game2->set_arrows(zinit2->counter[crARROWS]);
	
	for(int32_t q = 0; q < 100; ++q)
		game2->set_counter(zinit2->counter[q+crCUSTOM1], q+ crCUSTOM1);
	
	if(freshquest)
	{
		game2->gen_doscript = zinit2->gen_doscript;
		game2->gen_exitState = zinit2->gen_exitState;
		game2->gen_reloadState = zinit2->gen_reloadState;
		game2->gen_initd = zinit2->gen_initd;
		game2->gen_data = zinit2->gen_data;
		game2->gen_eventstate = zinit2->gen_eventstate;
		game2->screen_data = zinit2->screen_data;
	}
	
	game2->swim_mult = zinit2->hero_swim_mult;
	game2->swim_div = zinit2->hero_swim_div;
	game2->normalize();
	//flush the cache again (in case bombs became illegal to use by setting bombs to 0)
	flushItemCache();
}

template<std::size_t N, class T>
constexpr std::size_t countof(T(&)[N]) { return N; }

#define LIST_PROPS \
	ZFIXPROP(air_drag) \
	PROP(bomb_ratio) \
	PROP(bunny_ltm) \
	PROP(cont_heart) \
	PROP(darkcol) \
	PROP(def_lightrad) \
	PROP(dither_arg) \
	PROP(dither_percent) \
	PROP(dither_type) \
	PROP(ene_damage_multiplier) \
	PROP(exitWaterJump) \
	PROP(gravity) \
	PROP(hcp) \
	PROP(hcp_per_hc) \
	PROP(hero_damage_multiplier) \
	PROP(hero_swim_div) \
	PROP(hero_swim_mult) \
	PROP(hero_swim_speed) \
	PROP(heroAnimationStyle) \
	PROP(heroSideswimDownStep) \
	PROP(heroSideswimSideStep) \
	PROP(heroSideswimUpStep) \
	PROP(heroStep) \
	PROP(hp_per_heart) \
	PROP(item_spawn_flicker) \
	PROP(item_timeout_dur) \
	PROP(item_timeout_flicker) \
	PROP(item_flicker_speed) \
	PROP(jump_hero_layer_threshold) \
	PROP(last_map) \
	PROP(last_screen) \
	PROP(light_wave_rate) \
	PROP(light_wave_size) \
	PROP(magic_per_block) \
	PROP(magicdrainrate) \
	PROP(msg_more_is_offset) \
	PROP(msg_more_x) \
	PROP(msg_more_y) \
	PROP(msg_speed) \
	PROP(region_mapping) \
	ZFIXPROP(shove_offset) \
	PROP(ss_bbox_1_color) \
	PROP(ss_bbox_2_color) \
	PROP(ss_flags) \
	PROP(ss_grid_color) \
	PROP(ss_grid_x) \
	PROP(ss_grid_xofs) \
	PROP(ss_grid_y) \
	PROP(ss_grid_yofs) \
	PROP(start_dmap) \
	PROP(subscrSpeed) \
	PROP(swimgravity) \
	PROP(switchhookstyle) \
	PROP(spriteflickerspeed) \
	PROP(spriteflickercolor) \
	PROP(spriteflickertransp) \
	PROP(terminalv) \
	PROP(transdark_percent)

#define LIST_ARRAY_PROPS \
	ARRAY_PROP(bottle_slot) \
	ARRAY_PROP(counter) \
	BITSTR_PROP(gen_doscript) \
	VEC_PROP(gen_eventstate) \
	VEC_PROP(gen_exitState) \
	VEC_PROP(gen_reloadState) \
	VEC_PROP_2D(gen_initd) \
	VEC_PROP_2D(gen_data) \
	BITSTR_PROP(items) \
	VEC_PROP(level_keys) \
	VEC_PROP(lvlswitches) \
	ARRAY_PROP(litems) \
	ARRAY_PROP(mcounter) \
	ARRAY_PROP(sprite_z_thresholds) \
	BITSTR_PROP(flags)

static int DELTA_VERSION = 2;
string serialize_init_data_delta(zinitdata *base, zinitdata *changed)
{
	vector<string> tokens;
	tokens.push_back(fmt::format("VERSION={}",DELTA_VERSION));
	
	#define PROP(name) if (base->name != changed->name) \
		tokens.push_back(fmt::format("{}={}", #name, (int)changed->name));
	#define ZFIXPROP(name) if(base->name != changed->name) \
		tokens.push_back(fmt::format("{}={}", #name, (int)changed->name.getZLong()));
	LIST_PROPS;
	#undef ZFIXPROP
	#undef PROP
	
	#define ARRAY_PROP(name) \
		for (int i = 0; i < countof(base->name); i++) \
			if (base->name[i] != changed->name[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)changed->name[i]));
	#define VEC_PROP(name) \
		for (int i = 0; i < base->name.size(); i++) \
			if (base->name[i] != changed->name[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)changed->name[i]));
	#define VEC_PROP_2D(name) \
		for (int i = 0; i < base->name.size(); i++) \
			for (int j = 0; j < base->name[i].size(); j++) \
				if (base->name[i][j] != changed->name[i][j]) \
					tokens.push_back(fmt::format("{}[{}][{}]={}", #name, i, j, int(changed->name[i][j])));
	#define BITSTR_PROP(name) \
	{ \
		auto& bvec = base->name.inner(); \
		auto& cvec = changed->name.inner(); \
		auto sz = zc_max(bvec.capacity(), cvec.capacity()); \
		for (int i = 0; i < sz; ++i) \
			if(bvec[i] != cvec[i]) \
				tokens.push_back(fmt::format("{}[{}]={}", #name, i, (int)cvec[i])); \
	}
	LIST_ARRAY_PROPS;
	#undef BITSTR_PROP
	#undef VEC_PROP_2D
	#undef VEC_PROP
	#undef ARRAY_PROP
	
	return fmt::format("{}", fmt::join(tokens, " "));
}

// TODO use out param instead of allocating new zinitdata
zinitdata *apply_init_data_delta(zinitdata *base, string delta, string& out_error)
{
	zinitdata *result = new zinitdata(*base);
	if (delta.empty())
		return result;

	vector<string> tokens;
	util::split(delta, tokens, ' ');
	
	#define FAIL_IF(x, y) if (x) { \
		out_error = y; \
		delete result; \
		return nullptr; \
	}
	
	dword delta_version = 0;
	
	for (string token : tokens)
	{
		vector<string> kv;
		util::split(token, kv, '=');
		FAIL_IF(kv.size() != 2, fmt::format("invalid token '{}': expected one =", token));

		errno = 0;
		int as_int = std::strtol(kv[1].data(), nullptr, 10);
		FAIL_IF(errno, fmt::format("invalid token '{}': expected integer", token));
		
		if(kv[0] == "VERSION")
		{
			delta_version = as_int;
			continue;
		}
		bool is_array = kv[0].find('[') != string::npos;
		string key = kv[0];
		vector<int> index;
		if(is_array)
		{
			vector<string> name_index;
			util::split(kv[0], name_index, '[');

			errno = 0;
			for(int q = 1; q < name_index.size(); ++q)
				index.push_back(std::strtol(name_index[q].data(), nullptr, 10));
			
			FAIL_IF(errno, fmt::format("invalid token '{}': expected integer", token));
			key = name_index[0];
		}
		
		if(delta_version < DELTA_VERSION) //compat
		{
			#define DEPRPROP_IGNORE(name) if(key == #name) \
				continue;
			#define ARRAY_DEPRPROP_IOFS(old,new,indoffs) if (key == #old) \
			{ \
				FAIL_IF(index[0]+indoffs >= countof(result->new), fmt::format("invalid token '{}': integer too big", token)); \
				result->new[index[0]+indoffs] = as_int; \
				continue; \
			}
			#define DEPRPROP(old,new) if(key == #old) \
			{ \
				result->new = as_int; \
				continue; \
			}
			#define DEPRPROP_OFFS(old,new,op,offs) if(key == #old) \
			{ \
				result->new = as_int op offs; \
				continue; \
			}
			#define DEPRPROP_LITEM_BIT(old,flag) if(key == #old) \
			{ \
				for(int q = 0; q < 8; ++q) \
					SETFLAG(result->litems[index[0]+q], flag, get_bitl(as_int, index[0]+q)); \
				continue; \
			}
			if(delta_version < 1)
			{
				if(is_array)
				{
					if (key == "items")
					{
						FAIL_IF(index[0] >= 256, fmt::format("invalid token '{}': integer too big", token));
						result->set_item(index[0], as_int);
						continue;
					}
					if (key == "misc")
					{
						if(index[0] == 0)
							result->flags.set(INIT_FL_CONTPERCENT,as_int);
						else if(index[0] == 2)
							result->flags.set(INIT_FL_CANSLASH,as_int);
						continue;
					}
					if (key == "gen_doscript")
					{
						FAIL_IF(index[0] >= NUMSCRIPTSGENERIC, fmt::format("invalid token '{}': integer too big", token));
						result->gen_doscript.set(index[0], as_int);
						continue;
					}
					ARRAY_DEPRPROP_IOFS(scrcnt,counter,crCUSTOM1);
					ARRAY_DEPRPROP_IOFS(scrmaxcnt,mcounter,crCUSTOM1);
				}
				else
				{
					DEPRPROP(super_bombs,counter[crSBOMBS]);
					DEPRPROP_OFFS(start_heart,counter[crLIFE],*,result->hp_per_heart);
					if (key == "triforce")
					{
						for(int q = 0; q < 8; ++q)
							SETFLAG(result->litems[q+1], (1 << li_mcguffin), get_bitl(as_int,q));
						continue;
					}
					DEPRPROP(rupies,counter[crMONEY]);
					DEPRPROP(keys,counter[crKEYS]);
					DEPRPROP(arrows,counter[crARROWS]);
					DEPRPROP(bombs,counter[crBOMBS]);
					DEPRPROP_OFFS(hc,counter[crLIFE],*,result->hp_per_heart);
					DEPRPROP(magic,counter[crMAGIC]);
					DEPRPROP(max_arrows,mcounter[crARROWS]);
					DEPRPROP(max_bombs,mcounter[crBOMBS]);
					DEPRPROP(max_keys,mcounter[crKEYS]);
					DEPRPROP(max_magic,mcounter[crMAGIC]);
					DEPRPROP(max_rupees,mcounter[crMONEY]);
					DEPRPROP(max_sbombs,mcounter[crSBOMBS]);
					DEPRPROP(gravity2,gravity);
					
					DEPRPROP_IGNORE(subscreen);
					DEPRPROP_IGNORE(subscreen_style);
					DEPRPROP_IGNORE(usecustomsfx);
				}
			}
			if(delta_version < 2)
			{
				DEPRPROP_LITEM_BIT(map, (1 << li_map))
				DEPRPROP_LITEM_BIT(compass, (1 << li_compass))
				DEPRPROP_LITEM_BIT(boss_key, (1 << li_boss_key))
				DEPRPROP_LITEM_BIT(mcguffin, (1 << li_mcguffin))
			}
			#undef DEPRPROP_LITEM_BIT
			#undef DEPRPROP_OFFS
			#undef DEPRPROP
			#undef ARRAY_DEPRPROP_IOFS
			#undef DEPRPROP_IGNORE
		}
		
		// Current
		if(is_array)
		{
			#define ARRAY_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= countof(result->name), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]] = as_int; \
				continue; \
			}
			#define VEC_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= result->name.size(), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]] = as_int; \
				continue; \
			}
			#define VEC_PROP_2D(name) if (key == #name) \
			{ \
				FAIL_IF(index.size() < 2, fmt::format("invalid token '{}': expected 2 indeces, got {}", token, index.size())); \
				FAIL_IF(index[0] >= result->name.size() \
					|| index[1] >= result->name[index[0]].size(), fmt::format("invalid token '{}': integer too big", token)); \
				result->name[index[0]][index[1]] = as_int; \
				continue; \
			}
			#define BITSTR_PROP(name) if (key == #name) \
			{ \
				FAIL_IF(index[0] >= 65536, fmt::format("invalid token '{}': integer too big", token)); \
				result->name.inner()[index[0]] = as_int; \
				continue; \
			}
			LIST_ARRAY_PROPS
			#undef BITSTR_PROP
			#undef VEC_PROP_2D
			#undef VEC_PROP
			#undef ARRAY_PROP
		}
		else
		{
			#define PROP(name) if (key == #name) \
			{ \
				result->name = as_int; \
				continue; \
			}
			#define ZFIXPROP(name) if (key == #name) \
			{ \
				result->name = zslongToFix(as_int); \
				continue; \
			}
			LIST_PROPS
			#undef ZFIXPROP
			#undef PROP
		}
		
		FAIL_IF(true, fmt::format("invalid token '{}': unknown property", token));
	}

	return result;
}

