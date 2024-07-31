#ifndef ZC_DIALOG_COMPILESETTING_H
#define ZC_DIALOG_COMPILESETTING_H

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/label.h>
#include <functional>
#include <string_view>
#include <array>
#include "parser/parserDefs.h"

void call_compile_settings();

class CompileSettingsDlg: public GUI::Dialog<CompileSettingsDlg>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QUEST_CFG };

	CompileSettingsDlg();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> include_label;
	
	int dd_cfg[3];
	int old_dd_cfg[3];
	char include_str[MAX_INCLUDE_PATH_CHARS];
	byte qst_cfg[4];
	word timeout_secs, old_timeout_secs;
	
	void load();
	void save();
};

#endif
