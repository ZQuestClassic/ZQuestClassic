#include "control_binding.h"
#include "info.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "base/zsys.h"
#include "zinfo.h"
#include "gui/use_size.h"
#include "zc/cheats.h"
#include <fmt/format.h>

ControlBindingDialog::ControlBindingDialog(control_scheme& scheme, string const& scheme_name):
	local_scheme(scheme), dest_scheme(scheme), scheme_name(scheme_name),
	read_only(scheme_name == DEFAULT_CONTROL_SCHEME_NAME), num_gamepads(0)
{}

// KEYBOARD
static void load_u_keys(int** arr, control_scheme& scheme)
{
	for (int q = 0; q < u_key_mod1a; ++ q)
		arr[q] = &(scheme.keys[q]);
	for (int q = 0; q < 4; ++ q)
		arr[u_key_mod1a+q] = &(scheme.cheat_modifier_keys[q]);
}
static void load_u_keys(int* arr, control_scheme const& scheme)
{
	for (int q = 0; q < u_key_mod1a; ++ q)
		arr[q] = scheme.keys[q];
	for (int q = 0; q < 4; ++ q)
		arr[u_key_mod1a+q] = scheme.cheat_modifier_keys[q];
}

static const char* u_key_names[] =
{
	"Up", "Down", "Left", "Right",
	"A", "B", "Start", "L", "R", "Map",
	"X (Ex1)", "Y (Ex2)", "Ex3", "Ex4",
	"Mod L1", "Mod R1", "Mod L2", "Mod R2",
};
static std::string get_u_key_name(int32_t k)
{
	if (k < num_u_key) return u_key_names[k];
	return "";
}

void ControlBindingDialog::load_kb_labels()
{
	int ukeys[num_u_key];
	load_u_keys(ukeys, local_scheme);
	for (int q = 0; q < num_u_key; ++q)
	{
		u_key_labels[q]->setText(get_keystr(ukeys[q]));
	}
}
// GAMEPAD
static void load_u_btns(int** arr, control_scheme& scheme)
{
	for (int q = 0; q < u_btn_menu; ++ q)
		arr[q] = &(scheme.btns[q]);
	arr[u_btn_menu] = &(scheme.btn_menu);
}
static void load_u_btns(int* arr, control_scheme const& scheme)
{
	for (int q = 0; q < u_btn_menu; ++ q)
		arr[q] = scheme.btns[q];
	arr[u_btn_menu] = scheme.btn_menu;
}

static const char* u_btn_names[] =
{
	"Up", "Down", "Left", "Right",
	"A", "B", "Start", "L", "R", "Map",
	"X (Ex1)", "Y (Ex2)", "Ex3", "Ex4",
	"Menu",
};
static std::string get_u_btn_name(int32_t k)
{
	if (k < num_u_btn) return u_btn_names[k];
	return "";
}

static string joybtn_name(int stick_idx, int b)
{
	if (stick_idx >= al_get_num_joysticks())
		return fmt::format("? {} ?", b);
	if (b <= 0 || b > joy[stick_idx].num_buttons)
		return "";

	return joy[stick_idx].button[b-1].name;
}
static string joystick_name(int stick_idx, int s)
{
	if (stick_idx >= al_get_num_joysticks())
		return fmt::format("? {} ?", s);
	if (s < 0 || s >= joy[stick_idx].num_sticks)
		return "";

	return joy[stick_idx].stick[s].name;
}

void ControlBindingDialog::load_gamepad_labels()
{
	int ubtns[num_u_btn];
	load_u_btns(ubtns, local_scheme);
	for (int q = 0; q < num_u_btn; ++q)
		u_btn_labels[q]->setText(joybtn_name(local_scheme.joystick_index, ubtns[q]));
	for (int q = 0; q < control_scheme::num_sticks; ++q)
		stick_labels[q]->setText(joystick_name(local_scheme.joystick_index, local_scheme.stick_data[q][control_scheme::axis_x][control_scheme::data_stick]));
}
void set_binding_joystick(ALLEGRO_JOYSTICK* stick);
void ControlBindingDialog::check_joystick()
{
	set_binding_joystick(nullptr);
	if (!(num_gamepads && al_get_num_joysticks() > 0)) return;
	
	if (local_scheme.joystick_index >= al_get_num_joysticks())
	{
		local_scheme.joystick_index = 0;
		refresh_dlg();
	}
	
	auto* joystick = al_get_joystick(local_scheme.joystick_index);
	if (!joystick)
	{
		InfoDialog("ZC", "Invalid gamepad. Did it disconnect?").show();
		refresh_dlg();
	}
	set_binding_joystick(joystick);
}

