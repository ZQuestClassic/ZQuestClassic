#include "list_data.h"
#include <map>

using std::map, std::string, std::set, std::function, std::move;
namespace GUI
{

ListData::ListData(size_t numItems,
	function<string(size_t)> getString,
	function<int32_t(size_t)> getValue) : _invalid(false)
{
	listItems.reserve(numItems);
	for(size_t index = 0; index < numItems; ++index)
		listItems.emplace_back(move(getString(index)), getValue(index));
}

ListData::ListData(::ListData const& jwinldata, int32_t valoffs) : _invalid(false)
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
ListData ListData::numbers(bool none, int32_t start, uint32_t count, std::function<std::string(int)> formatter)
{
	ListData ls;
	if(none)
	{
		ls.add("(None)", start>0 ? 0 : start-1);
	}
	for(uint32_t i=0; i<count; ++i)
	{
		ls.add(formatter(start+i), start+i);
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

ListData ListData::operator+(ListData const& other) const
{
	ListData ld = *this;
	return ld += other;
}
ListData& ListData::operator+=(ListData const& other)
{
	listItems.insert(listItems.end(), other.listItems.begin(), other.listItems.end());
	return *this;
}

void ListData::for_all_items(std::function<void(ListItem const&)> proc)
{
	for (ListItem const& itm : listItems)
		proc(itm);
}

ListData& ListData::filter(std::function<bool(ListItem&)> filt_func)
{
	for(auto it = listItems.begin(); it != listItems.end();)
	{
		ListItem& itm = *it;
		
		if(filt_func(itm)) ++it;
		else it = listItems.erase(it);
	}
	return *this;
}

ListData& ListData::valsort(size_t frozen)
{
	vector<ListItem> frozen_items;
	size_t frozen_ind = 0;
	map<int32_t,std::vector<ListItem>> list;
	for(ListItem& li : listItems)
	{
		if(frozen_ind < frozen)
		{
			frozen_items.push_back(li);
			++frozen_ind;
		}
		else list[li.value].push_back(li);
	}
	listItems.clear();
	for(auto& li : frozen_items)
		listItems.push_back(li);
	for(auto& p : list)
	{
		auto& vec = p.second;
		for(auto& li : vec)
			listItems.push_back(li);
	}
	return *this;
}
ListData& ListData::tagsort(size_t frozen)
{
	vector<ListItem> frozen_items;
	size_t frozen_ind = 0;
	map<int32_t,std::vector<ListItem>> list;
	for(ListItem& li : listItems)
	{
		if(frozen_ind < frozen)
		{
			frozen_items.push_back(li);
			++frozen_ind;
		}
		else list[li.tag].push_back(li);
	}
	listItems.clear();
	for(auto& li : frozen_items)
		listItems.push_back(li);
	for(auto& p : list)
	{
		auto& vec = p.second;
		for(auto& li : vec)
			listItems.push_back(li);
	}
	return *this;
}
ListData& ListData::alphabetize(size_t frozen)
{
	vector<ListItem> frozen_items;
	size_t frozen_ind = 0;
	map<string,std::vector<ListItem>> list;
	for(ListItem& li : listItems)
	{
		if(frozen_ind < frozen)
		{
			frozen_items.push_back(li);
			++frozen_ind;
		}
		else list[li.text].push_back(li);
	}
	listItems.clear();
	for(auto& li : frozen_items)
		listItems.push_back(li);
	for(auto& p : list)
	{
		auto& vec = p.second;
		for(auto& li : vec)
			listItems.push_back(li);
	}
	return *this;
}

ListData& ListData::tag(int32_t tagval, bool onlyUntagged)
{
	for(ListItem& li : listItems)
	{
		if(onlyUntagged && li.tag != -1)
			continue;
		li.tag = tagval;
	}
	return *this;
}

ListItem& ListData::accessItem(int32_t key)
{
	for(ListItem& li : listItems)
	{
		if(li.value == key)
			return li;
	}
	static ListItem nil("",0);
	return nil;
}
ListItem& ListData::accessIndex(size_t index)
{
	return listItems.at(index);
}
}
