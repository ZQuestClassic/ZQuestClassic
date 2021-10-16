#ifndef ZC_GUI_QRPANEL_H
#define ZC_GUI_QRPANEL_H

#include "widget.h"
#include "tabpanel.h"
#include "tabref.h"
#include "list_data.h"
#include <vector>
#include "dialog_ref.h"

namespace GUI
{

/* A container that displays any one of multiple widgets. Used to dynamically
 * change the layout, e.g. to switch between a TextField and a DropDownList
 * when some selection is changed.
 */
class QRPanel: public TabPanel
{
public:
	QRPanel();
	
	void loadQRs(byte const* qrs);
	void setCount(size_t count);
	void loadList(GUI::ListData qrlist);

	template<typename T>
	RequireMessage<T> onToggle(T m)
	{
		message = static_cast<int32_t>(m);
	}
private:
	int32_t message;
	byte const* init_qrs;
	size_t qrCount;
};

}

#endif
