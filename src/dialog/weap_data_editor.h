#ifndef ZC_DIALOG_WEAPDATAEDITOR_H_
#define ZC_DIALOG_WEAPDATAEDITOR_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/grid.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <gui/list_data.h>
#include <gui/switcher.h>
#include <zq/gui/tileanim_frame.h>
#include <functional>
#include <string_view>
#include <map>
#include "items.h"
#include "core/flags.h"

void call_weap_data_editor(weapon_data& weap, bool lweapon, bool togglable = false);

class WeaponDataDialog: public GUI::Dialog<WeaponDataDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };
	

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	WeaponDataDialog(weapon_data& ref, bool lweapon, bool togglable);
	weapon_data local_ref;
	weapon_data& dest_ref;
	bool is_lw, togglable;
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Grid> burn_grid;
	std::shared_ptr<GUI::TextField> glow_field_base, glow_off_field_base, step_tf;
	std::shared_ptr<GUI::Label> glow_label, glow_off_label;
	std::shared_ptr<GUI::Checkbox> burn_box, glow_box;
	GUI::ListData list_sprites, list_weaptype, list_sfx, list_light_shapes,
		list_deftypes, list_lwpnscripts, list_ewpnscripts;
	
	void refresh_burnglow();
	
	std::shared_ptr<GUI::Widget> MoveFlag(move_flags index, string const& str);
	friend void call_weap_data_editor(weapon_data& weap, bool lweapon, bool togglable);
};

#endif
