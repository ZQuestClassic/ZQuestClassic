#ifndef ZC_DIALOG_ALERTFUNC_H_
#define ZC_DIALOG_ALERTFUNC_H_

#include "info.h"
#include <gui/button.h>
#include <gui/grid.h>

// A basic dialog that just shows some lines of text and a set of function buttons
class AlertFuncDialog: public InfoDialog
{
public:
	AlertFuncDialog(string const& title, string const& text, string info = "");
	AlertFuncDialog(string const& title, vector<string> const& lines, string info = "");
	
	// Giving a list of procs assigns each proc to a button
	AlertFuncDialog& add_buttons(uint32_t focused_button, std::initializer_list<string> buttonNames,
		std::initializer_list<std::function<bool()>> buttonProcs);
	// Giving an int& sets it to the clicked button, no procs needed
	AlertFuncDialog& add_buttons(uint32_t focused_button, std::initializer_list<string> buttonNames, int& chosen);
	
	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg) override;

private:
	bool didend;
	string helptxt;
	std::shared_ptr<GUI::Grid> buttonRow;
	vector<std::shared_ptr<GUI::Button>> buttons;
	int* chosen_ptr;
	
	void initButtons(std::initializer_list<string> buttonNames,
		std::vector<std::function<bool()>> buttonProcs,
		uint32_t focused_button);
};

#endif