// CHEATS
void ControlBindingDialog::load_cheat_labels()
{
	for(size_t q = 1; q < Cheat::Last; ++q)
	{
		if(!bindable_cheat((Cheat)q)) continue;
		for(size_t alt = 0; alt <= 1; ++alt)
		{
			cheat_labels[q][alt]->setText(get_keystr(local_scheme.cheatkeys[q][alt]));
		}
	}
}
static std::string cheatName(Cheat c)
{
	switch(c)
	{
		case Cheat::Life:
			return fmt::format("Refill {}", ZI.getCtrName(crLIFE));
		case Cheat::Magic:
			return fmt::format("Refill {}", ZI.getCtrName(crMAGIC));
		case Cheat::Rupies:
			return fmt::format("Refill {}", ZI.getCtrName(crMONEY));
		case Cheat::Bombs:
			return fmt::format("Refill {}", ZI.getCtrName(crBOMBS));
		case Cheat::Arrows:
			return fmt::format("Refill {}", ZI.getCtrName(crARROWS));
		case Cheat::Kill:
			return "Kill Enemies";
		case Cheat::Fast:
			return "Quick Movement";
		case Cheat::Clock:
			return "Invincibility";
		case Cheat::Walls:
			return "Noclip";
		case Cheat::Light:
			return "Toggle Darkness";
		case Cheat::IgnoreSideView:
			return "Ignore Sideview";
		case Cheat::MaxLife:
			return fmt::format("Max {}", ZI.getCtrName(crLIFE));
		case Cheat::MaxMagic:
			return fmt::format("Max {}", ZI.getCtrName(crMAGIC));
		case Cheat::MaxBombs:
			return fmt::format("Max {}", ZI.getCtrName(crBOMBS));
		case Cheat::HeroData:
			return "Hero Data";
		case Cheat::TrigSecrets:
			return "Trigger Secrets (Temp)";
		case Cheat::TrigSecretsPerm:
			return "Trigger Secrets (Perm)";
		case Cheat::ShowL0:
		case Cheat::ShowL1:
		case Cheat::ShowL2:
		case Cheat::ShowL3:
		case Cheat::ShowL4:
		case Cheat::ShowL5:
		case Cheat::ShowL6:
			return fmt::format("Toggle Layer {}", c-Cheat::ShowL0);
		case Cheat::ShowFFC:
			return "Toggle FFCs";
		case Cheat::ShowSprites:
			return "Toggle Sprites";
		case Cheat::ShowWalkability:
			return "Toggle Walkability";
		case Cheat::ShowEffects:
			return "Toggle Effect";
		case Cheat::ShowOverhead:
			return "Toggle Overheads";
		case Cheat::ShowPushblock:
			return "Toggle Pushblocks";
		case Cheat::ShowHitbox:
			return "Toggle Hitboxes";
		case Cheat::ShowFFCScripts:
			return "Toggle FFC Scripts";
		case Cheat::ShowInfoOpacity:
			return "Change Debug Opacity";
		case Cheat::HeroPosition:
			return "Hero Position";
		default:
			return cheat_to_string(c);
	}
}

