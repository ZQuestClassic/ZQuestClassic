#include "list_data.h"
#include <boost/format.hpp>
#include <map>
#include "../qst.h"
extern zcmodule moduledata;
extern char *weapon_string[];
using std::map, std::string, std::set, std::function, std::move, std::to_string;
namespace GUI
{

static bool skipchar(char c)
{
	return c == 0 || c == '-';
}

ListData::ListData(size_t numItems,
	function<string(size_t)> getString,
	function<int(size_t)> getValue)
{
	listItems.reserve(numItems);
	for(size_t index = 0; index < numItems; ++index)
		listItems.emplace_back(move(getString(index)), getValue(index));
}

ListData::ListData(::ListData const& jwinldata, int valoffs)
{
	int sz;
	jwinldata.listFunc(-1, &sz);
	listItems.reserve(size_t(sz));
	if (sz < 1) return;
	for(size_t index = 0; index < size_t(sz); ++index)
	{
		string str(jwinldata.listFunc(index, NULL));
		listItems.emplace_back(move(str), int(index)+valoffs);
	}
}

const char* ListData::jwinWrapper(int index, int* size, void* owner)
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

ListData ListData::itemclass(bool numbered)
{
	map<string, int> fams;
	set<string> famnames;
	
	for(int i=0; i<itype_max; ++i)
	{
        if(i < itype_last || moduledata.item_editor_type_names[i][0] != NULL )
		{
            char const* module_str = moduledata.item_editor_type_names[i];
            char* name = new char[strlen(module_str) + 7];
            if(numbered)
				sprintf(name, "%s (%03d)", module_str, i);
            else strcpy(name, module_str);
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
	
	for(set<string>::iterator it = famnames.begin(); it != famnames.end(); ++it)
	{
		ls.add(*it, fams[*it]);
	}
	return ls;
}

ListData ListData::counters()
{
	ListData ls;
	
	for(int q = -1; q < MAX_COUNTERS; ++q)
	{
		ls.add(moduledata.counter_names[q+1], q);
	}
	
	return ls;
}

ListData ListData::miscsprites()
{
	map<string, int> ids;
	set<string> sprnames;
	
	for(int i=0; i<wMAX; ++i)
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

ListData ListData::lweaptypes()
{
	map<string, int> vals;
	set<string> sprnames;
	
	string none(moduledata.player_weapon_names[0]);
	if(skipchar(moduledata.player_weapon_names[0][0]))
		none = "(None)";
	
	ListData ls;
	ls.add(none, 0);
	for(int i=1; i<41; ++i)
	{
		if(skipchar(moduledata.player_weapon_names[i][0]))
			continue;
		
		string sname(moduledata.player_weapon_names[i]);
		ls.add(sname, i);
	}
	
	return ls;
}



static void load_scriptnames(set<string> &names, map<string, int> &vals,
	map<int, script_slot_data> scrmap, int count)
{
	for(int i = 0; i < count; ++i)
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
	map<string, int> vals;
	set<string> names;
	
	load_scriptnames(names,vals,itemmap,NUMSCRIPTITEM-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::itemsprite_script()
{
	map<string, int> vals;
	set<string> names;
	
	load_scriptnames(names,vals,itemspritemap,NUMSCRIPTSITEMSPRITE-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

ListData ListData::lweapon_script()
{
	map<string, int> vals;
	set<string> names;
	
	load_scriptnames(names,vals,lwpnmap,NUMSCRIPTWEAPONS-1);
	
	ListData ls;
	ls.add("(None)", 0);
	ls.add(names,vals);
	return ls;
}

void ListData::add(set<string> names, map<string, int> vals)
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
