#ifndef ZC_DIALOG_INFO_H
#define ZC_DIALOG_INFO_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <string_view>

//A basic handler function
void displayinfo(std::string title, std::string text);

// A basic dialog that just shows some lines of text and a close button.
class InfoDialog: public GUI::Dialog<InfoDialog>
{
public:
	using message = int32_t;

	InfoDialog(std::string title, std::string text);
	InfoDialog(std::string title, std::vector<std::string_view> lines);

	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<int32_t>& msg);

protected:
	std::string dlgTitle;
	std::string dlgText;
};

#endif