static size_t tab_pos1 = 0;
static int32_t scroll_pos[3] = {0};
std::shared_ptr<GUI::Widget> ControlBindingDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	num_gamepads = al_get_num_joysticks();
	check_joystick();
	gamepad_list.clear();
	for (int q = 0; q < num_gamepads; ++q)
	{
		ALLEGRO_JOYSTICK* joy = al_get_joystick(q);
		gamepad_list.add(fmt::format("{} ({:03})", joy ? al_get_joystick_name(joy) : "?", q), q);
	}
	
	string title_text = fmt::format("Control Scheme '{}'", scheme_name);
	if (read_only)
		title_text += " (Read-Only)";
	
	std::shared_ptr<GUI::Grid> btnrow, cheatlist, keylist, btnlist, btnheader;
	const auto button_height_per_line = 1.5_em;
	const auto button_height_min = 1.7_em;
	const auto key_lbl_width = 6_em;
	const auto btn_lbl_width = 6_em;
	const auto vsep_padding = 6_px;
	const auto cheat_name_cap = 7_em;
	{ // build keys
		int *u_keys[num_u_key];
		load_u_keys(u_keys, local_scheme);
		
		size_t num_keys = num_u_key;
		size_t col_per_button = read_only ? 1 : 3;
		size_t columns = 3 + 2 * col_per_button;
		keylist = GUI::Internal::makeRows(columns);
		vector<unique_key> left_cols[3] = {{u_key_a, u_key_ex1, u_key_ex3, u_key_l, u_key_s }, {u_key_du, u_key_dd}, {u_key_mod1a, u_key_mod1b}};
		vector<unique_key> right_cols[3] = {{u_key_b, u_key_ex2, u_key_ex4, u_key_r, u_key_p }, {u_key_dl, u_key_dr}, {u_key_mod2a, u_key_mod2b}};
		string headers[3] = {"Main Controls", "Directions", "Cheat Modifiers"};
		for (int q = 0; q < 3; ++q)
		{
			auto const& lv = left_cols[q];
			auto const& rv = right_cols[q];
			size_t count = zc_max(lv.size(), rv.size());
			keylist->add(Label(text = headers[q], colSpan = columns, bottomPadding = 3_px, topPadding = 6_px));
			for (int c = 0; c < count; ++c)
			{
				if (c < lv.size())
				{
					string name = get_u_key_name(lv[c]);
					keylist->add(Frame(padding = 0_px, fitParent = true, Label(text = fmt::format("{}:", name), leftPadding = DEFAULT_PADDING + 0.75_em)));
					keylist->add(u_key_labels[lv[c]] = Label(minwidth = key_lbl_width, textAlign = 1));
					if (!read_only)
					{
						keylist->add(Button(text = "Bind", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_KB,
							bind_name = name, kb_ptr = u_keys[lv[c]],
							 onClick = message::RELOAD_KEYBOARD));
						keylist->add(Button(text = "Clear", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_KB_CLEAR,
							bind_name = name, kb_ptr = u_keys[lv[c]],
							 onClick = message::RELOAD_KEYBOARD));
					}
				}
				else keylist->add(DummyWidget(colSpan = read_only ? 2 : 4));
				if (c == 0)
					keylist->add(VSeparator(rowSpan = count, leftPadding = vsep_padding, rightPadding = vsep_padding + 2_px));
				if (c < rv.size())
				{
					string name = get_u_key_name(rv[c]);
					keylist->add(Frame(padding = 0_px, fitParent = true, Label(text = fmt::format("{}:", name), leftPadding = DEFAULT_PADDING + 0.75_em)));
					keylist->add(u_key_labels[rv[c]] = Label(minwidth = key_lbl_width, textAlign = 1));
					if (!read_only)
					{
						keylist->add(Button(text = "Bind", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_KB,
							bind_name = name, kb_ptr = u_keys[rv[c]],
							onClick = message::RELOAD_KEYBOARD));
						keylist->add(Button(text = "Clear", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_KB_CLEAR,
							bind_name = name, kb_ptr = u_keys[rv[c]],
							onClick = message::RELOAD_KEYBOARD));
					}
				}
				else keylist->add(DummyWidget(colSpan = read_only ? 2 : 4));
			}
		}
	}
	bool gamepad_read_only = read_only || !num_gamepads;
	{ // build gamepad
		int *u_btns[num_u_btn];
		load_u_btns(u_btns, local_scheme);
		
		size_t num_keys = num_u_btn;
		size_t col_per_button = gamepad_read_only ? 1 : 2;
		size_t columns = 3 + 2 * col_per_button;
		btnlist = GUI::Internal::makeRows(columns);
		vector<unique_btn> left_cols[2] = {{u_btn_a, u_btn_ex1, u_btn_ex3, u_btn_l, u_btn_s, u_btn_menu }, {u_btn_du, u_btn_dd}};
		vector<unique_btn> right_cols[2] = {{u_btn_b, u_btn_ex2, u_btn_ex4, u_btn_r, u_btn_p }, {u_btn_dl, u_btn_dr}};
		string headers[2] = {"Main Controls", "Directions"};
		for (int q = 0; q < 2; ++q)
		{
			auto const& lv = left_cols[q];
			auto const& rv = right_cols[q];
			size_t count = zc_max(lv.size(), rv.size());
			btnlist->add(Label(text = headers[q], colSpan = columns, bottomPadding = 3_px, topPadding = q == 0 ? 0_px : 3_px));
			for (int c = 0; c < count; ++c)
			{
				if (c < lv.size())
				{
					string name = get_u_btn_name(lv[c]);
					btnlist->add(Frame(padding = 0_px, fitParent = true, Label(text = fmt::format("{}:", name), leftPadding = DEFAULT_PADDING + 0.75_em)));
					btnlist->add(u_btn_labels[lv[c]] = Label(minwidth = btn_lbl_width, textAlign = 1));
					if (!gamepad_read_only)
						btnlist->add(Button(text = "Bind", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_JOYKEY,
							stick_index_ptr = &local_scheme.joystick_index, bind_name = name,
							kb_ptr = u_btns[lv[c]], onClick = message::RELOAD_GAMEPAD));
				}
				else btnlist->add(DummyWidget(colSpan = gamepad_read_only ? 2 : 3));
				if (c == 0)
					btnlist->add(VSeparator(rowSpan = count, leftPadding = vsep_padding, rightPadding = vsep_padding + 2_px));
				if (c < rv.size())
				{
					string name = get_u_btn_name(rv[c]);
					btnlist->add(Frame(padding = 0_px, fitParent = true, Label(text = fmt::format("{}:", name), leftPadding = DEFAULT_PADDING + 0.75_em)));
					btnlist->add(u_btn_labels[rv[c]] = Label(minwidth = btn_lbl_width, textAlign = 1));
					if (!gamepad_read_only)
						btnlist->add(Button(text = "Bind", height = button_height_min,
							padding = 0_px, type = GUI::Button::type::BIND_JOYKEY,
							stick_index_ptr = &local_scheme.joystick_index, bind_name = name,
							kb_ptr = u_btns[rv[c]], onClick = message::RELOAD_GAMEPAD));
				}
				else btnlist->add(DummyWidget(colSpan = gamepad_read_only ? 2 : 3));
			}
		}
		{ // sticks
			btnlist->add(
				Row(colSpan = columns,
					Checkbox(
						text = "Use Analog Sticks:",
						checked = local_scheme.analog_movement,
						onToggleFunc = [&](bool state)
						{
							local_scheme.analog_movement = state;
						}),
					INFOBTN_T("Analog Movement", "If checked, uses the sticks below instead of the directions above.")
				)
			);
			btnlist->add(Frame(padding = 0_px, fitParent = true, Label(text = "Primary", leftPadding = DEFAULT_PADDING + 0.75_em)));
			btnlist->add(stick_labels[0] = Label(minwidth = btn_lbl_width, textAlign = 1));
			if (!gamepad_read_only)
				btnlist->add(Button(text = "Bind", height = button_height_min,
					padding = 0_px, type = GUI::Button::type::BIND_JOYSTICK,
					stick_index_ptr = &local_scheme.joystick_index, bind_name = "Primary",
					kb_ptr = &local_scheme.stick_data[control_scheme::stick_1][control_scheme::axis_x][control_scheme::data_stick],
					onClick = message::RELOAD_GAMEPAD));
			btnlist->add(VSeparator(leftPadding = vsep_padding, rightPadding = vsep_padding + 2_px));
			btnlist->add(Frame(padding = 0_px, fitParent = true, Label(text = "Secondary", leftPadding = DEFAULT_PADDING + 0.75_em)));
			btnlist->add(stick_labels[1] = Label(minwidth = btn_lbl_width, textAlign = 1));
			if (!gamepad_read_only)
				btnlist->add(Button(text = "Bind", height = button_height_min,
					padding = 0_px, type = GUI::Button::type::BIND_JOYSTICK,
					stick_index_ptr = &local_scheme.joystick_index, bind_name = "Secondary",
					kb_ptr = &local_scheme.stick_data[control_scheme::stick_2][control_scheme::axis_x][control_scheme::data_stick],
					onClick = message::RELOAD_GAMEPAD));
		}
	}
	{ // build cheats
		std::map<int, std::vector<Cheat>> bindable_cheats;
		size_t num_cheats = 0;
		for(size_t q = 1; q < Cheat::Last; ++q)
		{
			Cheat c = (Cheat)q;
			if(!bindable_cheat(c)) continue;
			++num_cheats;
			bindable_cheats[cheat_lvl(c)].push_back(c);
		}
		
		size_t col_per_button = read_only ? 1 : 3;
		
		size_t columns = 3 + (col_per_button * 2) + (read_only ? 0 : 2);
		cheatlist = GUI::Internal::makeRows(columns);
		cheatlist->add(Label(text = "Lvl"));
		cheatlist->add(Label(text = "Cheat", colSpan = 2));
		if (!read_only)
			cheatlist->add(VSeparator(rowSpan = num_cheats+1, leftPadding = vsep_padding, rightPadding = vsep_padding + 2_px));
		cheatlist->add(Label(text = "Main Key", colSpan = col_per_button));
		if (!read_only)
			cheatlist->add(VSeparator(rowSpan = num_cheats+1, leftPadding = vsep_padding, rightPadding = vsep_padding + 2_px));
		cheatlist->add(Label(text = "Alternate Key", colSpan = col_per_button));
		
		for(auto& pair : bindable_cheats)
		{
			for(Cheat c : pair.second)
			{
				cheatlist->add(Label(text = std::to_string(cheat_lvl(c))));
				string name = cheatName(c);
				auto cheat_text = fmt::format("{}:",name);
				auto num_lines = 1 + (gui_text_width(GUI_DEF_FONT, cheat_text.c_str()) / cheat_name_cap);
				cheatlist->add(Label(text = cheat_text, hAlign = 1.0, textAlign = 2, maxwidth = cheat_name_cap, maxLines = num_lines));
				auto bh = zc_max(button_height_min, button_height_per_line * num_lines);
				cheatlist->add(INFOBTN_EX(cheat_help(c), height = bh, padding = 0_px));
				// vsep has rowspan here if not read_only
				cheatlist->add(cheat_labels[c][0] = Label(minwidth = key_lbl_width, textAlign = 1));
				if (!read_only)
				{
					cheatlist->add(Button(text = "Bind", height = bh,
						padding = 0_px, type = GUI::Button::type::BIND_KB,
						bind_name = name, kb_ptr = &(local_scheme.cheatkeys[c][0]),
						onClick = message::RELOAD_CHEATS));
					cheatlist->add(Button(text = "Clear", height = bh,
						padding = 0_px, type = GUI::Button::type::BIND_KB_CLEAR,
						bind_name = name, kb_ptr = &(local_scheme.cheatkeys[c][0]),
						onClick = message::RELOAD_CHEATS));
				}
				// vsep has rowspan here if not read_only
				cheatlist->add(cheat_labels[c][1] = Label(minwidth = key_lbl_width, textAlign = 1));
				if (!read_only)
				{
					// vsep has rowspan here
					cheatlist->add(Button(text = "Bind", height = bh,
						padding = 0_px, type = GUI::Button::type::BIND_KB,
						bind_name = name, kb_ptr = &(local_scheme.cheatkeys[c][1]),
						onClick = message::RELOAD_CHEATS));
					cheatlist->add(Button(text = "Clear", height = bh,
						padding = 0_px, type = GUI::Button::type::BIND_KB_CLEAR,
						bind_name = name, kb_ptr = &(local_scheme.cheatkeys[c][1]),
						onClick = message::RELOAD_CHEATS));
				}
			}
		}
	}
	
	btnheader = Column(padding = 0_px);
	if (!read_only && !num_gamepads)
	{
		btnheader->add(Label(text = "No gamepad connected! Connect a gamepad to bind gamepad controls."));
	}
	else
	{
		btnheader->add(Column(padding = 0_px,
			Row(padding = 0_px,
				Label(text = "Gamepad:"),
				DropDownList(data = gamepad_list,
					fitParent = true, minwidth = 50_px,
					selectedValue = local_scheme.joystick_index,
					onSelectFunc = [&](int val)
					{
						local_scheme.joystick_index = val;
						refresh_dlg();
					}
				)
			)
		));
	}
	btnheader->add(btnlist);
	const auto pane_height = 480_px - 8_em;
	std::shared_ptr<GUI::Window> window = Window(
		title = title_text,
		use_vsync = true,
		onTick = [&]()
		{
			// For Gamepad tab only, refresh if num gamepads changed
			if (tab_pos1 == 1 && al_get_num_joysticks() != num_gamepads)
				refresh_dlg();

			return ONTICK_CONTINUE;
		},
		onClose = message::CANCEL,
		Column(padding = 0_px,
			maxwidth = Size::pixels(zq_screen_w),
			TabPanel(ptr = &tab_pos1, padding = 0_px,
				TabRef(name = "Keyboard", ScrollingPane(
					ptr_y = &scroll_pos[0],
					fitParent = true,
					minheight = pane_height,
					Column(hPadding = 0_px, vPadding = 0.5_em,
						keylist
					)
				)),
				TabRef(name = "Gamepad", ScrollingPane(
					ptr_y = &scroll_pos[0],
					fitParent = true,
					minheight = pane_height,
					Column(hPadding = 0_px, vPadding = 0.5_em,
						btnheader
					)
				)),
				TabRef(name = "Cheat Keys", ScrollingPane(
					ptr_y = &scroll_pos[2],
					fitParent = true,
					minheight = pane_height,
					Column(hPadding = 0_px, vPadding = 0.5_em,
						cheatlist
					)
				))
			),
			btnrow = Row(
				vPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK)
			)
		)
	);
	
	if (!read_only)
		btnrow->add(Button(text = "Cancel", minwidth = 90_px, onClick = message::CANCEL));
	
	load_kb_labels();
	load_gamepad_labels();
	load_cheat_labels();
	return window;
}

