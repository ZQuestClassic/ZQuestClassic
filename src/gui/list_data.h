#ifndef ZC_GUI_LISTDATA_H
#define ZC_GUI_LISTDATA_H

#include "../jwin.h"
#include <functional>
#include <initializer_list>
#include <string>
#include <utility>

namespace GUI
{

struct ListItem
{
	ListItem(std::string text, int value) noexcept:
		text(std::move(text)), value(value)
	{}

	ListItem& operator=(const ListItem& other)=default;
	ListItem& operator=(ListItem&& other) noexcept=default;

	ListItem(const ListItem& other)=default;
	ListItem(ListItem&& other) noexcept=default;

	std::string text;
	int value;
};

// Data source for List and DropDownList.
// Remember to specify GUI::ListData to avoid confusion with the ListData
// defined in jwin.h.
class ListData
{
public:
	ListData(const ListData& other)=default;
	ListData(ListData&& other)=default;
	ListData(std::initializer_list<ListItem> listItems): listItems(listItems)
	{}

	ListData(std::vector<ListItem> listItems): listItems(std::move(listItems))
	{}

	ListData(size_t numItems, std::function<std::string(size_t)> getString,
		std::function<int(size_t)> getValue);

	ListData& operator=(const ListData& other)=default;
	ListData& operator=(ListData&& other) noexcept=default;

	/* Returns a jwin ListData object for use in DIALOGs. */
	inline ::ListData getJWin(FONT** font) const
	{
		// Not actually const, but it's never modified.
		return ::ListData(jwinWrapper, font, const_cast<ListData*>(this));
	}

	inline size_t size() const
	{
		return listItems.size();
	}

	inline const std::string& getText(size_t index) const
	{
		return listItems.at(index).text;
	}

	inline const int getValue(size_t index) const
	{
		return listItems.at(index).value;
	}

private:
	std::vector<ListItem> listItems;

	static const char* jwinWrapper(int index, int* size, void* owner);
};

}

#endif
