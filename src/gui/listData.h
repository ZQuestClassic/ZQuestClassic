#ifndef ZC_GUI_LISTDATA_H
#define ZC_GUI_LISTDATA_H

#include <functional>
#include <string>
#include <initializer_list>

namespace gui
{

// Type and functions for the most common cases (?)

struct BasicData
{
	const char* text;
	int value;
};

// Just returns the text as a string.
// { "Hookshot", 15 } => "Hookshot"
std::string itemText(size_t, const BasicData& data);

// Formats as "index: text".
// { "Hookshot", 15 } => "15: Hookshot"
std::string indexItemText(size_t index, const BasicData& data);

// Formats as "(index+1): text".
// { "Hookshot", 15 } => "16: Hookshot"
std::string indexPlusOneItemText(size_t index, const BasicData& data);

// Data source for List and DropDownList.
// Remember to specify gui::ListData to avoid confusion with the ListData
// defined in jwin.h.
// Might be worth renaming this one, actually...
// TODO: How hard would it be to make the value type a template parameter?
// Or maybe a std::any?
class ListData
{
public:
	ListData(ListData&& other)=default;

	ListData(size_t numItems, std::function<std::string(size_t)> getString,
		std::function<int(size_t)> getValue);

	ListData(std::function<std::string(size_t, const BasicData&)> format,
		const std::initializer_list<BasicData>& listSource);

	size_t size() const;
	const std::string& listEntry(size_t index) const;
	const int value(size_t index) const;

private:
	std::vector<std::string> listEntries;
	std::vector<int> values;
};

}

#endif
