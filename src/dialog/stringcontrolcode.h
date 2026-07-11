#ifndef ZC_DIALOG_SCC_H_
#define ZC_DIALOG_SCC_H_

#include "core/msgstr.h"
#include "components/scc/scc.h"
#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <zq/gui/selcombo_swatch.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>

std::string run_scc_dlg(MsgStr const* ref, int current_scc_index);

class SCCDialog: public GUI::Dialog<SCCDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, RELOAD, COPY, PASTE
	};

	SCCDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	byte curscc;
	bounded_map<word, scc_arg_array> args;
	GUI::ListData list_sccs, list_shtype, list_strings, list_items, list_counters, list_dmaps,
		list_weffect, list_sfx, list_midi, list_screenstate, list_level_items, list_font, list_font_order,
		list_genscr, list_bottletypes, list_music;
	std::shared_ptr<GUI::DropDownList> fontlist;
	zasm_meta meta;
	
	void refresh_music_list();
	bool load_scc_str(std::string const& str);
};

#endif
