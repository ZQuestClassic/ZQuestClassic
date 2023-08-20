#ifndef ZC_DIALOG_INFO_H
#define ZC_DIALOG_INFO_H

#include "base/qrs.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>

//A basic handler function
void displayinfo(std::string title, std::string text);

// A basic dialog that just shows some lines of text and a close button.
class InfoDialog: public GUI::Dialog<InfoDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QR, BTN };

	InfoDialog(std::string title, std::string text);
	InfoDialog(std::string title, std::vector<std::string_view> lines);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	std::string dlgTitle;
	std::string dlgText;
	
	std::set<int> qrs; //related qrs
	std::set<int> ruleTemplates; //related rule templates
	
	byte local_qrs[QR_SZ];
	byte* dest_qrs;
	byte* old_dest_qrs;
	bool on_templates[sz_ruletemplate] = {false};
	
	void postinit();
};

#endif
