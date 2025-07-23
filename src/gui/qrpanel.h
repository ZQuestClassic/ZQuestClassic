#ifndef ZC_GUI_QRPANEL_H_
#define ZC_GUI_QRPANEL_H_

#include "gui/widget.h"
#include "gui/tabpanel.h"
#include "gui/tabref.h"
#include "gui/button.h"
#include "list_data.h"
#include "gui/scrolling_pane.h"
#include <vector>
#include "gui/dialog_ref.h"

namespace GUI
{

class QRPanel: public TabPanel
{
public:
	QRPanel();
	
	void loadQRs(byte* qrs);
	void setDestQRs(byte* qrs);
	void setCount(size_t count);
	void setShowTags(bool v);
	void setIndexed(bool v);
	void setScrollWidth(Size sz);
	void setScrollHeight(Size sz);
	void loadList(GUI::ListData qrlist);

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int32_t>(m);
	}
	template<typename T>
	RequireMessage<T> onCloseInfo(T m)
	{
		info_message = static_cast<int32_t>(m);
	}
	
	ScrollingPane* getScrollPane();
	
private:
	std::shared_ptr<ScrollingPane> scrollpane;
	bool scrolling, showTags, indexed;
	Size scrollWidth, scrollHeight;
	
	int32_t message, info_message;
	byte* local_qrs;
	size_t qrCount;
};

}

#endif
