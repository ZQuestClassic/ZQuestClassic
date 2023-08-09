#include "zc_list_data.h"
#include "base/dmap.h"
#include "zq/zquest.h"
#include "qst.h"
#include "zinfo.h"
#include "base/misctypes.h"
#include <fmt/format.h>

extern zcmodule moduledata;
extern char *weapon_string[];
extern char *sfx_string[];
extern char *item_string[];
extern char *guy_string[eMAXGUYS];
extern const char *old_guy_string[OLDMAXGUYS];
extern item_drop_object item_drop_sets[MAXITEMDROPSETS];

#ifdef IS_PARSER
#elif defined(IS_PLAYER)
#define customtunes tunes
extern zctune tunes[MAXMIDIS];
#elif defined(IS_ZQUEST)
extern zctune *customtunes;
const char *msgslist(int32_t index, int32_t *list_size);
char *MsgString(int32_t index, bool show_number, bool pad_number);
int32_t addtomsglist(int32_t index, bool allow_numerical_sort = true);
#endif

extern std::string msgfont_str[font_max];
const char *ssfont2_str[] =
{
	"Zelda NES", "SS 1", "SS 2", "SS 3", "SS 4", "BS Time", "Small", "Small Prop.", "LttP Small", "Link's Awakening", "Link to the Past",
	"Goron", "Zoran", "Hylian 1", "Hylian 2", "Hylian 3", "Hylian 4", "Proportional", "Oracle", "Oracle Proportional", "Phantom", "Phantom Proportional",
	"Atari 800", "Acorn", "ADOS", "Allegro", "Apple II", "Apple II 80 Column", "Apple IIgs", "Aquarius",
	"Atari 400", "C64", "C64 HiRes", "IBM CGA", "COCO Mode I", "COCO Mode II", "Coupe", "Amstrad CPC",
	"Fantasy Letters", "FDS Katakana", "FDSesque", "FDS Roman", "FF", "Elder Futhark", "Gaia", "Hira",
	"JP Unsorted", "Kong", "Mana", "Mario", "Mot CPU", "MSX Mode 0", "MSX Mode 1", "PET", "Homebrew",
	"Mr. Saturn", "Sci-Fi", "Sherwood", "Sinclair QL", "Spectrum", "Spectrum Large", "TI99", "TRS",
	"Zelda 2", "ZX", "Lisa", ""
};
const char *icounter_str2[sscMAX]=
{
    "Rupees", "Bombs", "Super Bombs", "Arrows", "Gen. Keys w/Magic", "Gen. Keys w/o Magic", "Level Keys w/Magic",
    "Level Keys w/o Magic", "Any Keys w/Magic", "Any Keys w/o Magic", "Custom 1", "Custom 2", "Custom 3", "Custom 4",
    "Custom 5", "Custom 6", "Custom 7", "Custom 8", "Custom 9", "Custom 10", "Custom 11", "Custom 12", "Custom 13",
    "Custom 14", "Custom 15", "Custom 16", "Custom 17", "Custom 18", "Custom 19", "Custom 20", "Custom 21",
    "Custom 22", "Custom 23", "Custom 24", "Custom 25", "Life", "Magic", "Max Life", "Max Magic",
	"Custom 26", "Custom 27", "Custom 28", "Custom 29", "Custom 30",
	"Custom 31", "Custom 32", "Custom 33", "Custom 34", "Custom 35",
	"Custom 36", "Custom 37", "Custom 38", "Custom 39", "Custom 40",
	"Custom 41", "Custom 42", "Custom 43", "Custom 44", "Custom 45",
	"Custom 46", "Custom 47", "Custom 48", "Custom 49", "Custom 50",
	"Custom 51", "Custom 52", "Custom 53", "Custom 54", "Custom 55",
	"Custom 56", "Custom 57", "Custom 58", "Custom 59", "Custom 60",
	"Custom 61", "Custom 62", "Custom 63", "Custom 64", "Custom 65",
	"Custom 66", "Custom 67", "Custom 68", "Custom 69", "Custom 70",
	"Custom 71", "Custom 72", "Custom 73", "Custom 74", "Custom 75",
	"Custom 76", "Custom 77", "Custom 78", "Custom 79", "Custom 80",
	"Custom 81", "Custom 82", "Custom 83", "Custom 84", "Custom 85",
	"Custom 86", "Custom 87", "Custom 88", "Custom 89", "Custom 90",
	"Custom 91", "Custom 92", "Custom 93", "Custom 94", "Custom 95",
	"Custom 96", "Custom 97", "Custom 98", "Custom 99", "Custom 100"
};


