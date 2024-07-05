#ifndef ZC_DIALOG_SUBSCR_SETTINGS_H_
#define ZC_DIALOG_SUBSCR_SETTINGS_H_

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

void call_subscrsettings_dialog();

class SubscrSettingsDialog: public GUI::Dialog<SubscrSettingsDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, REFR_SELECTOR
	};

	SubscrSettingsDialog();
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Widget> SUBSCR_INITD(int index);
	
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Grid> selector_grid;
	
	std::string h_initd[8];
	std::shared_ptr<GUI::TextField> tf_initd[8];
	std::shared_ptr<GUI::Button> ib_initds[8];
	std::shared_ptr<GUI::Label> l_initds[8];
	
	byte ty;
	ZCSubscreen local_subref;
	
	GUI::ListData list_sfx, list_subscript;
	
	void refr_selector();
	void refr_info();
	void refr_script();
};

#endif
