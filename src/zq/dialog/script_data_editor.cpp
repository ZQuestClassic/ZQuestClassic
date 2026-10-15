#include "script_data_editor.h"
#include "zq/gui/builder_zq.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zc_list_data.h"
#include "dialog/info.h"
#include <fmt/ranges.h>
#include <optional>

extern int32_t numericalFlags;

static string format_export_value(int32_t val, bool is_long)
{
	return is_long ? std::to_string(val) : zslongToFix(val).str_trim();
}

static string not_an_option_problem(int32_t val, bool is_long)
{
	return fmt::format("{} isn't one of the options.\nIt's kept as is unless you choose an"
		" option or reset it.", format_export_value(val, is_long));
}
// Makes a dropdown for an engine value or a custom list. These differ only in
// how a stored value maps to a list key: key = value / scale. A stored value
// that isn't one of the options is shown as is, as an extra entry.
static std::shared_ptr<GUI::DropDownList> make_export_dropdown(ExportWidgetData& widget_data,
	GUI::ListData& options, int32_t stored, bool is_long, int32_t scale, std::function<void(int)> set_arg)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	auto is_option = [&options, scale](int val) {
		return val % scale == 0 && options.hasKey(val / scale);
	};

	// The extra entry gets a key that isn't one of the options.
	GUI::ListData* list = &options;
	int32_t selected = stored / scale;
	std::optional<int32_t> stored_key;
	if (!is_option(stored))
	{
		stored_key = INT32_MIN;
		while (options.hasKey(*stored_key))
			++*stored_key;
		// Don't add the entry to a list that other dropdowns share.
		if (list != &widget_data.list)
		{
			widget_data.list = options;
			list = &widget_data.list;
		}
		list->add(format_export_value(stored, is_long), *stored_key);
		selected = *stored_key;
	}

	GUI::ListData const* opts = list;
	widget_data.problem = [opts, is_long, scale, stored_key](int val) -> std::string
	{
		if (val % scale == 0 && val / scale != stored_key && opts->hasKey(val / scale))
			return "";
		return not_an_option_problem(val, is_long);
	};

	auto ddl = DropDownList(
		fitParent = true, width = 200_px,
		data = *list,
		selectedValue = selected,
		onSelectFunc = [set_arg, stored, scale, stored_key](int32_t key)
		{
			set_arg(key == stored_key ? stored : key * scale);
		}
	);
	widget_data.main_widget = ddl;
	widget_data.set_widg_val = [ddl, scale](int val)
		{
			ddl->setSelectedValue(val / scale);
		};
	return ddl;
}

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
	list_engine_combo_flag(GUI::ZCListData::mapflag(numericalFlags, true)),
	checks_engine_level_item(GUI::ZCCheckListData::level_items())
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
		meta = &scrdata->meta.get();
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
				update_export_invalid(idx);
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
					update_export_invalid(idx);
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
								tile = args[idx] / 10000,
								showvals = false,
								hAlign = 0.0,
								onSelectFunc = [&, set_arg](int32_t t, int32_t,int32_t,int32_t)
								{
									set_arg(t * 10000);
								}
							);
							widget_data.main_widget = tswatch;
							widget_data.set_widg_val = [&, tswatch](int val)
								{
									tswatch->setTile(val / 10000);
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
									tswatch->setCSet(val / 10000);
								};
								tswatch->setCSet(args[idx] / 10000);
								tswatch->setRowSpan(2);
								instvar_grid->add(_d); // setRowSpan doesn't update the grid, so add a dummy to eat the space
								auto set_tile = prev_widget_data->set_arg;
								auto set_cset = set_arg;
								tswatch->setOnSelectFunc([&, set_tile, set_cset](int t, int cs, int, int)
								{
									set_tile(t * 10000);
									set_cset(cs * 10000);
								});
							}
							break;
						}
						
						case special_engine_export::combo:
						{
							auto cswatch = SelComboSwatch(
								combo = args[idx] / 10000,
								showvals = false,
								hAlign = 0.0,
								onSelectFunc = [&, set_arg](int32_t cmb, int32_t)
								{
									set_arg(cmb * 10000);
								}
							);
							widget_data.main_widget = cswatch;
							widget_data.set_widg_val = [&, cswatch](int val)
								{
									cswatch->setCombo(val / 10000);
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
									cswatch->setCSet(val / 10000);
								};
								cswatch->setCSet(args[idx] / 10000);
								cswatch->setRowSpan(2);
								instvar_grid->add(_d); // setRowSpan doesn't update the grid, so add a dummy to eat the space
								auto set_combo = prev_widget_data->set_arg;
								auto set_cset = set_arg;
								cswatch->setOnSelectFunc([&, set_combo, set_cset](int c, int cs)
								{
									set_combo(c * 10000);
									set_cset(cs * 10000);
								});
							}
							break;
						}
						
						case special_engine_export::color:
						{
							auto csel = ColorSel(
								fitParent = true, width = 100_px,
								hAlign = 0.0,
								val = args[idx] / 10000,
								onValChangedFunc = [&, set_arg](byte val)
								{
									set_arg(val * 10000);
								}
							);
							widget_data.main_widget = csel;
							widget_data.set_widg_val = [&, csel](int val)
								{
									csel->setVal(val / 10000);
								};
							instvar_grid->add(csel);
							break;
						}
						
						 // Group bitflag types together for reduced duplication
						case special_engine_export::level_item:
						{
							bool is_long = false;
							CheckList* clist = nullptr;
							switch (expdata.engine_type)
							{
								case special_engine_export::level_item:
									clist = &checks_engine_level_item;
									break;
							}
							if (!clist)
							{
								instvar_grid->add(Label(text = "!!ERROR!!"));
								NOTREACHED();
								break;
							}
							
							auto btn = Button(text = "P",
								hAlign = 0.0, padding = 0_px,
								width = 24_px, height = 24_px,
								onPressFunc = [&, idx, is_long, set_arg, clist]()
								{
									int flags = args[idx];
									if (!is_long) flags /= 10000;
									if (!call_checklist_dialog(fmt::format("Select '{}'", expdata.name),
										*clist, flags))
										return;
									if (!is_long) flags *= 10000;
									set_arg(flags);
								}
							);
							widget_data.main_widget = btn;
							instvar_grid->add(btn);
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
							instvar_grid->add(make_export_dropdown(widget_data, *ld, args[idx], false, 10000, set_arg));
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
								ld.add(name, val.getZLong());
							bool values_long = expdata.btn_type == nswapLDEC || expdata.btn_type == nswapLHEX;
							instvar_grid->add(make_export_dropdown(widget_data, ld, args[idx], values_long, 1, set_arg));
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
							
							int known_flags = 0;
							for (auto& [v, _] : expdata.custom_export_names)
								known_flags |= is_long ? v.getZLong() : v.getInt();
							widget_data.problem = [is_long, known_flags](int val) -> std::string
							{
								if (!is_long && val % 10000)
									return fmt::format("{} isn't a valid set of flags.\nIt's kept as is unless"
										" you change the flags or reset them.", format_export_value(val, false));
								int unlisted = (is_long ? val : val / 10000) & ~known_flags;
								if (!unlisted)
									return "";
								std::vector<string> bits;
								for (int q = 0; q < 32; ++q)
									if (unlisted & (1u << q))
										bits.push_back(std::to_string(1u << q));
								return fmt::format("Flags that aren't listed are set ({}).\nThey're kept as is,"
									" even when changing the listed flags, unless you reset them.",
									fmt::join(bits, ", "));
							};
							
							auto btn = Button(text = "P",
								hAlign = 0.0, padding = 0_px,
								width = 24_px, height = 24_px,
								onPressFunc = [&, idx, is_long, set_arg]()
								{
									int flags = args[idx];
									if (!is_long) flags /= 10000;
									if (!call_checklist_dialog(fmt::format("Select '{}'", expdata.name),
										export_widgets[idx].clist_info, flags))
										return;
									if (!is_long) flags *= 10000;
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
						int min = expdata.min.getZLong(), max = expdata.max.getZLong();
						// Show the stored value as is; a typed one is clamped.
						tf->setBounds({min, max}, false);
						bool is_long = swp == nswapLDEC || swp == nswapLHEX;
						auto range = is_long ? fmt::format("{} to {}", min, max)
							: fmt::format("{} to {}", expdata.min.str_trim(), expdata.max.str_trim());
						widget_data.problem = [min, max, range, is_long](int val) -> std::string
						{
							if (val >= min && val <= max)
								return "";
							return fmt::format("{} is outside of the allowed range ({}).\nIt's kept as is"
								" unless you change it or reset it.", format_export_value(val, is_long), range);
						};
						// A typed value is clamped when saved, and shown clamped once the
						// field is left; until then, say so.
						GUI::TextField* tf_ptr = tf.get();
						tf->setOnValChanged([&, set_arg, tf_ptr, range, is_long](GUI::TextField::type, std::string_view, int32_t val)
						{
							set_arg(val);
							int32_t typed = tf_ptr->getRawVal();
							if (typed == val)
								return;
							tf_ptr->setInvalid(true);
							tf_ptr->setTooltip(fmt::format("{} is outside of the allowed range ({}).\n{} will be used instead.",
								format_export_value(typed, is_long), range, format_export_value(val, is_long)));
						});
					}
					widget_data.main_widget = tf;
					widget_data.set_widg_val = [&, tf](int val)
						{
							tf->setVal(val);
						};
					instvar_grid->add(tf);
				}
				instvar_grid->add(reset_btn);
				update_export_invalid(idx);
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

void ScriptDataDialog::update_export_invalid(word idx)
{
	auto& widget_data = export_widgets[idx];
	if (!widget_data.problem || !widget_data.main_widget)
		return;
	std::string problem = widget_data.problem(init_args[local_ref.script][idx]);
	widget_data.main_widget->setInvalid(!problem.empty());
	widget_data.main_widget->setTooltip(problem);
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

