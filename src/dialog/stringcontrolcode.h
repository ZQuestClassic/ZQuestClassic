#ifndef ZC_DIALOG_SCC_H
#define ZC_DIALOG_SCC_H

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>

std::string run_scc_dlg(MsgStr const* ref);

class SCCDialog: public GUI::Dialog<SCCDialog>
{
public:
	enum class message
	{
		OK, CANCEL, RELOAD
	};

	SCCDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	byte curscc;
	word args[256][5];
	GUI::ListData list_sccs, list_shtype, list_strings, list_items, list_counters;
	word* cur_args;
	
	void default_args();
};

#endif
