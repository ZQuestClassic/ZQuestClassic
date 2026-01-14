#include "save_menu_editor.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "base/misctypes.h"
#include "zc_list_data.h"

void call_editsavemenu_dialog(int index)
{
	if (unsigned(index-1) >= NUM_SAVE_MENUS)
		return;
	SaveMenuDialog(QMisc.save_menus[index-1]).show();
}

SaveMenuDialog::SaveMenuDialog(SaveMenu& dest):
	dest_ref(dest), local_ref(dest),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_music(GUI::ZCListData::music_names(true, false)),
	list_aligns(GUI::ZCListData::alignments()),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_genscr(GUI::ZCListData::generic_script())
{}

static size_t savemenu_tabs[2];
std::shared_ptr<GUI::Widget> SaveMenuDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TabPanel> option_tabs = TabPanel(ptr = &savemenu_tabs[1]);
	
	const auto btnsz = 32_px;
	for (size_t idx = 0; idx < local_ref.options.size(); ++idx)
	{
		option_tabs->add(TabRef(name = to_string(idx),
			Column(
				Row(
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_LEFT,
						disabled = (idx <= 0),
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							zc_swap(local_ref.options[idx], local_ref.options[idx-1]);
							--savemenu_tabs[1];
							refresh_dlg();
						}),
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_TRASH,
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							if (!alert_confirm("Are you sure?", fmt::format("This option '{}'"
								" will be erased.", local_ref.options[idx].text)))
								return;
							auto it = local_ref.options.begin();
							std::advance(it, idx);
							local_ref.options.erase(it);
							if (idx > 0)
								--savemenu_tabs[1];
							refresh_dlg();
						}),
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_RIGHT,
						disabled = (idx >= local_ref.options.size()-1),
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							zc_swap(local_ref.options[idx], local_ref.options[idx+1]);
							++savemenu_tabs[1];
							refresh_dlg();
						})
				),
				Columns<2>(
					Rows<3>(
						Label(text = "Text:", hAlign = 1.0),
						TextField(
							fitParent = true,
							type = GUI::TextField::type::TEXT,
							maxLength = SAVEMENU_STRING_LENGTH,
							text = local_ref.options[idx].text,
							onValChangedFunc = [&, idx](GUI::TextField::type, std::string_view text, int32_t)
							{
								local_ref.options[idx].text.assign(text);
							}
						),
						INFOBTN("The text for this option."),
						//
						Label(text = "Text Color:", hAlign = 1.0),
						ColorSel(val = local_ref.options[idx].color,
							fitParent = true,
							onValChangedFunc = [&, idx](byte val)
							{
								local_ref.options[idx].color = val;
							}),
						INFOBTN("The color of the text when not selected. Also used when flashing after choosing an option."),
						Label(text = "Selected Color:", hAlign = 1.0),
						ColorSel(val = local_ref.options[idx].picked_color,
							fitParent = true,
							onValChangedFunc = [&, idx](byte val)
							{
								local_ref.options[idx].picked_color = val;
							}),
						INFOBTN("The color of the text when selected. Also used when flashing after choosing an option.")
					),
					Rows<3>(
						Label(text = "Font:", hAlign = 1.0),
						DropDownList(data = list_font,
							fitParent = true, selectedValue = local_ref.options[idx].font,
							onSelectFunc = [&, idx](int32_t val)
							{
								local_ref.options[idx].font = val;
							}),
						INFOBTN("The font to draw this option in."),
						//
						Label(text = "Generic Script:", hAlign = 1.0),
						DropDownList(data = list_genscr,
							fitParent = true, selectedValue = local_ref.options[idx].gen_script,
							onSelectFunc = [&, idx](int32_t val)
							{
								local_ref.options[idx].gen_script = val;
							}),
						INFOBTN("A generic script to run in Frozen mode when the option is selected."
							"\nCurrently, no InitD[] can be supplied.")
					),
					Rows<2>(rowSpan = 2,
						Checkbox(_EX_RBOX, text = "Exit",
							checked = local_ref.options[idx].flags & SMENU_OPT_EXIT,
							onToggleFunc = [&, idx](bool state)
							{
								SETFLAG(local_ref.options[idx].flags, SMENU_OPT_EXIT, state);
							}),
						INFOBTN("When chosen, the game will be exited to the save menu. If 'Reload' is set, it takes priority."),
						Checkbox(_EX_RBOX, text = "Continue",
							checked = local_ref.options[idx].flags & SMENU_OPT_CONTINUE,
							onToggleFunc = [&, idx](bool state)
							{
								SETFLAG(local_ref.options[idx].flags, SMENU_OPT_CONTINUE, state);
							}),
						INFOBTN("When chosen, the game will be continued from the continue point. If 'Exit' or 'Reload' are set, they take priority."),
						Checkbox(_EX_RBOX, text = "Reload",
							checked = local_ref.options[idx].flags & SMENU_OPT_RELOAD,
							onToggleFunc = [&, idx](bool state)
							{
								SETFLAG(local_ref.options[idx].flags, SMENU_OPT_RELOAD, state);
							}),
						INFOBTN("When chosen, the game will be reloaded from the previous save."),
						Checkbox(_EX_RBOX, text = "Save",
							checked = local_ref.options[idx].flags & SMENU_OPT_SAVE,
							onToggleFunc = [&, idx](bool state)
							{
								SETFLAG(local_ref.options[idx].flags, SMENU_OPT_SAVE, state);
							}),
						INFOBTN("When chosen, the game will be saved."),
						Checkbox(_EX_RBOX, text = "Cancel",
							checked = local_ref.options[idx].flags & SMENU_OPT_CANCEL,
							onToggleFunc = [&, idx](bool state)
							{
								SETFLAG(local_ref.options[idx].flags, SMENU_OPT_CANCEL, state);
							}),
						INFOBTN("When chosen, the menu will be exited, leaving the player where they were before it opened."
							"\nHas no effect when used on a 'Game Over' menu.")
					)
				)
			)
		));
	}
	if (local_ref.options.size() < MAX_SAVEMENU_OPTIONS)
	{
		option_tabs->add(TabRef(name = "+",
			Button(text = "Add Option",
				onPressFunc = [&]()
				{
					local_ref.options.emplace_back();
					refresh_dlg();
				})
		));
	}
	return Window(
		title = fmt::format("Save Menu \"{}\"", local_ref.name),
		onClose = message::CANCEL,
		Column(
			TabPanel(ptr = &savemenu_tabs[0],
				TabRef(name = "Settings",
					Rows<2>(
						Row(colSpan = 2,
							Label(text = "Name:", hAlign = 1.0),
							TextField(
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								maxLength = SAVEMENU_NAME_LENGTH,
								text = local_ref.name,
								onValChangedFunc = [&](GUI::TextField::type, std::string_view text, int32_t)
								{
									local_ref.name.assign(text);
								}
							),
							INFOBTN("The name of this menu. Not used in-game, though scripts can read it.")
						),
						Rows<3>(
							Label(text = "BG Color:", hAlign = 1.0),
							ColorSel(val = local_ref.bg_color,
								fitParent = true,
								onValChangedFunc = [&](byte val)
								{
									local_ref.bg_color = val;
								}),
							INFOBTN("The background color of the menu."),
							//
							Label(text = "Option X:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 255, val = local_ref.opt_x,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.opt_x = val;
								}),
							INFOBTN("The X-position the text will be drawn at. How it is drawn is determined by the 'Text Align'."),
							Label(text = "Option Y:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 255, val = local_ref.opt_y,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.opt_y = val;
								}),
							INFOBTN("The Y-position the top option will be drawn at."),
							Label(text = "HSpace:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 255, val = local_ref.hspace,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.hspace = val;
								}),
							INFOBTN("The horizontal space, in pixels, between the options and the cursor."),
							Label(text = "VSpace:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 255, val = local_ref.vspace,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.vspace = val;
								}),
							INFOBTN("The vertical space, in pixels, between each option."),
							//
							Label(text = "Close Frames:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 65535, val = local_ref.close_frames,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.close_frames = val;
								}),
							INFOBTN("The time in frames after the user selects an option that the menu remains open."),
							Label(text = "Close Flash Rate:", hAlign = 1.0),
							TextField(
								type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
								low = 0, high = 255, val = local_ref.close_flash_rate,
								onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
								{
									local_ref.close_flash_rate = val;
								}),
							INFOBTN("The rate in frames that the selected option flashes between it's two colors after being selected.")
						),
						Column(
							Rows<6>(
								Label(text = "Cursor Tile:", hAlign = 1.0),
								SelTileSwatch(
									tile = local_ref.cursor_tile,
									cset = local_ref.cursor_cset,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_ref.cursor_tile = t;
										local_ref.cursor_cset = c;
									}),
								INFOBTN("The tile used for the cursor. Is drawn to the left of the selected option, vertically centered."),
								Label(text = "BG Tile:", hAlign = 1.0),
								SelTileSwatch(
									tile = local_ref.bg_tile,
									cset = local_ref.bg_cset,
									showvals = false,
									onSelectFunc = [&](int32_t t, int32_t c, int32_t, int32_t)
									{
										local_ref.bg_tile = t;
										local_ref.bg_cset = c;
									}),
								INFOBTN("The tile used for the background, drawn at the top-left corner."
									"\nIf 'Repeat Tile BG' is checked, will fill the whole screen by repeating the tiles."
									"\nIf 0, no tile background is drawn."),
								DummyWidget(colSpan = 3),
								Label(text = "W/H:", hAlign = 1.0),
								Row(
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										low = 1, high = 16, val = local_ref.bg_tw,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.bg_tw = val;
										}),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										low = 1, high = 16, val = local_ref.bg_th,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.bg_th = val;
										})
								),
								INFOBTN("The tile width and height of the 'BG Tile'.")
							),
							Rows<4>(
								Label(text = "Cursor SFX:", hAlign = 1.0),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_ref.cursor_sfx,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.cursor_sfx = val;
									}),
								INFOBTN("The SFX to play when the selected choice changes."),
								DummyWidget(),
								
								Label(text = "Choose SFX:", hAlign = 1.0),
								DropDownList(data = list_sfx,
									fitParent = true, selectedValue = local_ref.choose_sfx,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.choose_sfx = val;
									}),
								INFOBTN("The SFX to play when a choice is selected."),
								DummyWidget(),
								
								Label(text = "Text Align:", hAlign = 1.0),
								DropDownList(data = list_aligns,
									fitParent = true, selectedValue = local_ref.text_align,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.text_align = val;
									}),
								INFOBTN("The alignment of the text, relative to the other text."),
								DummyWidget(),
								
								Label(text = "Textbox Align:", hAlign = 1.0),
								DropDownList(data = list_aligns,
									fitParent = true, selectedValue = local_ref.textbox_align,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.textbox_align = val;
									}),
								INFOBTN("The alignment of the text, relative to 'Option X'."),
								DummyWidget(),
								
								Label(text = "Music:", hAlign = 1.0),
								midi_ddl = DropDownList(data = list_music,
									fitParent = true,
									selectedValue = local_ref.music,
									disabled = local_ref.flags & SMENU_DONT_KILL_MUSIC,
									onSelectFunc = [&](int32_t val)
									{
										local_ref.music = val;
									}),
								INFOBTN("The music to play during the save menu."),
								Button(text = "Edit Music",
									forceFitH = true,
									onPressFunc = [&]()
									{
										call_music_dialog(local_ref.music);
										list_music = GUI::ZCListData::music_names(true, false);
										refresh_dlg();
									})
							),
							Rows_Columns<2,3>(
								Checkbox(text = "'A' chooses",
									hAlign = 0.0,
									checked = local_ref.flags & SMENU_CONFIRM_A,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.flags, SMENU_CONFIRM_A, state);
									}),
								INFOBTN("If checked, the 'A' button can be used to select a menu choice. (The 'Start' button can always be used)"),
								Checkbox(text = "'B' cancels",
									hAlign = 0.0,
									checked = local_ref.flags & SMENU_CANCEL_B,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.flags, SMENU_CANCEL_B, state);
									}),
								INFOBTN("If checked, the 'B' button can be used to cancel out of the menu. (This does not work when being used as a 'Game Over' menu)"),
								Checkbox(text = "Repeat Tile BG",
									hAlign = 0.0,
									checked = local_ref.flags & SMENU_REPEAT_BG,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.flags, SMENU_REPEAT_BG, state);
									}),
								INFOBTN("If checked, the 'Tile BG' will be repeated to fill the whole screen."),
								Checkbox(text = "Don't Kill Music",
									hAlign = 0.0,
									checked = local_ref.flags & SMENU_DONT_KILL_MUSIC,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.flags, SMENU_DONT_KILL_MUSIC, state);
										midi_ddl->setDisabled(state);
									}),
								INFOBTN("If checked, the music will not be killed by the menu. This means that the set 'Music' will be ignored as well."),
								Checkbox(text = "Don't Kill SFX",
									hAlign = 0.0,
									checked = local_ref.flags & SMENU_DONT_KILL_SFX,
									onToggleFunc = [&](bool state)
									{
										SETFLAG(local_ref.flags, SMENU_DONT_KILL_SFX, state);
									}),
								INFOBTN("If checked, SFX will not be killed by the menu.")
								
							)
						)
					)
				),
				TabRef(name = "Options",
					option_tabs
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
					text = "Preview",
					minwidth = 90_px,
					disabled = local_ref.options.empty(),
					onPressFunc = [&]()
					{
						local_ref.run();
					}),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
}

bool SaveMenuDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			dest_ref = local_ref;
			mark_save_dirty();
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}
