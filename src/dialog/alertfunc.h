#ifndef ZC_DIALOG_ALERTFUNC_H_
#define ZC_DIALOG_ALERTFUNC_H_

#include "info.h"
#include <gui/button.h>
#include <gui/grid.h>

// A basic dialog that just shows some lines of text and a set of function buttons
class AlertFuncDialog: public InfoDialog
{
public:
	AlertFuncDialog(string const& title, string const& text, string info = "",
		uint32_t numButtons = 0, uint32_t focused_button = 0,
		std::initializer_list<string> buttonNames = {},
		std::initializer_list<std::function<bool()>> buttonProcs = {});
	AlertFuncDialog(string const& title, vector<string> const& lines,
		string info = "", uint32_t numButtons = 0, uint32_t focused_button = 0,
		std::initializer_list<string> buttonNames = {},
		std::initializer_list<std::function<bool()>> buttonProcs = {});
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg) override;

private:
	bool didend;
	string helptxt;
	std::shared_ptr<GUI::Grid> buttonRow;
	vector<std::shared_ptr<GUI::Button>> buttons;
	
	void initButtons(std::initializer_list<string> buttonNames,
		std::initializer_list<std::function<bool()>> buttonProcs,
		uint32_t numButtons, uint32_t focused_button);
};

#endif