bool ControlBindingDialog::check_no_duplicate_keys() const
{
	std::vector<std::string> uniqueError;
	char buf[512];
	
	{
		int u_keys[num_u_key];
		load_u_keys(u_keys, local_scheme);
		for(int32_t q = 0; q < num_u_key; ++q)
		{
			for(int32_t p = q+1; p < num_u_key; ++p)
			{
				if(u_keys[q] == u_keys[p] && u_keys[q] != 0)
				{
					sprintf(buf, "Key '%s' conflicts with '%s'", get_u_key_name(q).c_str(), get_u_key_name(p).c_str());
					uniqueError.emplace_back(buf);
				}
			}
		}
	}
	
	{
		int u_btns[num_u_btn];
		load_u_btns(u_btns, local_scheme);
		for(int32_t q = 0; q < num_u_btn; ++q)
		{
			for(int32_t p = q+1; p < num_u_btn; ++p)
			{
				if(u_btns[q] == u_btns[p] && u_btns[q] != 0)
				{
					sprintf(buf, "Button '%s' conflicts with '%s'", get_u_btn_name(q).c_str(), get_u_btn_name(p).c_str());
					uniqueError.emplace_back(buf);
				}
			}
		}
		// sticks don't care
	}
	
	{ // Cheats only conflict with cheats, due to modifier keys
		for(size_t q = 1; q < Cheat::Last; ++q)
		{
			if(!bindable_cheat((Cheat)q)) continue;
			for(size_t p = q+1; p < Cheat::Last; ++p)
			{
				if(!bindable_cheat((Cheat)p)) continue;
				for(size_t q2 = 0; q2 <= 1; ++q2)
					for(size_t p2 = 0; p2 <= 1; ++p2)
					{
						if(local_scheme.cheatkeys[q][q2] == local_scheme.cheatkeys[p][p2] && local_scheme.cheatkeys[q][q2] != 0)
						{
							uniqueError.push_back(fmt::format("Cheat '{}' ({}) conflicts with '{}' ({}) - both '{}'",
								cheat_to_string((Cheat)q), q2?"Alt":"Main",
								cheat_to_string((Cheat)p), p2?"Alt":"Main",
								get_keystr(local_scheme.cheatkeys[q][q2])));
						}
					}
			}
		}
	}
	
	if(uniqueError.size() == 0)
		return true;
	
	box_start(1, "Duplicate Keys", get_zc_font(font_lfont), get_zc_font(font_sfont), false, 500,400, 2);
	box_out("Cannot have duplicate keybinds!"); box_eol();
	for(std::vector<std::string>::iterator it = uniqueError.begin();
		it != uniqueError.end(); ++it)
	{
		box_out((*it).c_str()); box_eol();
	}
	box_end(true);
	return false;
}
bool ControlBindingDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::RELOAD_KEYBOARD:
			load_kb_labels();
			break;
		case message::RELOAD_GAMEPAD:
			load_gamepad_labels();
			break;
		case message::RELOAD_CHEATS:
			load_cheat_labels();
			break;
		case message::OK:
		{
			if (read_only)
				return true;
			local_scheme.simplify();
			if (!check_no_duplicate_keys())
				return false;
			
			dest_scheme = local_scheme;
			dest_scheme.save();
			return true;
		}
		case message::CANCEL:
			return true;
	}
	return false;
}

