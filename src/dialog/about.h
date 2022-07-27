#ifndef ZC_DIALOG_ABOUT_H
#define ZC_DIALOG_ABOUT_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <string_view>

// A basic dialog that just shows some lines of text and a close button.
class AboutDialog: public GUI::Dialog<AboutDialog>
{
public:
	using message = int32_t;

	AboutDialog(std::string title, std::string text);

	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<int32_t>& msg);

protected:
	std::string dlgTitle;
	std::string dlgText;
};

#endif
