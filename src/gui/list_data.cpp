#include "list_data.h"
#include <boost/format.hpp>
#include <map>
extern zcmodule moduledata;

namespace GUI
{

ListData::ListData(size_t numItems,
	std::function<std::string(size_t)> getString,
	std::function<int(size_t)> getValue)
{
	listItems.reserve(numItems);
	for(size_t index = 0; index < numItems; ++index)
		listItems.emplace_back(std::move(getString(index)), getValue(index));
}

ListData::ListData(::ListData const& jwinldata, int valoffs)
{
	int sz;
	jwinldata.listFunc(-1, &sz);
	listItems.reserve(size_t(sz));
	if (sz < 1) return;
	for(size_t index = 0; index < size_t(sz); ++index)
	{
		std::string str(jwinldata.listFunc(index, NULL));
		listItems.emplace_back(std::move(str), int(index)+valoffs);
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
	std::map<std::string, int> fams;
	std::set<std::string> famnames;
	
	for(int i=0; i<itype_max; ++i)
	{
        if(i < itype_last || moduledata.item_editor_type_names[i][0] != NULL )
		{
            char const* module_str = moduledata.item_editor_type_names[i];
            char* name = new char[strlen(module_str) + 7];
            if(numbered)
				sprintf(name, "%s (%03d)", module_str, i);
            else strcpy(name, module_str);
			std::string sname(name);
			
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
			std::string sname(name);
			
			fams[sname] = i;
			famnames.insert(sname);
			delete[] name;
		}
	}
	
	ListData ls;
	
	for(std::set<std::string>::iterator it = famnames.begin(); it != famnames.end(); ++it)
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

}
