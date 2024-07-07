#ifndef ZC_DIALOG_ALERT_H_
#define ZC_DIALOG_ALERT_H_

#include "info.h"

void info_dsa(string const& title, string const& text, string const& dsastr);
bool alert_confirm(string const& title, string const& text, bool okc = false);

// A basic dialog that just shows some lines of text and a yes/no button
class AlertDialog: public InfoDialog
{
public:
	AlertDialog(string const& title, string const& text, std::function<void(bool,bool)> onEnd, string truebtn = "OK", string falsebtn = "Cancel", uint32_t timeout = 0, bool default_ret = false, bool dontshow = false);
	AlertDialog(string const& title, vector<string> const& lines, std::function<void(bool,bool)> onEnd, string truebtn = "OK", string falsebtn = "Cancel", uint32_t timeout = 0, bool default_ret = false, bool dontshow = false);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg) override;

private:
	string truebtn, falsebtn;
	int32_t alert_on_tick();
	uint32_t timer, timeout;
	bool default_ret;
	bool dontshowagain;
	std::function<void(bool,bool)> onEnd;
};

#endif
