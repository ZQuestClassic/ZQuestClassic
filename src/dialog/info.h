#ifndef ZC_DIALOG_INFO_H
#define ZC_DIALOG_INFO_H

#include "base/qrs.h"
#include "base/headers.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>
#include "dialog/externs.h"

class InfoDialog: public GUI::Dialog<InfoDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QR, BTN };

	InfoDialog(string const& title, string const& text, optional<string> subtext = nullopt);
	InfoDialog(string const& title, vector<string> const& lines, optional<string> subtext = nullopt);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	string d_title, d_text;
	optional<string> d_subtext;
	
	std::set<int> qrs; //related qrs
	std::set<int> ruleTemplates; //related rule templates
	
	byte local_qrs[QR_SZ];
	byte* dest_qrs;
	byte* old_dest_qrs;
	bool on_templates[sz_ruletemplate] = {false};
	
	void postinit();
};

#endif
