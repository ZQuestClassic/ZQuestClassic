#include "save_menu_editor.h"
#include "dialog/externs.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include <dialog/info.h>
#include "zq/zquest.h"
#include "core/misctypes.h"
#include "zc_list_data.h"
#include "script_data_editor.h"

extern script_data *genericscripts[NUMSCRIPTSGENERIC];

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
	list_genscr(GUI::ZCListData::generic_script()),
	list_shadow_types(GUI::ZCListData::shadow_types(true))
{}

static size_t savemenu_tabs[4] = {0};
std::shared_ptr<GUI::Widget> SaveMenuDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TabPanel> option_tabs = TabPanel(ptr = &savemenu_tabs[1]);
	std::shared_ptr<GUI::TabPanel> misc_text_tabs = TabPanel(ptr = &savemenu_tabs[3], topPadding = DEFAULT_PADDING + 4_px);
	
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
				Row(
					Column(
						Rows<3>(hAlign = 1.0,
							Label(text = "Text:", hAlign = 1.0),
							TextField(
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								maxLength = SAVEMENU_OPTION_TEXT_LENGTH,
								text = local_ref.options[idx].text,
								onValChangedFunc = [&, idx](GUI::TextField::type, std::string_view text, int32_t)
								{
									local_ref.options[idx].text.assign(text);
								}
							),
							INFOBTN("The text for this option."),
							//
							Label(text = "Font:", hAlign = 1.0),
							DropDownList(data = list_font,
								fitParent = true, selectedValue = local_ref.options[idx].font,
								onSelectFunc = [&, idx](int32_t val)
								{
									local_ref.options[idx].font = val;
								}),
							INFOBTN("The font to draw this option in."),
							//
							Label(text = "Shadow Type:", hAlign = 1.0),
							DropDownList(data = list_shadow_types,
								fitParent = true, selectedValue = local_ref.options[idx].shadow_type,
								onSelectFunc = [&, idx](int32_t val)
								{
									local_ref.options[idx].shadow_type = val;
								}),
							INFOBTN("The shadow style to draw the options with.")
						),
						Rows<4>(hAlign = 1.0,
							_d,
							Label(text = "Normal"),
							Label(text = "Selected"),
							INFOBTN("'Selected' values are used for the option the cursor is currently hovering over."
								" While flashing before closing, the chosen option blinks between"
								" 'Selected' and 'Non-Selected' values."),
							//
							Label(text = "Text Color:", hAlign = 1.0),
							ColorSel(val = local_ref.options[idx].color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.options[idx].color = val;
								}),
							ColorSel(val = local_ref.options[idx].picked_color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.options[idx].picked_color = val;
								}),
							INFOBTN("The color of the text."),
							//
							Label(text = "Shadow Color:", hAlign = 1.0),
							ColorSel(val = local_ref.options[idx].shadow_color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.options[idx].shadow_color = val;
								}),
							ColorSel(val = local_ref.options[idx].picked_shadow_color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.options[idx].picked_shadow_color = val;
								}),
							INFOBTN("The color of the shadow/outline.")
						)
					),
					Frame(title = "Actions", info = "What to do when this option is chosen",
						Rows<2>(rowSpan = 2,
							Checkbox(_EX_RBOX, text = "Exit",
								checked = local_ref.options[idx].flags & SMENU_OPT_EXIT,
								onToggleFunc = [&, idx](bool state)
								{
									SETFLAG(local_ref.options[idx].flags, SMENU_OPT_EXIT, state);
								}),
							INFOBTN("The game will be exited to the save menu. If 'Reload' is set, it takes priority."),
							Checkbox(_EX_RBOX, text = "Continue",
								checked = local_ref.options[idx].flags & SMENU_OPT_CONTINUE,
								onToggleFunc = [&, idx](bool state)
								{
									SETFLAG(local_ref.options[idx].flags, SMENU_OPT_CONTINUE, state);
								}),
							INFOBTN("The game will be continued from the continue point. If 'Exit' or 'Reload' are set, they take priority."),
							Checkbox(_EX_RBOX, text = "Reload",
								checked = local_ref.options[idx].flags & SMENU_OPT_RELOAD,
								onToggleFunc = [&, idx](bool state)
								{
									SETFLAG(local_ref.options[idx].flags, SMENU_OPT_RELOAD, state);
								}),
							INFOBTN("The game will be reloaded from the previous save."),
							Checkbox(_EX_RBOX, text = "Save",
								checked = local_ref.options[idx].flags & SMENU_OPT_SAVE,
								onToggleFunc = [&, idx](bool state)
								{
									SETFLAG(local_ref.options[idx].flags, SMENU_OPT_SAVE, state);
								}),
							INFOBTN("The game will be saved."),
							Checkbox(_EX_RBOX, text = "Cancel",
								checked = local_ref.options[idx].flags & SMENU_OPT_CANCEL,
								onToggleFunc = [&, idx](bool state)
								{
									SETFLAG(local_ref.options[idx].flags, SMENU_OPT_CANCEL, state);
								}),
							INFOBTN("The menu will be exited, leaving the player where they were before it opened."
								"\nHas no effect when used on a 'Game Over' menu."),
							//
							Button(hAlign = 1.0,
								text = "Frozen Script",
								height = 2_em,
								onPressFunc = [&, idx]()
								{
									ScriptDataDialog("Save Menu Choice Generic Frozen Script Setup",
										local_ref.options[idx].gen_scrconfig, list_genscr, genericscripts).show();
								}
							),
							INFOBTN("This generic script will run in Frozen mode.")
						)
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
	
	for (size_t idx = 0; idx < local_ref.misc_texts.size(); ++idx)
	{
		misc_text_tabs->add(TabRef(name = to_string(idx),
			Column(
				Row(
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_LEFT,
						disabled = (idx <= 0),
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							zc_swap(local_ref.misc_texts[idx], local_ref.misc_texts[idx-1]);
							--savemenu_tabs[3];
							refresh_dlg();
						}),
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_TRASH,
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							if (!alert_confirm("Are you sure?", fmt::format("This misc text '{}'"
								" will be erased.", local_ref.misc_texts[idx].text)))
								return;
							auto it = local_ref.misc_texts.begin();
							std::advance(it, idx);
							local_ref.misc_texts.erase(it);
							if (idx > 0)
								--savemenu_tabs[3];
							refresh_dlg();
						}),
					Button(type = GUI::Button::type::ICON,
						icon = BTNICON_ARROW_RIGHT,
						disabled = (idx >= local_ref.misc_texts.size()-1),
						width = btnsz, height = btnsz,
						onPressFunc = [&, idx]()
						{
							zc_swap(local_ref.misc_texts[idx], local_ref.misc_texts[idx+1]);
							++savemenu_tabs[3];
							refresh_dlg();
						})
				),
				Row(
					Column(
						Rows<3>(hAlign = 1.0,
							Label(text = "Text:", hAlign = 1.0),
							TextField(
								fitParent = true,
								type = GUI::TextField::type::TEXT,
								maxLength = SAVEMENU_MISC_TEXT_LENGTH,
								text = local_ref.misc_texts[idx].text,
								onValChangedFunc = [&, idx](GUI::TextField::type, std::string_view text, int32_t)
								{
									local_ref.misc_texts[idx].text.assign(text);
								}
							),
							INFOBTN("The text for this misc text."),
							//
							Label(text = "Font:", hAlign = 1.0),
							DropDownList(data = list_font,
								fitParent = true, selectedValue = local_ref.misc_texts[idx].font,
								onSelectFunc = [&, idx](int32_t val)
								{
									local_ref.misc_texts[idx].font = val;
								}),
							INFOBTN("The font to draw this misc text in."),
							//
							Label(text = "Text Align:", hAlign = 1.0),
							DropDownList(data = list_aligns,
								fitParent = true, selectedValue = local_ref.misc_texts[idx].text_align,
								onSelectFunc = [&, idx](int32_t val)
								{
									local_ref.misc_texts[idx].text_align = val;
								}),
							INFOBTN("The alignment of the text, relative to the specified Text X."),
							//
							Label(text = "Shadow Type:", hAlign = 1.0),
							DropDownList(data = list_shadow_types,
								fitParent = true, selectedValue = local_ref.misc_texts[idx].shadow_type,
								onSelectFunc = [&, idx](int32_t val)
								{
									local_ref.misc_texts[idx].shadow_type = val;
								}),
							INFOBTN("The shadow style to draw the misc text with.")
						),
						Rows<3>(hAlign = 1.0,
							Label(text = "Text Color:", hAlign = 1.0),
							ColorSel(val = local_ref.misc_texts[idx].color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.misc_texts[idx].color = val;
								}),
							INFOBTN("The color of the text."),
							//
							Label(text = "Shadow Color:", hAlign = 1.0),
							ColorSel(val = local_ref.misc_texts[idx].shadow_color,
								width = 120_px,
								onValChangedFunc = [&, idx](byte val)
								{
									local_ref.misc_texts[idx].shadow_color = val;
								}),
							INFOBTN("The color of the shadow/outline.")
						)
					),
					Rows<3>(
						Label(text = "Text X:", hAlign = 1.0),
						TextField(
							type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
							bounds = {0, 255}, val = local_ref.misc_texts[idx].x,
							onValChangedFunc = [&, idx](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.misc_texts[idx].x = val;
							}),
						INFOBTN("The X-position the text will be drawn at. How it is drawn is determined by the 'Text Align'."),
						Label(text = "Text Y:", hAlign = 1.0),
						TextField(
							type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
							bounds = {0, 255}, val = local_ref.misc_texts[idx].y,
							onValChangedFunc = [&, idx](GUI::TextField::type,std::string_view,int32_t val)
							{
								local_ref.misc_texts[idx].y = val;
							}),
						INFOBTN("The Y-position the text will be drawn at. Corresponds to the top of the text.")
					)
				)
			)
		));
	}
	if (local_ref.misc_texts.size() < MAX_SAVEMENU_MISC_TEXTS)
	{
		misc_text_tabs->add(TabRef(name = "+",
			Button(text = "Add Misc Text",
				onPressFunc = [&]()
				{
					local_ref.misc_texts.emplace_back();
					refresh_dlg();
				})
		));
	}
	
	return Window(
		title = fmt::format("Save Menu \"{}\"", local_ref.name),
		onClose = message::CANCEL,
		Column(
			Row(
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
			TabPanel(ptr = &savemenu_tabs[0],
				TabRef(name = "Settings",
					TabPanel(ptr = &savemenu_tabs[2],
						TabRef(name = "1",
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
											bounds = {1, 16}, val = local_ref.bg_tw,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.bg_tw = val;
											}),
										TextField(
											type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
											bounds = {1, 16}, val = local_ref.bg_th,
											onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
											{
												local_ref.bg_th = val;
											})
									),
									INFOBTN("The tile width and height of the 'BG Tile'.")
								),
								Rows_Columns<3, 5>(
									Label(text = "Option X:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 255}, val = local_ref.opt_x,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.opt_x = val;
										}),
									INFOBTN("The X-position the text will be drawn at. How it is drawn is determined by the 'Text Align'."),
									Label(text = "Option Y:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 255}, val = local_ref.opt_y,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.opt_y = val;
										}),
									INFOBTN("The Y-position the top option will be drawn at."),
									
									Label(text = "HSpace:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 255}, val = local_ref.hspace,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.hspace = val;
										}),
									INFOBTN("The horizontal space, in pixels, between the options and the cursor."),
									Label(text = "VSpace:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 255}, val = local_ref.vspace,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.vspace = val;
										}),
									INFOBTN("The vertical space, in pixels, between each option."),
									
									Label(text = "BG Color:", hAlign = 1.0),
									ColorSel(val = local_ref.bg_color,
										fitParent = true,
										onValChangedFunc = [&](byte val)
										{
											local_ref.bg_color = val;
										}),
									INFOBTN("The background color of the menu."),
									//
									Label(text = "Selected Offset X:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {-32768, 32767}, val = local_ref.opt_sel_x_offset,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.opt_sel_x_offset = val;
										}),
									INFOBTN("An offset to the X-position of the currently selected option and cursor."
										"\nCan be used to ex. indent / shift the selected option."),
									Label(text = "Selected Offset Y:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {-32768, 32767}, val = local_ref.opt_sel_y_offset,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.opt_sel_y_offset = val;
										}),
									INFOBTN("An offset to the Y-position of the currently selected option and cursor."
										"\nCan be used to ex. indent / shift the selected option."),
									
									Label(text = "Close Frames:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 65535}, val = local_ref.close_frames,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.close_frames = val;
										}),
									INFOBTN("The time in frames after the user selects an option that the menu remains open."),
									Label(text = "Close Flash Rate:", hAlign = 1.0),
									TextField(
										type = GUI::TextField::type::INT_DECIMAL, fitParent = true,
										bounds = {0, 255}, val = local_ref.close_flash_rate,
										onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val)
										{
											local_ref.close_flash_rate = val;
										}),
									INFOBTN("The rate in frames that the selected option flashes between it's two colors after being selected.")
									//
									
								)
							)
						),
						TabRef(name = "2",
							Row(
								Rows<2>(
									INFOBTN("If checked, the 'A' button can be used to select a menu choice. (The 'Start' button can always be used)"),
									Checkbox(text = "'A' chooses",
										hAlign = 0.0,
										checked = local_ref.flags & SMENU_CONFIRM_A,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.flags, SMENU_CONFIRM_A, state);
										}),
									INFOBTN("If checked, the 'B' button can be used to cancel out of the menu. (This does not work when being used as a 'Game Over' menu)"),
									Checkbox(text = "'B' cancels",
										hAlign = 0.0,
										checked = local_ref.flags & SMENU_CANCEL_B,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.flags, SMENU_CANCEL_B, state);
										}),
									INFOBTN("If checked, the 'Tile BG' will be repeated to fill the whole screen."),
									Checkbox(text = "Repeat Tile BG",
										hAlign = 0.0,
										checked = local_ref.flags & SMENU_REPEAT_BG,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.flags, SMENU_REPEAT_BG, state);
										}),
									INFOBTN("If checked, the music will not be killed by the menu. This means that the set 'Music' will be ignored as well."),
									Checkbox(text = "Don't Kill Music",
										hAlign = 0.0,
										checked = local_ref.flags & SMENU_DONT_KILL_MUSIC,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.flags, SMENU_DONT_KILL_MUSIC, state);
											midi_ddl->setDisabled(state);
										}),
									INFOBTN("If checked, SFX will not be killed by the menu."),
									Checkbox(text = "Don't Kill SFX",
										hAlign = 0.0,
										checked = local_ref.flags & SMENU_DONT_KILL_SFX,
										onToggleFunc = [&](bool state)
										{
											SETFLAG(local_ref.flags, SMENU_DONT_KILL_SFX, state);
										})
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
								)
							)
						)
					)
				),
				TabRef(name = "Options",
					option_tabs
				),
				TabRef(name = "Misc Texts",
					Frame(info = "Extra text to draw on the menu. This could be flavor text, a 'menu title', etc.",
						fitParent = true,
						misc_text_tabs
					)
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
