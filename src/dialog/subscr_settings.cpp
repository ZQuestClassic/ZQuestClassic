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

extern ZCSubscreen subscr_edit;

void call_subscrsettings_dialog()
{
	SubscrSettingsDialog().show();
}

SubscrSettingsDialog::SubscrSettingsDialog() :
	ty(subscr_edit.sub_type),
	list_sfx(GUI::ZCListData::sfxnames(true))
{
	local_subref.copy_settings(subscr_edit);
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
	std::map<std::string, std::shared_ptr<GUI::Widget>> tabs;
	switch(ty)
	{
		case sstACTIVE:
		{
			tabs["Basic"] = Rows<3>(
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
			);
			break;
		}
		case sstPASSIVE:
		{
			tabs["Basic"] = Label(text = "No settings for passive subscreens yet!");
			break;
		}
		case sstOVERLAY:
		{
			tabs["Basic"] = Rows<2>(
				INFOBTN("Hides the overlay while a message string is being displayed"),
				Checkbox(
					text = "Hide for Message String",
					checked = local_subref.flags & SUBFLAG_OVR_HIDEFORSTR,
					onToggleFunc = [&](bool state)
					{
						SETFLAG(local_subref.flags,SUBFLAG_OVR_HIDEFORSTR,state);
					}
				)
			);
			break;
		}
	}
	for(auto& ref : tabs)
		tpan->add(TabRef(name = ref.first, ref.second)); 
	refr_info();
	return window;
}

void SubscrSettingsDialog::refr_info()
{
}

bool SubscrSettingsDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
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

