#include "script_data_editor.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include "info.h"

extern int32_t numericalFlags;

static const string export_variable_url = "https://docs.zquestclassic.com/zscript/lang/scripts#exported-variables";

ScriptDataDialog::ScriptDataDialog(string const& title, script_config& cfg,
	GUI::ListData const& scripts, script_data const* const* scrdatas)
	: dest_ref(cfg), local_ref(cfg), init_args(), title_str(title),
	list_scriptchoices(scripts), scrdatas(scrdatas), specified_args(),
	list_engine_item(GUI::ZCListData::items(true)),
	list_engine_enemy(GUI::ZCListData::enemies(true)),
	list_engine_counter(GUI::ZCListData::counters(true)),
	list_engine_sprite(GUI::ZCListData::miscsprites(false, false, true)),
	list_engine_sfx(GUI::ZCListData::sfxnames(true)),
	list_engine_midi(GUI::ZCListData::midinames(true)),
	list_engine_music(GUI::ZCListData::music_names(true)),
	list_engine_save_menu(GUI::ZCListData::savemenus(true)),
	list_engine_message_string(GUI::ZCListData::strings(true, false, true)),
	list_engine_weapon_type(GUI::ZCListData::weaptypes(true)),
	list_engine_lweapon_type(GUI::ZCListData::lweaptypes(true)),
	list_engine_eweapon_type(GUI::ZCListData::eweaptypes(true)),
	list_engine_dropset(GUI::ZCListData::dropsets(true, true)),
	list_engine_font_id(GUI::ZCListData::fonts(false, true)),
	list_engine_bottle_type(GUI::ZCListData::bottletype(true)),
	list_engine_combo_type(GUI::ZCListData::combotype(true)),
	list_engine_combo_flag(GUI::ZCListData::mapflag(numericalFlags, true))
{
	if (cfg.script)
		init_args[cfg.script] = cfg.inst_init;
}

