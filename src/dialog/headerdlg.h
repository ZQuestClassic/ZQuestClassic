#ifndef ZC_DIALOG_HEADERDLG_H
#define ZC_DIALOG_HEADERDLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_header_dlg();

class HeaderDialog: public GUI::Dialog<HeaderDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TITLE, AUTHOR };

	HeaderDialog(std::string verstr, std::string initVals[5], std::function<void(std::string_view[5])> setVals);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TextField> questRev, minRev, titlestr, author, questNum;
	std::string vals[5];
	std::string verstr;
	std::shared_ptr<GUI::Label> titleLabel, authorLabel;
	std::function<void(std::string_view[5])> setVals;
};

#endif
