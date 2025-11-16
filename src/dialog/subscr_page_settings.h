#ifndef ZC_DIALOG_SUBSCR_PAGE_SETTINGS_H_
#define ZC_DIALOG_SUBSCR_PAGE_SETTINGS_H_

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/text_field.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/grid.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

void call_subscr_pagesettings_dialog();

class SubscrPageSettings: public GUI::Dialog<SubscrPageSettings>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, REFR_SELECTOR
	};

	SubscrPageSettings();
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Button> pgbtns[8];
	std::shared_ptr<GUI::TextField> pgtfs[8];
	
	GUI::ListData list_pg_mode;
	
	SubscrPage local_pageref;
	
	void refr_info();
};

#endif
