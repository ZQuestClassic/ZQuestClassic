#include "subscr_settings.h"
#include <gui/builder.h>
#include "info.h"
#include <utility>
#include <sstream>
#include <fmt/format.h>
#include "new_subscr.h"
#include "zq/zq_subscr.h"
#include "zc_list_data.h"
#include "gui/use_size.h"
#include "gui/common.h"
#include "subscr_transition.h"
#include "subscr_macros.h"

extern script_data *subscreenscripts[NUMSCRIPTSSUBSCREEN];
extern ZCSubscreen subscr_edit;

void call_subscrsettings_dialog()
{
	SubscrSettingsDialog().show();
}

SubscrSettingsDialog::SubscrSettingsDialog() :
	ty(subscr_edit.sub_type),
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_subscript(GUI::ZCListData::subscreen_script())
{
	local_subref.copy_settings(subscr_edit);
}

std::shared_ptr<GUI::Widget> SubscrSettingsDialog::SUBSCR_INITD(int index)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	return Row(padding = 0_px,
		l_initds[index] = Label(minwidth = 12_em, textAlign = 2),
		ib_initds[index] = Button(forceFitH = true, text = "?",
			disabled = true,
			onPressFunc = [&, index]()
			{
				InfoDialog("InitD Info",h_initd[index]).show();
			}),
		tf_initd[index] = TextField(
			fitParent = true, minwidth = 8_em,
			type = GUI::TextField::type::SWAP_ZSINT,
			val = local_subref.initd[index],
			onValChangedFunc = [&, index](GUI::TextField::type,std::string_view,int32_t val)
			{
				local_subref.initd[index] = val;
			})
	);
}

