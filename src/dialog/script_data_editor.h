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
#include "base/general.h"

struct ExportWidgetData
{
	word index = 0;
	std::shared_ptr<GUI::Widget> main_widget;
	std::shared_ptr<GUI::Button> reset_button;
	std::function<void(int)> set_widg_val;
	std::function<void(int)> set_arg;
	exported_variable const* expdata = nullptr;
	GUI::ListData list {};
	std::vector<CheckListInfo> clist_info {};
};

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
	
	std::map<word, ExportWidgetData> export_widgets;
	
	GUI::ListData list_engine_item, list_engine_enemy, list_engine_counter, list_engine_sprite,
		list_engine_sfx, list_engine_midi, list_engine_music, list_engine_save_menu,
		list_engine_message_string, list_engine_weapon_type, list_engine_lweapon_type,
		list_engine_eweapon_type, list_engine_dropset, list_engine_font_id,
		list_engine_bottle_type, list_engine_combo_type, list_engine_combo_flag;
	
	std::shared_ptr<GUI::Window> window;
};

#endif