const char *shadowstyle_str[sstsMAX] =
{
    "None", "Shadow", "Shadow (U)", "Shadow (O)", "Shadow (+)", "Shadow (X)", "Shadowed", "Shadowed (U)", "Shadowed (O)", "Shadowed (+)", "Shadowed (X)"
};

static bool skipchar(char c)
{
	return c == 0 || c == '-';
}

GUI::ListData GUI::ZCListData::fonts(bool ss_fonts, bool numbered, bool sorted)
{
	map<std::string, int32_t> ids;
	std::set<std::string> names;
	std::vector<std::string> strings;
	if(ss_fonts)
	{
		for(auto q = 0; ssfont2_str[q][0]; ++q)
		{
			char const* fontname = ssfont2_str[q];
			std::string name;
			if(numbered)
				name = fmt::format("{} ({:03})", fontname, q);
			else name = fontname;
			
			if(sorted)
			{
				ids[name] = q;
				names.insert(name);
			}
			else strings.push_back(name);
		}
	}
	else for(auto q = 0; q < font_max; ++q)
	{
		char const* fontname = msgfont_str[q].c_str();
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", fontname, q);
		else name = fontname;
		
		if(sorted)
		{
			ids[name] = q;
			names.insert(name);
		}
		else strings.push_back(name);
	}
	if(!sorted)
		return GUI::ListData(strings);
	
	GUI::ListData ls;
	for(auto it = names.begin(); it != names.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

static const GUI::ListData combostrs
{
	{ "-17: Screen->D[7]", -17 },
	{ "-16: Screen->D[6]", -16 },
	{ "-15: Screen->D[5]", -15 },
	{ "-14: Screen->D[4]", -14 },
	{ "-13: Screen->D[3]", -13 },
	{ "-12: Screen->D[2]", -12 },
	{ "-11: Screen->D[1]", -11 },
	{ "-10: Screen->D[0]", -10 },
	{ " -2: Screen Catchall", -2 },
	{ " -1: Screen Message String", -1 }
};
GUI::ListData GUI::ZCListData::strings(bool combostr, bool respect_order, bool numbered)
{
	GUI::ListData ls;
	#ifdef IS_ZQUEST
	if(combostr)
		ls = combostrs;
	
	std::map<int,int> poses;
	for(int q = 0; q < msg_count; ++q)
	{
		if(respect_order)
		{
			int listpos = addtomsglist(q, false);
			poses[q] = listpos;
		}
		else poses[q] = q;
	}
	for(auto it = poses.begin(); it != poses.end(); ++it)
	{
		int val = it->second;
		std::string name(MsgString(val, numbered, numbered));
		ls.add(name,val);
	}
	#endif
	return ls;
}

GUI::ListData GUI::ZCListData::ss_counters()
{
	GUI::ListData ls;
	
	ls.add("(None)", -1);
	for(int32_t q = 0; q < sscMAX; ++q)
	{
		char const* module_str = icounter_str2[q];
		std::string name(module_str);
		ls.add(name, q);
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::shadow_types()
{
	std::vector<std::string> strings;

	for(auto q = 0; q < sstsMAX; ++q)
	{
		strings.push_back(shadowstyle_str[q]);
	}

	return GUI::ListData(strings);
}

static const GUI::ListData aligns
{
	{ "Left", 0 },
	{ "Center", 1 },
	{ "Right", 2 }
};

GUI::ListData const& GUI::ZCListData::alignments()
{
	return aligns;
}

static const GUI::ListData button
{
	{ "A", 0 },
	{ "B", 1 },
	{ "X", 2 },
	{ "Y", 3 }
};

GUI::ListData const& GUI::ZCListData::buttons()
{
	return button;
}

GUI::ListData GUI::ZCListData::enemies(bool numbered, bool defaultFilter)
{
	map<std::string, int32_t> ids;
	std::set<std::string> names;
	
	for(int32_t q=1; q < eMAXGUYS; ++q)
	{
		if(defaultFilter)
		{
			if(q >= 11 && q <= 19) //Segment components
				continue;
			if(q < OLDMAXGUYS && old_guy_string[q][strlen(old_guy_string[q])-1]==' ')
				continue; //'Hidden' enemies
			if(guysbuf[q].family == eeGUY)
				continue; //Guys
		}
		char const* npcname = guy_string[q];
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", npcname, q);
		else name = npcname;
		
		ids[name] = q;
		names.insert(name);
	}
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	for(auto it = names.begin(); it != names.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::items(bool numbered)
{
	map<std::string, int32_t> ids;
	std::set<std::string> names;
	
	for(int32_t q=0; q < MAXITEMS; ++q)
	{
		char const* itname = item_string[q];
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", itname, q);
		else name = itname;
		
		ids[name] = q;
		names.insert(name);
	}
	
	GUI::ListData ls;
	ls.add("(None)", -1);
	for(auto it = names.begin(); it != names.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::dropsets(bool numbered, bool none)
{
	map<std::string, int32_t> ids;
	std::set<std::string> names;
	
	for(int32_t q=0; q < MAXITEMDROPSETS; ++q)
	{
		char const* dropname = item_drop_sets[q].name;
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", dropname, q);
		else name = dropname;
		
		ids[name] = q;
		names.insert(name);
	}
	
	GUI::ListData ls;
	if(none)
		ls.add("(None)", -1);
	for(auto it = names.begin(); it != names.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::itemclass(bool numbered, bool zero_none)
{
	map<std::string, int32_t> fams;
	std::set<std::string> famnames;
	
	for(int32_t i=zero_none?1:0; i<itype_max; ++i)
	{
		if(!ZI.isUsableItemclass(i))
			continue; //Hidden
		char const* itname = ZI.getItemClassName(i);
        if(i < itype_last || itname[0])
		{
			std::string name;
			if(numbered)
				name = fmt::format("{} ({:03})", itname, i);
			else name = itname;
			
			fams[name] = i;
			famnames.insert(name);
		}
		else 
		{
			std::string name;
			if(numbered)
				name = fmt::format("zz{:03} ({:03})", i, i);
			else name = fmt::format("zz{:03}",i);
			
			fams[name] = i;
			famnames.insert(name);
		}
	}
	
	GUI::ListData ls;
	if(zero_none)
		ls.add("(None)", 0);
	for(auto it = famnames.begin(); it != famnames.end(); ++it)
	{
		ls.add(*it, fams[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::combotype(bool numbered, bool skipNone)
{
	GUI::ListData ls;
	map<std::string, int32_t> types;
	std::set<std::string> typenames;

	if(!skipNone) ls.add("(None)", 0);
	for(int32_t i=1; i<cMAX; ++i)
	{
		if(!ZI.isUsableComboType(i))
			continue; //Hidden
		char const* module_str = ZI.getComboTypeName(i);
		if(module_str[0])
		{
			std::string name;
			if(numbered)
				name = fmt::format("{} ({:03})", module_str, i);
			else name = module_str;
			
			types[name] = i;
			typenames.insert(name);
		}
		else 
		{
			std::string name;
			if(numbered)
				name = fmt::format("zz{:03} ({:03})", i, i);
			else name = fmt::format("zz{:03}",i);
			
			types[name] = i;
			typenames.insert(name);
		}
	}

	for(auto it = typenames.begin(); it != typenames.end(); ++it)
	{
		ls.add(*it, types[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::mapflag(int32_t numericalFlags, bool numbered, bool skipNone)
{
	GUI::ListData ls;
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	if(!skipNone) ls.add("(None)", 0);
	for(int32_t q = 1; q < mfMAX; ++q)
	{
		if(!ZI.isUsableMapFlag(q))
			continue; //Hidden
		char const* module_str = ZI.getMapFlagName(q);
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", module_str, q);
		else name = module_str;
		
		if (numericalFlags)
			ls.add(name, q);
		else
		{
			vals[name] = q;
			names.insert(name);
		}
	}
	if (!numericalFlags)
	{
		for(auto it = names.begin(); it != names.end(); ++it)
		{
			ls.add(*it, vals[*it]);
		}
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::dmaps(bool numbered)
{
	GUI::ListData ls;
	
	for(int32_t q = 0; q < MAXDMAPS; ++q)
	{
		char const* dm_str = DMaps[q].name;
		if(numbered)
			ls.add(fmt::format("{:3}-{}",q,dm_str), q);
		else ls.add(dm_str, q);
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::counters(bool numbered, bool skipNone)
{
	GUI::ListData ls;
	// std::map<std::string, int32_t> vals;
	// std::set<std::string> names;
	
	if(!skipNone) ls.add("(None)", crNONE);
	for(int32_t q = 0; q < MAX_COUNTERS; ++q)
	{
		if(!ZI.isUsableCtr(q))
			continue; //Hidden
		char const* module_str = ZI.getCtrName(q);
		std::string sname;
		if(numbered)
			sname = fmt::format("{} ({:02})", module_str, q);
		else sname = module_str;
		
		
		// vals[sname] = q;
		// names.insert(sname);
		ls.add(sname, q);
	}
	
	// for(auto it = names.begin(); it != names.end(); ++it)
	// {
		// ls.add(*it, vals[*it]);
	// }
	
	return ls;
}

GUI::ListData GUI::ZCListData::miscsprites(bool skipNone, bool inclNegSpecialVals, bool numbered)
{
	std::map<std::string, int32_t> ids;
	std::set<std::string> sprnames;
	
	for(int32_t i=0; i<MAXWPNS; ++i)
	{
		std::string name;
		if(numbered)
			name = fmt::format("{} ({:03})", weapon_string[i], i);
		else name = weapon_string[i];
		
		ids[name] = i;
		sprnames.insert(name);
	}
	
	GUI::ListData ls;
	if(inclNegSpecialVals)
	{
		if(numbered)
		{
			ls.add("Grass Clippings (-004)", -4);
			ls.add("Flower Clippings (-003)", -3);
			ls.add("Bush Leaves (-002)", -2);
		}
		else
		{
			ls.add("Grass Clippings", -4);
			ls.add("Flower Clippings", -3);
			ls.add("Bush Leaves", -2);
		}
	}
	if(!skipNone)
		ls.add("(None)", -1);
	for(auto it = sprnames.begin(); it != sprnames.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

GUI::ListData GUI::ZCListData::bottletype()
{
	GUI::ListData ls;
	
	ls.add("(None)", 0);
	for(int32_t q = 0; q < 64; ++q)
	{
		if(QMisc.bottle_types[q].name[0])
			ls.add(QMisc.bottle_types[q].name,q+1);
		else
			ls.add(fmt::format("{:2}", q+1), q+1);
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::lweaptypes()
{
	std::map<std::string, int32_t> vals;
	
	std::string none(moduledata.player_weapon_names[0]);
	if(skipchar(moduledata.player_weapon_names[0][0]))
		none = "(None)";
	
	GUI::ListData ls;
	ls.add(none, 0);
	for(int32_t i=1; i<41; ++i)
	{
		if(skipchar(moduledata.player_weapon_names[i][0]))
			continue;
		
		std::string sname(moduledata.player_weapon_names[i]);
		ls.add(sname, i);
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::weaptypes(bool numbered)
{
	std::map<std::string, int32_t> vals;
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	for(int32_t q=1; q<wMax; ++q)
	{
		if(!ZI.isUsableWeap(q))
			continue; //Hidden
		char const* module_str = ZI.getWeapName(q);
		if(numbered)
			ls.add(fmt::format("{} ({:03})", module_str, q), q);
		else ls.add(module_str, q);
	}
	
	return ls;
}

GUI::ListData GUI::ZCListData::sfxnames(bool numbered)
{
	std::map<std::string, int32_t> vals;
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	for(int32_t i=1; i<WAV_COUNT; ++i)
	{
		char const* sfx_name = sfx_string[i];
		if(numbered)
			ls.add(fmt::format("{} ({:03})", sfx_name, i), i);
		else ls.add(sfx_name, q);
	}
	
	return ls;
}
GUI::ListData GUI::ZCListData::midinames(bool numbered)
{
	std::map<std::string, int32_t> vals;
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	for(int32_t i=0; i<MAXCUSTOMTUNES; ++i)
	{
		char const* midi_name = customtunes[i].title;
		std::string name;
		if(numbered)
			ls.add(fmt::format("{} ({:03})", midi_name, i+1), i+1);
		else ls.add(midi_name, i+1);
	}
	
	return ls;
}

static void load_scriptnames(std::set<std::string> &names, std::map<std::string, int32_t> &vals,
	std::map<int32_t, script_slot_data> scrmap, int32_t count)
{
	for(int32_t i = 0; i < count; ++i)
	{
		if(!scrmap[i].scriptname[0])
			continue;
		std::string sname(scrmap[i].scriptname);
		sname += " (" + std::to_string(i+1) + ")";
		
		vals[sname] = i+1;
		names.insert(sname);
	}
}

GUI::ListData GUI::ZCListData::itemdata_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,itemmap,NUMSCRIPTITEM-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::itemsprite_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,itemspritemap,NUMSCRIPTSITEMSPRITE-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::ffc_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,ffcmap,NUMSCRIPTFFC-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::screen_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,screenmap,NUMSCRIPTSCREEN-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::lweapon_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,lwpnmap,NUMSCRIPTWEAPONS-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::eweapon_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,ewpnmap,NUMSCRIPTWEAPONS-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::combodata_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,comboscriptmap,NUMSCRIPTSCOMBODATA-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

GUI::ListData GUI::ZCListData::generic_script()
{
	std::map<std::string, int32_t> vals;
	std::set<std::string> names;
	
	load_scriptnames(names,vals,genericmap,NUMSCRIPTSGENERIC-1);
	
	GUI::ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

static const GUI::ListData defense_types
{
	{ "(None)", 0 },
	{ "1/2 Damage", 1 },
	{ "1/4 Damage", 2 },
	{ "Stun", 3 },
	{ "Stun Or Block", 4 },
	{ "Stun Or Ignore", 5 },
	{ "Block If < 1", 6 },
	{ "Block If < 2", 7 },
	{ "Block If < 4", 8 },
	{ "Block If < 6", 9 },
	{ "Block If < 8", 10 },
	{ "Block", 11 },
	{ "Ignore If < 1", 12 },
	{ "Ignore", 13 },
	{ "One-Hit-Kill", 14 },
	{ "Block if Power < 10", 15 },
	{ "Double Damage", 16 },
	{ "Triple Damage", 17 },
	{ "Quadruple Damage", 18 },
	{ "Enemy Gains HP = Damage", 19 },
	{ "Trigger Screen Secrets", 20 },
	// { "-freeze", 21 },
	// { "-msgnotenabled", 22 },
	// { "-msgline", 23 }, 
	// { "-lvldamage", 24 },
	// { "-lvlreduction", 25 },
	{ "Split", 26 },
	{ "Transform", 27 },
	// { "-lvlblock2", 28 },
	// { "-lvlblock3", 29 },
	// { "-lvlblock4", 30 }, 
	// { "-lvlblock5", 31 },
	// { "-shock", 32 },
	{ "Bomb Explosion", 33 },
	{ "Superbomb Explosion", 34 },
	// { "-deadrock", 35 },
	// { "-breakshoeld", 36 },
	// { "-restoreshield", 37 },
	// { "-specialdrop", 38 },
	// { "-incrcounter", 39 },
	// { "-reducecounter", 40 },
	{ "Harmless Explosion", 41 },
	// { "-killnosplit", 42 },
	// { "-tribble", 43 },
	// { "-fireball", 44 },
	// { "-fireballlarge", 45 },
	{ "Summon", 46 }// ,
	// { "-wingame", 47 },
	// { "-jump", 48 },
	// { "-eat", 49 },
	// { "-showmessage", 50 }
};

GUI::ListData const& GUI::ZCListData::deftypes()
{
	return defense_types;
}

static const GUI::ListData warp_effects
{
	{ "None", 0 },
	{ "Zap", 1 },
	{ "Wave", 2 },
	{ "Instant", 3 },
	{ "Open", 4 }
};

GUI::ListData const& GUI::ZCListData::warpeffects()
{
	return warp_effects;
}

static const GUI::ListData screen_state
{
	{ "Door Up", 0 },
	{ "Door Down", 1 },
	{ "Door Left", 2 },
	{ "Door Right", 3 },
	{ "Screen Item", 4 },
	{ "Special Item", 5 },
	{ "Enemies Never Return", 6 },
	{ "Enemies Temp No Return", 7 },
	{ "Lockblock", 8 },
	{ "Boss Lockblock", 9 },
	{ "Chest", 10 },
	{ "Locked Chest", 11 },
	{ "Boss Chest", 12 },
	{ "Secrets", 13 },
	{ "Visited", 14 },
	{ "Light Triggers", 15 }
};

GUI::ListData const& GUI::ZCListData::screenstate()
{
	return screen_state;
}



