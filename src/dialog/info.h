#ifndef ZC_DIALOG_INFO_H_
#define ZC_DIALOG_INFO_H_

#include "base/qrs.h"
#include "base/headers.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/grid.h>
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

	InfoDialog(string const& title, string const& text, optional<string> subtext = nullopt, byte* dest_qrs = nullptr, int text_align = 1);
	InfoDialog(string const& title, vector<string> const& lines, optional<string> subtext = nullopt, byte* dest_qrs = nullptr, int text_align = 1);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
	
	void setSubtext(optional<string> subtext) {d_subtext = subtext;}
protected:
	std::shared_ptr<GUI::Window> window;
	string d_title, d_text;
	optional<string> d_subtext;
	int _text_align;
	
	
	std::set<int> qrs; //related qrs
	std::set<int> ruleTemplates; //related rule templates
	
	byte local_qrs[QR_SZ];
	byte* dest_qrs;
	byte* old_dest_qrs;
	bool on_templates[sz_ruletemplate] = {false};
	
	void postinit();
	
	std::shared_ptr<GUI::Grid> build_text();
};

#endif