static size_t tab_pos = 0;
static int32_t scroll_pos = 0;
std::shared_ptr<GUI::Widget> ScriptDataDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	script_data const* scrdata = nullptr;
	zasm_meta const* meta = nullptr;
	if (local_ref.script)
	{
		scrdata = scrdatas[local_ref.script];
		meta = &scrdata->meta;
	}
	
	auto initd_grid = Rows<3>(spacing = 2_px, topPadding = 10_px);
	auto instvar_grid = Rows<4>(spacing = 2_px, hPadding = 0_px);
	
	auto main_column = Column(spacing = 0_px);
	if (!list_scriptchoices.empty())
		main_column->add(Rows<2>(
			// Changing script refreshes the entire dialog
			// This changes the index used for `init_args`, leaving the old args stored until the dialog
			//   closes in case you change back. Only the active args are saved when closing.
			Label(text = "Script:"),
			DropDownList(
				fitParent = true, minwidth = 200_px,
				data = list_scriptchoices,
				selectedValue = local_ref.script,
				onSelectFunc = [&](int32_t val)
				{
					local_ref.script = val;
					refresh_dlg();
				}
			)
		));
	if (!list_scriptchoices.empty() || (meta && !(meta->script_info.empty() && meta->script_setup.empty())))
		main_column->add(Row(
			Button(text = "Info",
				height = 2_em,
				disabled = !meta || meta->script_info.empty(),
				onPressFunc = [&, meta]()
				{
					InfoDialog(fmt::format("'{}' Script Info", meta->script_name), meta->script_info).show();
				}
			),
			Button(text = "Setup Instructions",
				height = 2_em,
				disabled = !meta || meta->script_setup.empty(),
				onPressFunc = [&, meta]()
				{
					InfoDialog(fmt::format("'{}' Setup Instructions", meta->script_name), meta->script_setup).show();
				}
			)
		));
	
	auto tab_initd = TabRef(name = "InitD[]",
		Frame(info = "These values represent the up to 8 parameters of the script's `Run` function."
			" Modifying them will configure script-specific behaviors."
			"\n\nUsing '@ExportInitD0()' through '@ExportInitD7()' in scripts allows specifying custom"
			" help text for the '?' buttons of each option, as well as customizing the name and default"
			" input state of the field.",
			fitParent = true,
			initd_grid
		)
	);
	auto tab_exports = TabRef(name = "Exports",
		Frame(info = "These variables are specific to the currently-selected script,"
			" and configure script-specific behaviors."
			"\nThe value will be the 'script default' value, unless you modify it here."
			" Values that have been modified will have a reset button to their right to"
			" set them back to default. (Modified values will not change even if the script"
			" is edited to change a value)"
			+ INFO_URL("Docs: How To Export Variables", export_variable_url),
			fitParent = true,
			ScrollingPane(
				ptr_y = &scroll_pos,
				fitParent = true,
				padding = 0_px, topPadding = 10_px,
				instvar_grid
			)
		)
	);
	
	main_column->add(TabPanel(ptr = &tab_pos,
		tab_exports,
		tab_initd
	));
	
	window = Window(
		title = title_str,
		onClose = message::CANCEL,
		Column(
			main_column,
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_px,
					focused = true,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	
	for (int ind = 0; ind < 8; ++ind) // InitD[]
	{
		string label = fmt::format("InitD[{}]", ind);
		byte swp = nswapDEC;
		string help;
		if (meta)
		{
			if (!meta->initd_label[ind].empty())
				label = meta->initd_label[ind];
			if (!meta->initd_help[ind].empty())
				help = meta->initd_help[ind];
			if (unsigned(meta->initd_type[ind]) < nswapMAX)
				swp = meta->initd_type[ind];
		}
		
		initd_grid->add(Label(
			text = label, hAlign = 1.0
		));
		initd_grid->add(help.empty() ? DINFOBTN() : INFOBTN(help));
		initd_grid->add(TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT2,
			swap_type = swp,
			val = local_ref.run_args[ind],
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_ref.run_args[ind] = val;
			}));
	}
	
	std::set<word> added_idxs;
	auto& args = init_args[local_ref.script];
	
	export_widgets.clear();
	
	if (scrdata)
	{
		if (!specified_args.contains(local_ref.script))
		{
			auto& specified = specified_args[local_ref.script];
			auto const& exports = scrdata->script_d_exports.inner();
			for (auto const& [idx, val] : args)
				specified.set(idx, true);
			for (auto const& [idx, val] : scrdata->script_d_init.inner())
			{
				if (exports.contains(idx) && !specified.get(idx))
					args[idx] = val;
			}
		}
		if (!scrdata->script_d_exports.inner_empty())
		{
			auto on_reset = [&, scrdata](ExportWidgetData& widget_data)
			{
				auto idx = widget_data.index;
				specified_args[local_ref.script].set(idx, false);
				args[idx] = scrdata->script_d_init.get(idx);
				if (widget_data.set_widg_val)
					widget_data.set_widg_val(args[idx]);
				widget_data.reset_button->setVisible(false);
			};
			for (auto const& [idx, expdata] : scrdata->script_d_exports.inner())
			{
				added_idxs.insert(idx);
				
				ExportWidgetData const* prev_widget_data = nullptr;
				if (idx > 0)
				{
					if (export_widgets.contains(idx-1))
						prev_widget_data = &export_widgets[idx-1];
				}
				auto& widget_data = export_widgets[idx];
				widget_data.index = idx;
				widget_data.expdata = &expdata;
				auto info_btn = INFOBTN_T_EX(expdata.name, expdata.helptext,
					disabled = expdata.helptext.empty(),
					padding = 0_px,
					width = 24_px, height = 24_px
				);
				auto reset_btn = Button(
					padding = 0_px, rightPadding = 12_px,
					width = 24_px, height = 24_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_LOOP_ARROW,
					visible = specified_args[local_ref.script].get(idx),
					onPressFunc = [&, idx, on_reset]()
					{
						on_reset(export_widgets[idx]);
					}
				);
				widget_data.reset_button = reset_btn;
				
				auto set_arg = [&, idx, reset_btn](int val)
				{
					args[idx] = val;
					reset_btn->setVisible(true);
					specified_args[local_ref.script].set(idx, true);
				};
				widget_data.set_arg = set_arg;
				instvar_grid->add(Label(text = expdata.name, hAlign = 1.0));
				instvar_grid->add(info_btn);
				byte swp = nswapDEC;
				if (unsigned(expdata.btn_type) < nswapMAX)
					swp = expdata.btn_type;
				if (expdata.engine_type != special_engine_export::none)
				{
					switch (expdata.engine_type)
					{
						case special_engine_export::tile:
						{
							auto tswatch = SelTileSwatch(
								tile = args[idx],
								showvals = false,
								hAlign = 0.0,
								onSelectFunc = [&, set_arg](int32_t t, int32_t,int32_t,int32_t)
								{
									set_arg(t);
								}
							);
							widget_data.main_widget = tswatch;
							widget_data.set_widg_val = [&, tswatch](int val)
								{
									tswatch->setTile(val);
								};
							instvar_grid->add(tswatch);
							break;
						}
						case special_engine_export::tile_cset:
						{
							if (!prev_widget_data || prev_widget_data->expdata->engine_type != special_engine_export::tile)
							{
								auto lbl = Label(text = "!!ERROR!!");
								widget_data.main_widget = lbl;
								widget_data.set_widg_val = [&](int){};
								instvar_grid->add(lbl);
							}
							else
							{
								widget_data.main_widget = prev_widget_data->main_widget;
								GUI::SelTileSwatch* tswatch = (GUI::SelTileSwatch*)(prev_widget_data->main_widget.get());
								widget_data.set_widg_val = [&, tswatch](int val)
								{
									tswatch->setCSet(val);
								};
								tswatch->setCSet(args[idx]);
								tswatch->setRowSpan(2);
								instvar_grid->add(_d); // setRowSpan doesn't update the grid, so add a dummy to eat the space
								auto set_tile = prev_widget_data->set_arg;
								auto set_cset = set_arg;
								tswatch->setOnSelectFunc([&, set_tile, set_cset](int t, int cs, int, int)
								{
									set_tile(t);
									set_cset(cs);
								});
							}
							break;
						}
						
						case special_engine_export::combo:
						{
							auto cswatch = SelComboSwatch(
								combo = args[idx],
								showvals = false,
								hAlign = 0.0,
								onSelectFunc = [&, set_arg](int32_t cmb, int32_t)
								{
									set_arg(cmb);
								}
							);
							widget_data.main_widget = cswatch;
							widget_data.set_widg_val = [&, cswatch](int val)
								{
									cswatch->setCombo(val);
								};
							instvar_grid->add(cswatch);
							break;
						}
						case special_engine_export::combo_cset:
						{
							if (!prev_widget_data || prev_widget_data->expdata->engine_type != special_engine_export::combo)
							{
								auto lbl = Label(text = "!!ERROR!!");
								widget_data.main_widget = lbl;
								widget_data.set_widg_val = [&](int){};
								instvar_grid->add(lbl);
							}
							else
							{
								widget_data.main_widget = prev_widget_data->main_widget;
								GUI::SelComboSwatch* cswatch = (GUI::SelComboSwatch*)(prev_widget_data->main_widget.get());
								widget_data.set_widg_val = [&, cswatch](int val)
								{
									cswatch->setCSet(val);
								};
								cswatch->setCSet(args[idx]);
								cswatch->setRowSpan(2);
								instvar_grid->add(_d); // setRowSpan doesn't update the grid, so add a dummy to eat the space
								auto set_combo = prev_widget_data->set_arg;
								auto set_cset = set_arg;
								cswatch->setOnSelectFunc([&, set_combo, set_cset](int c, int cs)
								{
									set_combo(c);
									set_cset(cs);
								});
							}
							break;
						}
						
						case special_engine_export::color:
						{
							auto csel = ColorSel(
								fitParent = true, width = 100_px,
								hAlign = 0.0,
								val = args[idx],
								onValChangedFunc = [&, set_arg](byte val)
								{
									set_arg(val);
								}
							);
							widget_data.main_widget = csel;
							widget_data.set_widg_val = [&, csel](int val)
								{
									csel->setVal(val);
								};
							instvar_grid->add(csel);
							break;
						}
						default: // Group dropdown types together for reduced duplication
						{
							GUI::ListData* ld = nullptr;
							switch (expdata.engine_type)
							{
								case special_engine_export::item:
									ld = &list_engine_item;
									break;
								case special_engine_export::enemy:
									ld = &list_engine_enemy;
									break;
								case special_engine_export::counter:
									ld = &list_engine_counter;
									break;
								case special_engine_export::sprite:
									ld = &list_engine_sprite;
									break;
								case special_engine_export::sfx:
									ld = &list_engine_sfx;
									break;
								case special_engine_export::midi:
									ld = &list_engine_midi;
									break;
								case special_engine_export::music:
									ld = &list_engine_music;
									break;
								case special_engine_export::save_menu:
									ld = &list_engine_save_menu;
									break;
								case special_engine_export::message_string:
									ld = &list_engine_message_string;
									break;
								case special_engine_export::weapon_type:
									ld = &list_engine_weapon_type;
									break;
								case special_engine_export::lweapon_type:
									ld = &list_engine_lweapon_type;
									break;
								case special_engine_export::eweapon_type:
									ld = &list_engine_eweapon_type;
									break;
								case special_engine_export::dropset:
									ld = &list_engine_dropset;
									break;
								case special_engine_export::font_id:
									ld = &list_engine_font_id;
									break;
								case special_engine_export::bottle_type:
									ld = &list_engine_bottle_type;
									break;
								case special_engine_export::combo_type:
									ld = &list_engine_combo_type;
									break;
								case special_engine_export::combo_flag:
									ld = &list_engine_combo_flag;
									break;
							}
							if (!ld)
							{
								instvar_grid->add(Label(text = "!!ERROR!!"));
								NOTREACHED();
								break;
							}
							auto ddl = DropDownList(
								fitParent = true, width = 200_px,
								data = *ld,
								selectedValue = args[idx],
								onSelectFunc = [&, set_arg](int32_t val)
								{
									set_arg(val);
								}
							);
							widget_data.main_widget = ddl;
							widget_data.set_widg_val = [&, ddl](int val)
								{
									ddl->setSelectedValue(val);
								};
							instvar_grid->add(ddl);
							break;
						}
					}
				}
				else if (expdata.export_custom_type != var_custom_export_type::none)
				{
					bool is_long = false;
					switch (expdata.export_custom_type)
					{
						case var_custom_export_type::custom_dropdown:
						{
							auto& ld = widget_data.list;
							map<string, zfix> vals;
							
							// Build out the custom list data
							bool decimal = false;
							size_t digits = 1;
							for (auto& [val, str] : expdata.custom_export_names)
							{
								if (val % 1_zf)
								{
									decimal = true;
									break; // decimal negates the need for any of the other info
								}
								auto length = val.str_trim().size();
								if (digits < length)
									digits = length;
							}
							
							for (auto& [val, str] : expdata.custom_export_names)
							{
								string key;
								if (decimal)
									key = fmt::format("{} ({})", str, val);
								else
									key = fmt::format("{} ({:0{}})", str, val.getInt(), digits);
								vals[key] = val;
							}
							
							for(auto& [name, val] : vals)
								ld.add(name, val);
							
							auto ddl = DropDownList(
								fitParent = true, width = 200_px,
								data = ld,
								selectedValue = args[idx],
								onSelectFunc = [&, set_arg](int32_t val)
								{
									set_arg(val);
								}
							);
							widget_data.main_widget = ddl;
							widget_data.set_widg_val = [&, ddl](int val)
								{
									ddl->setSelectedValue(val);
								};
							instvar_grid->add(ddl);
							break;
						}
						
						case var_custom_export_type::custom_long_bitflags:
							is_long = true;
						[[fallthrough]];
						case var_custom_export_type::custom_bitflags:
						{
							auto& clist = widget_data.clist_info;
							
							int numbits = is_long ? 32 : 18;
							int queued_blanks = 0;
							for (int q = 0; q < numbits; ++q)
							{
								zfix v = is_long ? zslongToFix(1 << q) : zfix(1 << q);
								
								if (expdata.custom_export_names.contains(v))
								{
									if (queued_blanks)
									{
										clist.insert(clist.end(), queued_blanks, {CheckListInfo::DISABLED, "--"});
										queued_blanks = 0;
									}
									
									clist.emplace_back(expdata.custom_export_names.at(v));
								}
								else
									++queued_blanks;
							}
							if (clist.empty())
								clist.emplace_back(CheckListInfo::DISABLED, "No Flags Found");
							
							auto btn = Button(text = "P",
								hAlign = 0.0, padding = 0_px,
								width = 24_px, height = 24_px,
								onPressFunc = [&, idx, set_arg]()
								{
									int flags = args[idx];
									if (!call_checklist_dialog(fmt::format("Select '{}'", expdata.name),
										export_widgets[idx].clist_info, flags))
										return;
									set_arg(flags);
								}
							);
							widget_data.main_widget = btn;
							instvar_grid->add(btn);
							break;
						}
					}
				}
				else if (expdata.min == 0_zf && expdata.max == 0.0001_zf && swp == nswapBOOL)
				{
					// Checkbox input
					auto cb = Checkbox(
						fitParent = true, checked = args[idx],
						onToggleFunc = [&, set_arg](bool state)
						{
							set_arg(state ? 10000 : 0);
						});
					widget_data.main_widget = cb;
					widget_data.set_widg_val = [&, cb](int val)
						{
							cb->setChecked(val);
						};
					instvar_grid->add(cb);
				}
				else
				{
					auto tf = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_ZSINT,
						swap_type = swp, val = args[idx],
						onValChangedFunc = [&, set_arg](GUI::TextField::type,std::string_view,int32_t val)
						{
							set_arg(val);
						});
					if (expdata.min < expdata.max && !(expdata.min == -214748.3648_zf && expdata.max == 214748.3647_zf))
					{
						tf->setBounds({expdata.min.getZLong(), expdata.max.getZLong()});
					}
					widget_data.main_widget = tf;
					widget_data.set_widg_val = [&, tf](int val)
						{
							tf->setVal(val);
						};
					instvar_grid->add(tf);
				}
				instvar_grid->add(reset_btn);
				DCHECK((instvar_grid->maxChildIndex()+1) % 4 == 0);
			}
		}
	}
	
	std::set<word> bad_idxs;
	for (auto const& [idx, val] : args)
	{
		if (!added_idxs.contains(idx))
			bad_idxs.insert(idx);
	}
	if (added_idxs.empty())
		instvar_grid->add(Label(text = "No exported Instance Variables.", colSpan = 4));
	if (!bad_idxs.empty())
	{
		bool singular = bad_idxs.size() == 1;
		instvar_grid->add(Label(colSpan = 4,
			maxwidth = 250_px,
			text = fmt::format("{} instance value{} found set in the quest file that"
				" do{} not match any exported instance variable. This data may be leftover"
				" from a previous script, and is likely useless in its current state.",
				bad_idxs.size(), singular ? " was" : "s were", singular ? "es" : "")
		));
		instvar_grid->add(Button(colSpan = 4,
			text = "Clear Leftover Data",
			onPressFunc = [&, bad_idxs]()
			{
				for (auto idx : bad_idxs)
					args.erase(idx);
				refresh_dlg();
			}
		));
	}
	
	return window;
}

bool ScriptDataDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			// Save the active inst_init values
			local_ref.inst_init.clear();
			auto& specified = specified_args[local_ref.script];
			for (auto const& [id, val] : init_args[local_ref.script])
				if (specified.get(id))
					local_ref.inst_init[id] = val;
			dest_ref = local_ref;
			mark_save_dirty();
			return true;
		}
		case message::CANCEL:
			return true;
	}
	return false;
}

