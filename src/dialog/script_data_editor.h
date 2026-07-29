#ifndef ZC_DIALOG_SCRDATAEDITORDLG_H_
#define ZC_DIALOG_SCRDATAEDITORDLG_H_

#include <gui/dialog.h>
#include <gui/grid.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include <array>

class ScriptDataDialog: public GUI::Dialog<ScriptDataDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ScriptDataDialog(std::string const& title, script_config& cfg,
		GUI::ListData const& scripts, script_data const* const* scrdatas);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	script_config& dest_ref;
	script_config local_ref;
	
	map<word, map<word, int>> init_args;
	map<word, bitstring> specified_args;
	
	std::string title_str;
	GUI::ListData const& list_scriptchoices;
	script_data const* const* scrdatas;
	
	std::shared_ptr<GUI::Window> window;
};

#endif
