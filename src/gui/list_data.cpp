#include "list_data.h"
#include <map>

using std::map, std::string, std::set, std::function, std::move;
namespace GUI
{

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

void ListData::add(set<string> names, map<string, int32_t> vals)
{
	for(set<string>::iterator it = names.begin(); it != names.end(); ++it)
	{
		add(*it, vals[*it]);
	}
}

}
