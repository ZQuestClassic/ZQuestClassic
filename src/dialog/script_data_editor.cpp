#include "script_data_editor.h"
#include <gui/builder.h>
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "zc_list_data.h"
#include "info.h"

ScriptDataDialog::ScriptDataDialog(string const& title, script_config& cfg,
	GUI::ListData const& scripts, script_data const* const* scrdatas)
	: dest_ref(cfg), local_ref(cfg), init_args(), title_str(title),
	list_scriptchoices(scripts), scrdatas(scrdatas), specified_args()
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
	
	auto initd_grid = Rows<3>(spacing = 2_px, topPadding = 10_px);
	auto instvar_grid = Rows<4>(spacing = 2_px, padding = 0_px);
	
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
	main_column->add(TabPanel(ptr = &tab_pos,
		TabRef(name = "InitD[]",
			Frame(info = "These values represent the up to 8 parameters of the script's `Run` function."
				" Modifying them will configure script-specific behaviors."
				"\n\nUsing '@ExportInitD0()' through '@ExportInitD7()' in scripts allows specifying custom"
				" help text for the '?' buttons of each option, as well as customizing the name and default"
				" input state of the field.",
				fitParent = true,
				initd_grid
			)
		),
		TabRef(name = "Exports",
			Frame(info = "These variables are specific to the currently-selected script,"
				" and configure script-specific behaviors."
				"\nThe value will be the 'script default' value, unless you modify it here."
				" Values that have been modified will have a reset button to their right to"
				" set them back to default. (Modified values will not change even if the script"
				" is edited to change a value)"
				"\n\nUsing '@Export()' allows adding a script-scope variable to this list."
				" Using '@ExportRange()' on an already exported variable allows setting a min/max value"
				" for the field.",
				fitParent = true,
				ScrollingPane(
					ptr_y = &scroll_pos,
					fitParent = true,
					padding = 0_px, topPadding = 10_px,
					instvar_grid
				)
			)
		)
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
	
	script_data const* scrdata = nullptr;
	if (local_ref.script)
		scrdata = scrdatas[local_ref.script];
	for (int ind = 0; ind < 8; ++ind) // InitD[]
	{
		string label = fmt::format("InitD[{}]", ind);
		byte swp = nswapDEC;
		string help;
		if (scrdata)
		{
			auto& meta = scrdata->meta;
			if (!meta.initd_label[ind].empty())
				label = meta.initd_label[ind];
			if (!meta.initd_help[ind].empty())
				help = meta.initd_help[ind];
			if (unsigned(meta.initd_type[ind]) < nswapMAX)
				swp = meta.initd_type[ind];
		}
		
		initd_grid->add(Label(
			text = label, hAlign = 1.0
		));
		initd_grid->add(help.empty() ? DINFOBTN() : INFOBTN(help));
		initd_grid->add(TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			swap_type = swp,
			val = local_ref.run_args[ind],
			onValChangedFunc = [&, ind](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_ref.run_args[ind] = val;
			}));
	}
	
	std::set<word> added_idxs;
	auto& args = init_args[local_ref.script];
	
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
			auto& specified = specified_args[local_ref.script];
			for (auto const& [idx, expdata] : scrdata->script_d_exports.inner())
			{
				added_idxs.insert(idx);
				auto reset_btn = Button(
					padding = 0_px, rightPadding = 12_px,
					width = 24_px, height = 24_px,
					type = GUI::Button::type::ICON,
					icon = BTNICON_LOOP_ARROW,
					visible = specified.get(idx)
				);
				instvar_grid->add(Label(
					text = expdata.name, hAlign = 1.0
				));
				instvar_grid->add(expdata.helptext.empty() ? DINFOBTN() : INFOBTN(expdata.helptext));
				byte swp = nswapDEC;
				if (unsigned(expdata.btn_type) < nswapMAX)
					swp = expdata.btn_type;
				if (expdata.min == 0_zf && expdata.max == 0.0001_zf && swp == nswapBOOL)
				{
					// Checkbox input
					auto cb = Checkbox(
						fitParent = true, checked = args[idx],
						onToggleFunc = [&, idx, reset_btn](bool state)
						{
							args[idx] = state ? 10000 : 0;
							reset_btn->setVisible(true);
							specified.set(idx, true);
						});
					reset_btn->setOnPress([&, idx, cb, scrdata, reset_btn]()
						{
							specified.set(idx, false);
							args[idx] = scrdata->script_d_init.get(idx);
							cb->setChecked(args[idx]);
							reset_btn->setVisible(false);
						});
					instvar_grid->add(cb);
				}
				else
				{
					auto tf = TextField(
						fitParent = true, minwidth = 8_em,
						type = GUI::TextField::type::SWAP_ZSINT,
						swap_type = swp, val = args[idx],
						onValChangedFunc = [&, idx, reset_btn](GUI::TextField::type,std::string_view,int32_t val)
						{
							args[idx] = val;
							reset_btn->setVisible(true);
							specified.set(idx, true);
						});
					if (expdata.min < expdata.max && !(expdata.min == -214748.3648_zf && expdata.max == 214748.3647_zf))
					{
						tf->setLowBound(expdata.min.getZLong());
						tf->setHighBound(expdata.max.getZLong());
					}
					reset_btn->setOnPress([&, idx, tf, scrdata, reset_btn]()
						{
							specified.set(idx, false);
							args[idx] = scrdata->script_d_init.get(idx);
							tf->setVal(args[idx]);
							reset_btn->setVisible(false);
						});
					instvar_grid->add(tf);
				}
				instvar_grid->add(reset_btn);
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
		instvar_grid->add(Label(text = "No '@Export'ed Instance Variables.", colSpan = 3));
	if (!bad_idxs.empty())
	{
		bool singular = bad_idxs.size() == 1;
		instvar_grid->add(Label(colSpan = 4,
			maxwidth = 250_px,
			text = fmt::format("{} instance value{} found set in the quest file that"
				" do{} not match any '@Export'ed instance variable. This data may be leftover"
				" from a previous script, and is likely useless in it's current state.",
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

