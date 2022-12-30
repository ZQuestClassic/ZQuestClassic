#ifndef ZC_GUI_QRPANEL_H
#define ZC_GUI_QRPANEL_H

#include "widget.h"
#include "tabpanel.h"
#include "tabref.h"
#include "button.h"
#include "list_data.h"
#include "scrolling_pane.h"
#include <vector>
#include "dialog_ref.h"

namespace GUI
{

class QRPanel: public TabPanel
{
public:
	QRPanel();
	
	void loadQRs(byte const* qrs);
	void setCount(size_t count);
	void setScrollWidth(Size sz);
	void setScrollHeight(Size sz);
	void loadList(GUI::ListData qrlist);

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int32_t>(m);
	}
	
	ScrollingPane* getScrollPane();
	
private:
	std::shared_ptr<ScrollingPane> scrollpane;
	bool scrolling;
	Size scrollWidth, scrollHeight;
	
	int32_t message;
	byte const* init_qrs;
	size_t qrCount;
};

}

#endif
