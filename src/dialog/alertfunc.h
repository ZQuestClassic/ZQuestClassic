#ifndef ZC_DIALOG_ALERTFUNC_H
#define ZC_DIALOG_ALERTFUNC_H

#include "info.h"
#include <gui/button.h>
#include <gui/grid.h>

// A basic dialog that just shows some lines of text and a set of function buttons
class AlertFuncDialog: public InfoDialog
{
public:
	AlertFuncDialog(std::string title, std::string text, std::string info = "", uint32_t numButtons = 0, uint32_t focused_button = 0, ...);
	AlertFuncDialog(std::string title, std::vector<std::string_view> lines, std::string info = "", uint32_t numButtons = 0, uint32_t focused_button = 0, ...);
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg) override;

private:
	bool didend;
	std::string helptxt;
	std::shared_ptr<GUI::Grid> buttonRow;
	std::vector<std::shared_ptr<GUI::Button>> buttons;
	
	void initButtons(va_list args, uint32_t numButtons, uint32_t focused_button);
};

#endif
