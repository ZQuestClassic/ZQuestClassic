#ifndef ZC_DIALOG_ALERT_H
#define ZC_DIALOG_ALERT_H

#include "info.h"

// A basic dialog that just shows some lines of text and a yes/no button
class AlertDialog: public InfoDialog
{
public:
	enum class message { OK, CANCEL };

	AlertDialog(std::string title, std::string text, std::function<void(bool)> onEnd);
	AlertDialog(std::string title, std::vector<std::string_view> lines, std::function<void(bool)> onEnd);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<int>& msg) override;

private:
	std::function<void(bool)> onEnd;
};

#endif
