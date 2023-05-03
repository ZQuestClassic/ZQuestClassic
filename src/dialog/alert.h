#ifndef ZC_DIALOG_ALERT_H
#define ZC_DIALOG_ALERT_H

#include "info.h"

// A basic dialog that just shows some lines of text and a yes/no button
class AlertDialog: public InfoDialog
{
public:
	AlertDialog(std::string title, std::string text, std::function<void(bool,bool)> onEnd, std::string truebtn = "OK", std::string falsebtn = "Cancel", uint32_t timeout = 0, bool default_ret = false, bool dontshow = false);
	AlertDialog(std::string title, std::vector<std::string_view> lines, std::function<void(bool,bool)> onEnd, std::string truebtn = "OK", std::string falsebtn = "Cancel", uint32_t timeout = 0, bool default_ret = false, bool dontshow = false);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg) override;

private:
	std::string truebtn, falsebtn;
	int32_t alert_on_tick();
	uint32_t timer, timeout;
	bool default_ret;
	bool dontshowagain;
	std::function<void(bool,bool)> onEnd;
};

#endif
