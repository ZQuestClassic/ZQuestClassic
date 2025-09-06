#ifndef ZC_GUI_LISTDATA_H_
#define ZC_GUI_LISTDATA_H_

#include "gui/jwin.h"
#include <functional>
#include <initializer_list>
#include <string>
#include <map>
#include <utility>
#include <set>

namespace GUI
{

struct ListItem
{
	ListItem(std::string text, int32_t value, int32_t tag = -1) noexcept:
		text(std::move(text)), info(""), value(value), tag(tag)
	{}
	
	ListItem(std::string text, int32_t value, std::string info, int32_t tag = -1) noexcept:
		text(std::move(text)), info(std::move(info)), value(value), tag(tag)
	{}
	
	ListItem() : value(-1), tag(-1){}

	ListItem& operator=(const ListItem& other) = default;
	ListItem& operator=(ListItem&& other) noexcept = default;

	ListItem(const ListItem& other)=default;
	ListItem(ListItem&& other) noexcept=default;

	std::string text;
	std::string info;
	int32_t value;
	int32_t tag;
};

// Data source for List and DropDownList.
// Remember to specify GUI::ListData to avoid confusion with the ListData
// defined in jwin.h.
// XXX This will probably need to be reworked for lists that change,
// like in the string editor.
class ListData
{
public:
	ListData() : _invalid(false) {}
	ListData(const ListData& other) = default;
	ListData(ListData&& other) = default;
	ListData(std::initializer_list<ListItem> listItems) : listItems(listItems), _invalid(false)
	{}

	ListData(std::vector<ListItem> listItems) : listItems(std::move(listItems)), _invalid(false)
	{}
	ListData(std::vector<std::string> strings) : _invalid(false)
	{
		for(int i = 0; i < strings.size(); i++)
		{
			add(strings[i], i);
		}
	}
	
	ListData(::ListData const& jwinldata, int32_t valoffs = 0);

	ListData(size_t numItems, std::function<std::string(size_t)> getString,
		std::function<int32_t(size_t)> getValue);

	ListData& operator=(const ListData& other) = default;
	ListData& operator=(ListData&& other) noexcept = default;
	
	ListData operator+(ListData const& other) const;
	ListData& operator+=(ListData const& other);

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

	inline bool empty() const
	{
		return !listItems.size();
	}
	
	inline bool invalid() const
	{
		return _invalid;
	}
	
	inline void clear()
	{
		listItems.clear();
	}
	
	inline void setInvalid(bool b)
	{
		_invalid = b;
	}

	inline const std::string& getText(size_t index) const
	{
		return listItems.at(index).text;
	}

	inline const int32_t getValue(size_t index) const
	{
		return listItems.at(index).value;
	}

	inline const std::string& getInfo(size_t index) const
	{
		return listItems.at(index).info;
	}
	
	inline static const std::string nullstr = "";
	inline const std::string& findText(int32_t key) const
	{
		for(ListItem const& li : listItems)
		{
			if(li.value == key)
				return li.text;
		}
		return nullstr;
	}
	
	inline const std::string& findInfo(int32_t key) const
	{
		for(ListItem const& li : listItems)
		{
			if(li.value == key)
				return li.info;
		}
		return nullstr;
	}
	
	inline const optional<size_t> findIndex(int32_t key) const
	{
		for(size_t ind = 0; ind < listItems.size(); ++ind)
		{
			if(listItems[ind].value == key)
				return ind;
		}
		return nullopt;
	}
	
	inline void removeVal(int32_t key)
	{
		for(std::vector<ListItem>::iterator it = listItems.begin(); it != listItems.end();)
		{
			if((*it).value == key)
				it = listItems.erase(it);
			else ++it;
		}
	}
	
	inline void removeInd(int32_t ind)
	{
		if(unsigned(ind) >= listItems.size())
			return;
		for(std::vector<ListItem>::iterator it = listItems.begin(); it != listItems.end(); ++it)
		{
			if(ind)
				--ind;
			else
			{
				listItems.erase(it);
				break;
			}
		}
	}
	
	inline bool hasKey(int key) const
	{
		return bool(findIndex(key));
	}
	
	//Static constructors for specific lists
	static ListData nullData()
	{
		return ListData();
	}
	static ListData numbers(bool none, int32_t start, uint32_t count);
	static ListData numbers(bool none, int32_t start, uint32_t count, std::function<std::string(int)> formatter);

	void add(ListItem item) {listItems.push_back(item);}
	void add(std::string name, int32_t val) {listItems.emplace_back(name, val);};
	void add(std::string name, int32_t val, std::string desc) {listItems.emplace_back(name, val,desc);};
	void add(std::set<std::string> names, std::map<std::string, int32_t> vals);
	
	ListData copy() const
	{
		ListData other = *this;
		return other;
	}
	
	//Modifies the listdata, and returns a self-reference
	ListData& filter(std::function<bool(ListItem&)>);
	ListData& valsort(size_t frozen = 0);
	ListData& tagsort(size_t frozen = 0);
	ListData& alphabetize(size_t frozen = 0);
	ListData& tag(int32_t tagval, bool onlyUntagged = false);
	
	ListItem& accessItem(int32_t val);
	ListItem& accessIndex(size_t index);
	
private:
	std::vector<ListItem> listItems;
	bool _invalid;
	
	static const char* jwinWrapper(int32_t index, int32_t* size, void* owner);
};

}

#endif
