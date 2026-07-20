#include "common.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include "global_initd.h"
#include "core/initdata.h"
#include "script_data_editor.h"

extern script_data *globalscripts[NUMSCRIPTGLOBAL];
extern script_data *playerscripts[NUMSCRIPTHERO];

void call_global_initd_dialog()
{
	GlobalInitDDialog().show();
}

GlobalInitDDialog::GlobalInitDDialog()
{}

static size_t tabpptr = 0;
static int32_t scrollptrs[2] = {0};
std::shared_ptr<GUI::Widget> GlobalInitDDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Key;
	using namespace GUI::Props;
	
	auto global_script_col = Rows<2>();
	auto hero_script_col = Rows<2>();
	
	auto list_global = GUI::ZCListData::slots_global_script(false, true);
	auto list_hero = GUI::ZCListData::slots_hero_script(false, true);
	
	bool found_global = false, found_hero = false;
	for (int q = 1; q < NUMSCRIPTGLOBAL; ++q)
	{
		if (!list_global.hasKey(q))
			continue;
		found_global = true;
		string name = list_global.findText(q);
		global_script_col->add(Label(hAlign = 1.0, text = name));
		global_script_col->add(
			Button(
				text = "Script Setup",
				height = 2_em,
				onPressFunc = [&, q, name]()
				{
					ScriptDataDialog(fmt::format("Global Script '{}' Setup", name),
						zinit.global_scrconfig[q], GUI::ListData::nullData(), globalscripts).show();
				}
			)
		);
	}
	for (int q = 1; q < NUMSCRIPTHERO; ++q)
	{
		if (!list_hero.hasKey(q))
			continue;
		found_hero = true;
		string name = list_hero.findText(q);
		hero_script_col->add(Label(hAlign = 1.0, text = name));
		hero_script_col->add(
			Button(
				text = "Script Setup",
				height = 2_em,
				onPressFunc = [&, q, name]()
				{
					ScriptDataDialog(fmt::format("Hero Script '{}' Setup", name),
						zinit.hero_scrconfig[q], GUI::ListData::nullData(), playerscripts).show();
				}
			)
		);
	}
	if (!found_global)
		global_script_col->add(Label(colSpan = 2, text = "No Global Scripts Found"));
	if (!found_hero)
		hero_script_col->add(Label(colSpan = 2, text = "No Hero Scripts Found"));
	
	window = Window(
		title = "Global / Hero Script Data",
		onClose = message::OK,
		Column(
			TabPanel(ptr = &tabpptr,
				fitParent = true,
				TabRef(name = "Global", ScrollingPane(
					ptr_y = &scrollptrs[0],
					fitParent = true,
					minheight = 12_em,
					global_script_col
				)),
				TabRef(name = "Hero", ScrollingPane(
					ptr_y = &scrollptrs[0],
					fitParent = true,
					minheight = 12_em,
					hero_script_col
				))
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "Done",
					minwidth = 90_px,
					onClick = message::OK)
			)
		)
	);
	return window;
}

bool GlobalInitDDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			return true;
	}
	return false;
}
