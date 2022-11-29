#ifndef ZC_GUI_DDPANEL_H
#define ZC_GUI_DDPANEL_H

#include "widget.h"
#include "tabpanel.h"
#include "tabref.h"
#include "button.h"
#include "list_data.h"
#include <vector>
#include "dialog_ref.h"

namespace GUI
{

class DDPanel: public TabPanel
{
public:
	DDPanel();
	void linkVals(int32_t* vals);
	void setCount(size_t count);
	void loadDDList(GUI::ListData const& theData);
	void loadList(GUI::ListData qrlist);

	template<typename T>
	RequireMessage<T> onSelectionChanged(T m)
	{
		message = static_cast<int32_t>(m);
	}
private:
	int32_t message;
	int32_t* theVals;
	GUI::ListData const* dataList;
	size_t ddCount;
};

}

#endif
