// See 'zc/control_scheme.cpp' for design comments
#include "control_schemes.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "base/zsys.h"
#include "zinfo.h"
#include "gui/use_size.h"
#include "zc/cheats.h"
#include "dialog/control_binding.h"
#include <fmt/format.h>

extern const char* qstpath;
extern string quest_control_path;

// Cached scheme names for dialog editing
static string global_scheme;
static optional<string> quest_scheme;
static string edit_scheme;

static bool ret = false;
static void reload_schemes()
{
	global_scheme = global_control_scheme_name;
	quest_scheme = quest_control_scheme_name;
	edit_scheme = quest_scheme ? *quest_scheme : global_scheme;
}
bool edit_controls_dialog()
{
	ret = false;
	reload_schemes();
	ControlSchemeDialog().show();
	if (ret)
	{
		global_control_scheme_name = global_scheme;
		quest_control_scheme_name = quest_scheme;
		save_schemes();
	}
	refresh_control_scheme();
	return ret;
}

ControlSchemeDialog::ControlSchemeDialog(){}

std::shared_ptr<GUI::Widget> ControlSchemeDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	bool quest_active = !quest_control_path.empty();
	string quest_str = quest_active ? quest_control_path.substr(quest_control_prefix.size()) : "[No Quest Loaded]";
	
	scheme_global = DropDownList(data = schemes_list_no_none,
		fitParent = true, maxwidth = 250_px,
		onSelectFunc = [&](int32_t val)
		{
			global_scheme = schemes_list_no_none.findText(val);
		});
	scheme_quest = DropDownList(data = schemes_list_quest,
		fitParent = true, maxwidth = 250_px,
		disabled = !quest_active,
		onSelectFunc = [&](int32_t val)
		{
			if (val < 0)
				quest_scheme = std::nullopt;
			else quest_scheme = schemes_list_quest.findText(val);
		});
	scheme_edit = DropDownList(data = schemes_list_no_none,
		fitParent = true, maxwidth = 250_px,
		onSelectFunc = [&](int32_t val)
		{
			edit_scheme = schemes_list_no_none.findText(val);
			refresh_edit_buttons();
		});
	generate_lists(); // AFTER the widgets are constructed, so it can modify the DropDownList selections directly
	
	std::shared_ptr<GUI::Window> window = Window(
		title = "Control Schemes",
		onClose = message::CANCEL,
		Column(
			Row(
				Column(
					Label(text = "Global Control Scheme:"),
					scheme_global,
					Label(text = "Quest Control Scheme:"),
					Label(text = quest_str, maxwidth = 300_px),
					scheme_quest
				),
				Columns<3>(
					Column(colSpan = 2,
						Label(text = "Edit Scheme:"),
						scheme_edit
					),
					Button(text = "New Scheme", fitParent = true, onPressFunc = [&]()
						{
							call_get_text("New Control Scheme", "What will the new scheme be named?", "", [&](string const& str, string& error)
								{
									if (!create_control_scheme(str))
									{
										error = fmt::format("A scheme named '{}' already exists!", str);
										return false;
									}
									edit_scheme = str;
									refresh_dlg();
									return true;
								});
						}),
					edit_buttons[0] = Button(fitParent = true, onPressFunc = [&]()
						{
							ControlBindingDialog(control_schemes[edit_scheme], edit_scheme).show();
						}),
					edit_buttons[1] = Button(text = "Delete Scheme", fitParent = true, onPressFunc = [&]()
						{
							if (!control_schemes.contains(edit_scheme))
								return; // shouldn't happen
							if (!alert_confirm("Are you sure?",
								fmt::format("This will remove control scheme '{}' entirely."
									" Quests set to use it will no longer have a control override set.", edit_scheme)))
							{
								return;
							}
							refresh_dlg();
							if (delete_control_scheme(edit_scheme))
								reload_schemes();
						}),
					edit_buttons[2] = Button(text = "Rename Scheme", fitParent = true, onPressFunc = [&]()
						{
							if (!control_schemes.contains(edit_scheme))
								return; // shouldn't happen, but this sanity prevents an inaccurate error message below
							call_get_text(fmt::format("Rename Control Scheme '{}'", edit_scheme), fmt::format("What will '{}' be renamed?", edit_scheme), edit_scheme, [&](string const& str, string& error)
								{
									if (edit_scheme == str)
										return true; // effectively canceling
									if (!rename_control_scheme(edit_scheme, str))
									{
										error = fmt::format("A scheme named '{}' already exists!", str);
										return false;
									}
									reload_schemes();
									edit_scheme = str;
									refresh_dlg();
									return true;
								});
						})
					
				)
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	button_word = ""; // force full refresh
	refresh_edit_buttons();
	return window;
}


void ControlSchemeDialog::generate_lists()
{
	schemes_list_quest.clear();
	
	int global_idx = 0;
	int quest_idx = -1;
	int edit_idx = 0;
	
	schemes_list_quest.add("(None)", -1);
	schemes_list_quest.add(DEFAULT_CONTROL_SCHEME_NAME, 0);
	if (quest_scheme && DEFAULT_CONTROL_SCHEME_NAME == *quest_scheme)
		quest_idx = 0;
	int idx = 1;
	for (auto& [key, _val] : control_schemes)
	{
		if (key == DEFAULT_CONTROL_SCHEME_NAME) continue;
		if (key == global_scheme)
			global_idx = idx;
		if (key == edit_scheme)
			edit_idx = idx;
		if (quest_scheme && key == *quest_scheme)
			quest_idx = idx;
		schemes_list_quest.add(key, idx);
		++idx;
	}
	schemes_list_no_none = schemes_list_quest.copy().filter([](GUI::ListItem& li){return li.value != -1;});
	
	if (scheme_global)
	{
		scheme_global->setSelectedValue(global_idx);
		global_scheme = schemes_list_no_none.findText(global_idx);
	}
	if (scheme_quest)
	{
		scheme_quest->setSelectedValue(quest_idx);
		if (quest_idx < 0)
			quest_scheme = std::nullopt;
		else quest_scheme = schemes_list_no_none.findText(quest_idx);
	}
	if (scheme_edit)
	{
		scheme_edit->setSelectedValue(edit_idx);
		edit_scheme = schemes_list_no_none.findText(edit_idx);
	}
}

void ControlSchemeDialog::refresh_edit_buttons()
{
	bool default_scheme = (edit_scheme == DEFAULT_CONTROL_SCHEME_NAME); // Default is non-editable
	string new_button_word = default_scheme ? "View" : "Edit";
	if (button_word != new_button_word)
	{
		button_word = new_button_word;
		edit_buttons[0]->setText(fmt::format("{} Scheme", button_word));
		edit_buttons[1]->setDisabled(default_scheme);
		edit_buttons[2]->setDisabled(default_scheme);
	}
}

bool ControlSchemeDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			ret = true;
			return true;
		case message::CANCEL:
			ret = false;
			return true;
	}
	return false;
}
