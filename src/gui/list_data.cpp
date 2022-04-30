#include "list_data.h"
#include <boost/format.hpp>
#include <map>
#include "../qst.h"

#ifndef IS_LAUNCHER
extern zcmodule moduledata;
extern char *weapon_string[];
extern char *sfx_string[];

#ifdef IS_ZQUEST
extern const char *msgfont_str[font_max];
extern const char *shadowstyle_str[sstsMAX];
extern int32_t numericalFlags;
extern miscQdata misc;
#define QMisc misc
#else
extern miscQdata QMisc;
#endif

#endif

using std::map, std::string, std::set, std::function, std::move, std::to_string;
namespace GUI
{

static bool skipchar(char c)
{
	return c == 0 || c == '-';
}

ListData::ListData(size_t numItems,
	function<string(size_t)> getString,
	function<int32_t(size_t)> getValue)
{
	listItems.reserve(numItems);
	for(size_t index = 0; index < numItems; ++index)
		listItems.emplace_back(move(getString(index)), getValue(index));
}

ListData::ListData(::ListData const& jwinldata, int32_t valoffs)
{
	int32_t sz;
	jwinldata.listFunc(-1, &sz);
	listItems.reserve(size_t(sz));
	if (sz < 1) return;
	for(size_t index = 0; index < size_t(sz); ++index)
	{
		string str(jwinldata.listFunc(index, NULL));
		listItems.emplace_back(move(str), int32_t(index)+valoffs);
	}
}

const char* ListData::jwinWrapper(int32_t index, int32_t* size, void* owner)
{
	ListData* cb=static_cast<ListData*>(owner);
	
	if(index >= 0)
		return cb->getText(index).c_str();
	else
	{
		*size = cb->size();
		return nullptr;
	}
}

ListData ListData::numbers(bool none, int32_t start, uint32_t count)
{
	ListData ls;
	if(none)
	{
		ls.add("(None)", start>0 ? 0 : start-1);
	}
	for(uint32_t i=0; i<count; ++i)
	{
		ls.add(std::to_string(start+i), start+i);
	}
	
	return ls;
}

#ifndef IS_LAUNCHER
ListData ListData::itemclass(bool numbered)
{
	map<string, int32_t> fams;
	set<string> famnames;
	
	for(int32_t i=0; i<itype_max; ++i)
	{
		if(!ZI.isUsableItemclass(i))
			continue; //Hidden
		char const* itname = ZI.getItemClassName(i);
        if(i < itype_last || itname[0])
		{
            char* name = new char[strlen(itname) + 7];
            if(numbered)
				sprintf(name, "%s (%03d)", itname, i);
            else strcpy(name, itname);
			string sname(name);
			
			fams[sname] = i;
			famnames.insert(sname);
			delete[] name;
		}
		else 
		{
			char *name = new char[12];
			if(numbered)
				sprintf(name, "zz%03d (%03d)", i, i);
			else sprintf(name, "zz%03d", i);
			string sname(name);
			
			fams[sname] = i;
			famnames.insert(sname);
			delete[] name;
		}
	}
	
	ListData ls;
	
	for(auto it = famnames.begin(); it != famnames.end(); ++it)
	{
		ls.add(*it, fams[*it]);
	}
	return ls;
}

ListData ListData::combotype(bool numbered, bool skipNone)
{
	ListData ls;
	map<string, int32_t> types;
	set<string> typenames;

	if(!skipNone) ls.add("(None)", 0);
	for(int32_t i=1; i<cMAX; ++i)
	{
		if(!ZI.isUsableComboType(i))
			continue; //Hidden
		char const* module_str = ZI.getComboTypeName(i);
		if(module_str[0])
		{
			char* name = new char[strlen(module_str) + 7];
			if(numbered)
				sprintf(name, "%s (%03d)", module_str, i);
			else strcpy(name, module_str);
			string sname(name);
			
			types[sname] = i;
			typenames.insert(sname);
			delete[] name;
		}
		else 
		{
			char *name = new char[12];
			if(numbered)
				sprintf(name, "zz%03d (%03d)", i, i);
			else sprintf(name, "zz%03d", i);
			string sname(name);
			
			types[sname] = i;
			typenames.insert(sname);
			delete[] name;
		}
	}

	for(auto it = typenames.begin(); it != typenames.end(); ++it)
	{
		ls.add(*it, types[*it]);
	}
	return ls;
}
#ifdef IS_ZQUEST
ListData ListData::mapflag(bool numbered, bool skipNone)
{
	ListData ls;
	map<string, int32_t> vals;
	set<string> names;
	
	if(!skipNone) ls.add("(None)", 0);
	for(int32_t q = 1; q < mfMAX; ++q)
	{
		if(!ZI.isUsableMapFlag(q))
			continue; //Hidden
		char const* module_str = ZI.getMapFlagName(q);
		char* name = new char[strlen(module_str) + 7];
		if(numbered)
			sprintf(name, "%s (%03d)", module_str, q);
		else strcpy(name, module_str);
		
		string sname(name);
		if (numericalFlags)
		{
			ls.add(sname, q);
		}
		else
		{
			vals[sname] = q;
			names.insert(sname);
		}
		
		delete[] name;
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
#endif

ListData ListData::dmaps(bool numbered)
{
	ListData ls;
	
	for(int32_t q = 0; q < MAXDMAPS; ++q)
	{
		char const* dm_str = DMaps[q].name;
		if(numbered)
		{
			char* name = new char[strlen(dm_str) + 7];
			sprintf(name, "%3d-%s", q, dm_str);
			ls.add(name, q);
			delete[] name;
		}
		else ls.add(dm_str, q);
	}
	
	return ls;
}

ListData ListData::counters(bool numbered, bool skipNone)
{
	ListData ls;
	// map<string, int32_t> vals;
	// set<string> names;
	
	if(!skipNone) ls.add("(None)", crNONE);
	for(int32_t q = 0; q < MAX_COUNTERS; ++q)
	{
		if(!ZI.isUsableCtr(q))
			continue; //Hidden
		char const* module_str = ZI.getCtrName(q);
		char* name = new char[strlen(module_str) + 6];
		if(numbered)
			sprintf(name, "%s (%02d)", module_str, q);
		else strcpy(name, module_str);
		
		string sname(name);
		
		// vals[sname] = q;
		// names.insert(sname);
		ls.add(sname, q);
		
		delete[] name;
	}
	
	// for(auto it = names.begin(); it != names.end(); ++it)
	// {
		// ls.add(*it, vals[*it]);
	// }
	
	return ls;
}

ListData ListData::miscsprites()
{
	map<string, int32_t> ids;
	set<string> sprnames;
	
	for(int32_t i=0; i<wMAX; ++i)
	{
		string sname(weapon_string[i]);
		
		ids[sname] = i;
		sprnames.insert(sname);
	}
	
	ListData ls;
	
	for(set<string>::iterator it = sprnames.begin(); it != sprnames.end(); ++it)
	{
		ls.add(*it, ids[*it]);
	}
	return ls;
}

ListData ListData::bottletype()
{
	ListData ls;
	
	ls.add("(None)", 0);
	for(int32_t q = 0; q < 64; ++q)
	{
		if(QMisc.bottle_types[q].name[0])
			ls.add(QMisc.bottle_types[q].name,q+1);
		else
		{
			char buf[8] = { 0 };
			sprintf(buf, "%2d", q+1);
			
			ls.add(buf, q+1);
		}
	}
	
	return ls;
}

ListData ListData::lweaptypes()
{
	map<string, int32_t> vals;
	
	string none(moduledata.player_weapon_names[0]);
	if(skipchar(moduledata.player_weapon_names[0][0]))
		none = "(None)";
	
	ListData ls;
	ls.add(none, 0);
	for(int32_t i=1; i<41; ++i)
	{
		if(skipchar(moduledata.player_weapon_names[i][0]))
			continue;
		
		string sname(moduledata.player_weapon_names[i]);
		ls.add(sname, i);
	}
	
	return ls;
}

ListData ListData::sfxnames(bool numbered)
{
	map<string, int32_t> vals;
	
	ListData ls;
	ls.add("(None)", 0);
	for(int32_t i=1; i<WAV_COUNT; ++i)
	{
		char const* sfx_name = sfx_string[i];
		char* name = new char[strlen(sfx_name) + 7];
		if(numbered)
			sprintf(name, "%s (%03d)", sfx_name, i);
		else strcpy(name, sfx_name);
		ls.add(name, i);
		delete[] name;
	}
	
	return ls;
}

static void load_scriptnames(set<string> &names, map<string, int32_t> &vals,
	map<int32_t, script_slot_data> scrmap, int32_t count)
{
	for(int32_t i = 0; i < count; ++i)
	{
		if(!scrmap[i].scriptname[0])
			continue;
		string sname(scrmap[i].scriptname);
		sname += " (" + to_string(i+1) + ")";
		
		vals[sname] = i+1;
		names.insert(sname);
	}
}

ListData ListData::itemdata_script()
{
	map<string, int32_t> vals;
	set<string> names;
	
	load_scriptnames(names,vals,itemmap,NUMSCRIPTITEM-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::itemsprite_script()
{
	map<string, int32_t> vals;
	set<string> names;
	
	load_scriptnames(names,vals,itemspritemap,NUMSCRIPTSITEMSPRITE-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::ffc_script()
{
	map<string, int32_t> vals;
	set<string> names;
	
	load_scriptnames(names,vals,ffcmap,NUMSCRIPTFFC-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::lweapon_script()
{
	map<string, int32_t> vals;
	set<string> names;
	
	load_scriptnames(names,vals,lwpnmap,NUMSCRIPTWEAPONS-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::combodata_script()
{
	map<string, int32_t> vals;
	set<string> names;
	
	load_scriptnames(names,vals,comboscriptmap,NUMSCRIPTSCOMBODATA-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}
#endif
#if IS_ZQUEST
ListData ListData::fonts()
{
	ListData ls;
	for(auto q = 0; q < font_max; ++q)
	{
		ls.add(msgfont_str[q], q);
	}
	return ls;
}
ListData ListData::shadowtypes()
{
	ListData ls;
	
	for(int32_t q = 0; q < sstsMAX; ++q)
	{
		ls.add(shadowstyle_str[q], q);
	}
	
	return ls;
}
#endif

void ListData::add(set<string> names, map<string, int32_t> vals)
{
	for(set<string>::iterator it = names.begin(); it != names.end(); ++it)
	{
		add(*it, vals[*it]);
	}
}

const ListData defense_types
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

ListData const& ListData::deftypes()
{
	return defense_types;
}

}
