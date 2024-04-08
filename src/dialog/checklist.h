#ifndef ZC_DIALOG_CHECKLIST_H
#define ZC_DIALOG_CHECKLIST_H

#include "base/headers.h"
#include "base/containers.h"
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

class ChecklistDialog: public GUI::Dialog<ChecklistDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ChecklistDialog(string const& title, vector<CheckListInfo> const& flagnames,
		bitstring& flags, bool& confirm);
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	string d_title;
	vector<CheckListInfo> const& flagnames;
	bitstring& flags;
	bool& confirm;
	std::shared_ptr<GUI::Window> window;
};

#endif