size_t subscr_sett_tabs[3];
std::shared_ptr<GUI::Widget> SubscrSettingsDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TabPanel> tpan;
	window = Window(
		title = "Subscreen Settings",
		onClose = message::CANCEL,
		Column(
			tpan = TabPanel(ptr = &subscr_sett_tabs[ty]),
			Row(
				Button(
					text = "&OK",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::OK,
					focused = true),
				Button(
					text = "&Cancel",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	std::vector<std::pair<std::string, std::shared_ptr<GUI::Widget>>> tabs;
	switch(ty)
	{
		case sstACTIVE:
		{
			tabs.push_back({"Basic", Rows<3>(
				Frame(title = "Page Left",
					info = "Pressing this button will change the page of the subscreen by '-1'.",
					Column(padding = 0_px,
						_INTBTN_PANEL_HELPER(local_subref.btn_left),
						Button(text = "Edit Transition",
							maxheight = 2_em,
							onPressFunc = [&]()
							{
								call_subscrtransition_dlg(local_subref.trans_left,"Transition Editor: Page Left Btn");
							})
					)
				),
				Frame(title = "Page Right",
					info = "Pressing this button will change the page of the subscreen by '+1'.",
					Column(padding = 0_px,
						_INTBTN_PANEL_HELPER(local_subref.btn_right),
						Button(text = "Edit Transition",
							maxheight = 2_em,
							onPressFunc = [&]()
							{
								call_subscrtransition_dlg(local_subref.trans_right,"Transition Editor: Page Right Btn");
							})
					)
				),
				Frame(title = "Flags", fitParent = true,
					Rows<2>(
						INFOBTN("If checked, trying to page left from page 0 or"
							" right from the final page will do nothing."),
						Checkbox(
							text = "No Page Wrap",
							checked = local_subref.flags & SUBFLAG_ACT_NOPAGEWRAP,
							onToggleFunc = [&](bool state)
							{
								SETFLAG(local_subref.flags,SUBFLAG_ACT_NOPAGEWRAP,state);
							}
						)
					)
				)
				// Frame(title = "Attribs", fitParent = true, colSpan = 3,
					// Rows<3>(
						
					// )
				// )
			)});
			tabs.push_back({"Selector", Column(
				Rows<2>(
					CBOX_EX(local_subref.flags,SUBFLAG_ACT_OVERRIDESEL,"Override Selector",
						_EX_RBOX,onToggle = message::REFR_SELECTOR),
					INFOBTN("Change the selector for this subscreen")
				),
				selector_grid = Row(
					Frame(title = "Dimensions", fitParent = true,
						Rows<3>(
							Label(text = "Selector X-offset", hAlign = 1.0),
							NUM_FIELD(local_subref.selector_setting.x,-256,256),
							INFOBTN("The selector x will be offset by this much"),
							Label(text = "Selector Y-offset", hAlign = 1.0),
							NUM_FIELD(local_subref.selector_setting.y,-256,256),
							INFOBTN("The selector y will be offset by this much"),
							Label(text = "Selector Width-offset", hAlign = 1.0),
							NUM_FIELD(local_subref.selector_setting.w,-256,256),
							INFOBTN("The selector width will be offset by this much"),
							Label(text = "Selector Height-offset", hAlign = 1.0),
							NUM_FIELD(local_subref.selector_setting.h,-256,256),
							INFOBTN("The selector height will be offset by this much")
						)),
					Frame(title = "Selector 1 Graphic", fitParent = true,
						info = "Override the Tile, CSet, and Width/Height used by selectors"
							" with 'Use Selector 2' unchecked."
							"\nWidth/Height are given in pixels, and that pixel size will be"
							" used as the source size of the draw. These sizes rounded up to the"
							" next full tile will be the tile width/height of the draw.",
						SELECTOR_GRAPHIC(local_subref.selector_setting.tileinfo[0])
					),
					Frame(title = "Selector 2 Graphic", fitParent = true,
						info = "Override the Tile, CSet, and Width/Height used by selectors"
							" with 'Use Selector 2' checked."
							"\nWidth/Height are given in pixels, and that pixel size will be"
							" used as the source size of the draw. These sizes rounded up to the"
							" next full tile will be the tile width/height of the draw.",
						SELECTOR_GRAPHIC(local_subref.selector_setting.tileinfo[1])
					)
				)
			)});
			tabs.push_back({"Script", Row(
					Column(
						SUBSCR_INITD(0),
						SUBSCR_INITD(1),
						SUBSCR_INITD(2),
						SUBSCR_INITD(3),
						SUBSCR_INITD(4),
						SUBSCR_INITD(5),
						SUBSCR_INITD(6),
						SUBSCR_INITD(7)
					),
					Column(
						padding = 0_px, fitParent = true,
						Rows<2>(vAlign = 0.0,
							SCRIPT_LIST_PROC("Script:", list_subscript, local_subref.script, refr_script)
						)
					)
				)});
			break;
		}
		case sstPASSIVE:
		{
			tabs.push_back({"Basic", Label(text = "No settings for passive subscreens yet!")});
			break;
		}
		case sstOVERLAY:
		{
			tabs.push_back({"Basic", Label(text = "No settings for overlay subscreens yet!")});
			break;
		}
	}
	for(auto& ref : tabs)
		tpan->add(TabRef(name = ref.first, ref.second)); 
	refr_info();
	refr_script();
	return window;
}

void SubscrSettingsDialog::refr_selector()
{
	if(ty == sstACTIVE)
	{
		selector_grid->setDisabled(!(local_subref.flags&SUBFLAG_ACT_OVERRIDESEL));
	}
}
void SubscrSettingsDialog::refr_script()
{
	std::string label[8], help[8];
	for(auto q = 0; q < 8; ++q)
	{
		label[q] = "InitD["+std::to_string(q)+"]";
	}
	if(local_subref.script)
	{
		zasm_meta const& meta = subscreenscripts[local_subref.script]->meta;
		for(size_t q = 0; q < 8; ++q)
		{
			if(meta.initd[q].size())
				label[q] = meta.initd[q];
			if(meta.initd_help[q].size())
				help[q] = meta.initd_help[q];
		}
		
		for(auto q = 0; q < 8; ++q)
		{
			if(unsigned(meta.initd_type[q]) < nswapMAX)
				tf_initd[q]->setSwapType(meta.initd_type[q]);
		}
	}
	else
	{
		for(auto q = 0; q < 8; ++q)
		{
			tf_initd[q]->setSwapType(nswapDEC);
		}
	}
	for(auto q = 0; q < 8; ++q)
	{
		l_initds[q]->setText(label[q]);
		h_initd[q] = help[q];
		ib_initds[q]->setDisabled(help[q].empty());
	}
}
void SubscrSettingsDialog::refr_info()
{
}

bool SubscrSettingsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_SELECTOR:
			refr_selector();
			break;
		case message::REFR_INFO:
			refr_info();
			break;
		case message::OK:
			subscr_edit.copy_settings(local_subref);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

