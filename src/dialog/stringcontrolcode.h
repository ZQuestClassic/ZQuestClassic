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
		OK, CANCEL, RELOAD, COPY, PASTE
	};

	SCCDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	byte curscc;
	int32_t args[256][6];
	GUI::ListData list_sccs, list_shtype, list_strings, list_items, list_counters, list_dmaps,
		list_weffect, list_sfx, list_midi, list_screenstate, list_font, list_font_order;
	std::shared_ptr<GUI::DropDownList> fontlist;
	int32_t* cur_args;
	bool warp_xy_toggle;
	
	void default_args();
	bool load_scc_str(std::string const& str);
};

#endif
