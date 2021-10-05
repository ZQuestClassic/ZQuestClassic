#include "list_data.h"
#include <boost/format.hpp>

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

}
