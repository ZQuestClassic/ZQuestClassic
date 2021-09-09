#include "listData.h"
#include <boost/format.hpp>

namespace gui
{

ListData::ListData(size_t numItems,
	std::function<std::string(size_t)> getString,
	std::function<int(size_t)> getValue)
{
	for(auto index=0; index<numItems; index++)
	{
		listEntries.emplace_back(getString(index));
		values.push_back(getValue(index));
	}
}

ListData::ListData(std::function<std::string(size_t, const BasicData&)> format,
	const std::initializer_list<BasicData>& listSource)
{
	// initializer_list doesn't have operator[] or at() or anything
	size_t index=0;
	for(const auto& li: listSource)
	{
		listEntries.emplace_back(format(index, li));
		values.push_back(li.value);
		index++;
	}
}

size_t ListData::size() const
{
	return listEntries.size();
}

const std::string& ListData::listEntry(size_t index) const
{
	return listEntries.at(index);
}

const int ListData::value(size_t index) const
{
	return values.at(index);
}

std::string itemText(size_t, const BasicData& data)
{
	return boost::str(boost::format("%1%") % data.text);
}

std::string indexItemText(size_t index, const BasicData& data)
{
	return boost::str(boost::format("%1%: %2%") % index % data.text);
}

std::string indexPlusOneItemText(size_t index, const BasicData& data)
{
	return boost::str(boost::format("%1%: %2%") % (index+1) % data.text);
}

}
