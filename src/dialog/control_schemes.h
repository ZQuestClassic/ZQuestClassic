#ifndef ZC_DIALOG_CONTROLSCHEME_H_
#define ZC_DIALOG_CONTROLSCHEME_H_

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/button.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "zc/control_scheme.h"

class ControlSchemeDialog: public GUI::Dialog<ControlSchemeDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ControlSchemeDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	GUI::ListData schemes_list_no_none, schemes_list_quest;
	std::shared_ptr<GUI::DropDownList> scheme_global, scheme_quest, scheme_edit;
	std::shared_ptr<GUI::Button> edit_buttons[3];
	std::string button_word;
	
	void generate_lists();
	void refresh_edit_buttons();
};

#endif

