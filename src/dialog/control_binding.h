#pragma once

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "gui/tabpanel.h"
#include "zc/cheats.h"
#include "zc/control_scheme.h"

enum unique_key
{
	u_key_du, u_key_dd, u_key_dl, u_key_dr,
	u_key_a, u_key_b, u_key_s, u_key_l, u_key_r, u_key_p,
	u_key_ex1, u_key_ex2, u_key_ex3, u_key_ex4,
	u_key_mod1a, u_key_mod2a, u_key_mod1b, u_key_mod2b,
	num_u_key
};
enum unique_btn
{
	u_btn_du, u_btn_dd, u_btn_dl, u_btn_dr,
	u_btn_a, u_btn_b, u_btn_s, u_btn_l, u_btn_r, u_btn_p,
	u_btn_ex1, u_btn_ex2, u_btn_ex3, u_btn_ex4,
	u_btn_menu,
	num_u_btn
};
class ControlBindingDialog: public GUI::Dialog<ControlBindingDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RELOAD_KEYBOARD, RELOAD_GAMEPAD, RELOAD_CHEATS };

	ControlBindingDialog(control_scheme& scheme, string const& scheme_name);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	control_scheme local_scheme;
	control_scheme& dest_scheme;
	std::string const& scheme_name;
	bool read_only;
	int num_gamepads;
	std::shared_ptr<GUI::Label> cheat_labels[Cheat::Last][2];
	std::shared_ptr<GUI::Label> u_key_labels[num_u_key];
	std::shared_ptr<GUI::Label> u_btn_labels[num_u_btn];
	std::shared_ptr<GUI::Label> stick_labels[control_scheme::num_sticks];
	
	GUI::ListData gamepad_list;

	void load_kb_labels();
	void load_gamepad_labels();
	void load_cheat_labels();
	
	void check_joystick();
	
	bool check_no_duplicate_keys() const;
};
