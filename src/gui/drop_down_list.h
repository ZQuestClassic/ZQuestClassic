#ifndef ZC_GUI_DROPDOWNLIST_H
#define ZC_GUI_DROPDOWNLIST_H

#include "widget.h"
#include "dialog_ref.h"
#include "list_data.h"
#include "../jwin.h"
#include <vector>

namespace GUI
{

class DialogRunner;

class DropDownList: public Widget
{
public:
	DropDownList();

	/* Sets the source of items to appear in the list. */
	void setListData(const ::GUI::ListData& newListData);

	/* Sets the value of the list item to select. If there is no item with
	 * a matching value, the closest possible will be selected. If there are
	 * multiple equally good matches, it is unspecified which will be selected.
	 */
	void setSelectedValue(int32_t value);

	/* Selects the list item with the given index. */
	void setSelectedIndex(int32_t index);

	/* Returns the value of the currently selected item. */
	int32_t getSelectedValue() const;

	void setOnSelectFunc(std::function<void(int32_t)> newOnSelect)
	{
		onSelectFunc = newOnSelect;
	}
	
	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}

private:
	// A bit ugly because there was already a ListData struct in jwin
	::ListData jwinListData;
	const ::GUI::ListData* listData;
	int32_t selectedIndex, selectedValue;
	DialogRef alDialog;
	int32_t message;
	std::function<void(int32_t)> onSelectFunc;

	/* If a value was set rather than an index, find an index to select. */
	void setIndex();
	void applyVisibility(bool visible) override;
	void applyDisabled(bool dis) override;
	void realize(DialogRunner& runner) override;
	void calculateSize() override;
	int32_t onEvent(int32_t event, MessageDispatcher& sendMessage) override;
	void applyFont(FONT* newFont) override;
};

}

#endif
