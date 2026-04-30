#ifndef ZC_DIALOG_INFO_H_
#define ZC_DIALOG_INFO_H_

#include "core/qrs.h"
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

	InfoDialog(string const& title, string const& text);
	InfoDialog(string const& title, vector<string> const& lines);
	
	InfoDialog& set_subtext(optional<string> subtext);
	InfoDialog& set_dest_qrs(byte* dest_qrs);
	InfoDialog& set_text_align(int text_align);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	std::shared_ptr<GUI::Window> window;
	string d_title, d_text;
	optional<string> d_subtext;
	int d_text_align;
	bool _has_run_postinit;
	
	std::set<int> qrs; //related qrs
	std::set<int> ruleTemplates; //related rule templates
	
	byte local_qrs[QR_SZ];
	byte* d_dest_qrs;
	byte* old_dest_qrs;
	bool on_templates[sz_ruletemplate] = {false};
	
	void postinit();
	
	std::shared_ptr<GUI::Grid> build_text();
};

#endif
