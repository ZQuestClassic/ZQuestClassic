#ifndef ZC_GUI_LIST_H_
#define ZC_GUI_LIST_H_

#include "gui/widget.h"
#include "gui/dialog_ref.h"
#include "list_data.h"
#include "gui/jwin.h"
#include <vector>

namespace GUI
{

class DialogRunner;

/* This and DropDownList are nearly identical. They should have a common
 * base class, or perhaps even be the same class. This is just a quick
 * and dirty copy-paste job.
 */
class List: public Widget
{
public:
	List();

	/* Sets the source of items to appear in the list. */
	void setListData(const ::GUI::ListData& newListData);

	/* Sets the value of the list item to select. If there is no item with
	 * a matching value, the closest possible will be selected. If there are
	 * multiple equally good matches, it is unspecified which will be selected.
	 */
	void setSelectedValue(int32_t value);

	/* Selects the list item with the given index. */
	void setSelectedIndex(int32_t index, bool offset = false);

	/* Returns the value of the currently selected item. */
	int32_t getSelectedValue() const;

	int32_t getSelectedIndex() const;

	void setOnSelectFunc(std::function<void(int32_t)> newFunc)
	{
		onSelectFunc = newFunc;
	}
	void setOnRClickFunc(std::function<void(int32_t,int32_t,int32_t)> newFunc)
	{
		onRClickFunc = newFunc;
	}
	void setOnDClickFunc(std::function<void(int32_t,int32_t,int32_t)> newFunc)
	{
		onDClickFunc = newFunc;
	}
	
	void setIsABC(bool abc);

	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
	template<typename T>
	RequireMessage<T> onRClick(T m)
	{
		msg_r = static_cast<int32_t>(m);
	}
	template<typename T>
	RequireMessage<T> onDClick(T m)
	{
		msg_d = static_cast<int32_t>(m);
	}
	
	/* If a value was set rather than an index, find an index to select. */
	void setIndex();
private:
	// A bit ugly because there was already a ListData struct in jwin
	::ListData jwinListData;
	const ::GUI::ListData* listData;
	int32_t selectedIndex, selectedValue;
	DialogRef alDialog;
	int32_t message, msg_r, msg_d;
	std::function<void(int32_t)> onSelectFunc;
	std::function<void(int32_t,int32_t,int32_t)> onRClickFunc;
	std::function<void(int32_t,int32_t,int32_t)> onDClickFunc;
	bool isABC;
	
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
};

}

#endif
