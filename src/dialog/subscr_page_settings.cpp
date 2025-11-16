#include "subscr_page_settings.h"
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

extern ZCSubscreen subscr_edit;

extern const GUI::ListData list_pgmode;

void call_subscr_pagesettings_dialog()
{
	SubscrPageSettings().show();
}

SubscrPageSettings::SubscrPageSettings()
	: list_pg_mode(list_pgmode.copy().filter([&](GUI::ListItem& itm)
		{
			return itm.value != PGGOTO_NONE;
		}))
{
	local_pageref.copy_settings(subscr_edit.cur_page());
}

static size_t subscr_page_tabs[2];
std::shared_ptr<GUI::Widget> SubscrPageSettings::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	std::shared_ptr<GUI::TabPanel> tpan;
	window = Window(
		title = "Subscreen Page Settings",
		onClose = message::CANCEL,
		Column(
			tpan = TabPanel(ptr = &subscr_page_tabs[0]),
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
	
	{
		std::shared_ptr<GUI::TabPanel> tr_pan = TabPanel(ptr = &subscr_page_tabs[1]);
		
		const auto btnsz = 32_px;
		byte idx = 0;
		for (SubscrPageTransition& transition : local_pageref.transitions)
		{
			#define TRANSITION (local_pageref.transitions[idx])
			tr_pan->add(TabRef(name = to_string(idx),
				Frame(title = "Page Change",
					info = "Change the current subscreen page when a button is pressed.",
					fitParent = true,
					Column(
						Row(
							Button(type = GUI::Button::type::ICON,
								icon = BTNICON_ARROW_LEFT,
								width = btnsz, height = btnsz,
								disabled = idx <= 0,
								onPressFunc = [&, idx]()
								{
									zc_swap(TRANSITION, local_pageref.transitions[idx-1]);
									refresh_dlg();
								}),
							Button(type = GUI::Button::type::ICON,
								icon = BTNICON_TRASH,
								width = btnsz, height = btnsz,
								onPressFunc = [&, idx]()
								{
									auto it = local_pageref.transitions.begin();
									for (byte q = 0; q < idx; ++q)
										++it;
									local_pageref.transitions.erase(it);
									refresh_dlg();
								}),
							Button(type = GUI::Button::type::ICON,
								icon = BTNICON_ARROW_RIGHT,
								width = btnsz, height = btnsz,
								disabled = idx >= local_pageref.transitions.size()-1,
								onPressFunc = [&, idx]()
								{
									zc_swap(TRANSITION, local_pageref.transitions[idx+1]);
									refresh_dlg();
								})
						),
						Rows<3>(
							Label(text = "Mode:",hAlign = 1.0),
							DropDownList(data = list_pg_mode,
								fitParent = true, selectedValue = TRANSITION.pg_mode,
								onSelectFunc = [&, idx](int32_t val)
								{
									TRANSITION.pg_mode = val;
									pgtfs[idx]->setDisabled(val != PGGOTO_TRG);
									pgbtns[idx]->setDisabled(!val);
								}),
							INFOBTN("Which mode to use for page swapping."
								"\nPrev/Next move to the previous or next page."
								"\nTarget moves to a specific page number."
								"\nNone disables page swapping altogether."),
							//
							Label(text = "Target:",hAlign = 1.0),
							pgtfs[idx] = TextField(
								fitParent = true,
								type = GUI::TextField::type::INT_DECIMAL,
								low = 1, high = MAX_SUBSCR_PAGES, val = TRANSITION.pg_targ+1,
								onValChangedFunc = [&, idx](GUI::TextField::type,std::string_view,int32_t val)
								{
									TRANSITION.pg_targ = val-1;
								}),
							INFOBTN("The target page for 'Target' mode."
								" If set to an invalid page, no page change occurs."),
							//
							Label(text = "Transition Effects:"),
							pgbtns[idx] = Button(text = "Edit",
								maxheight = 2_em,
								onPressFunc = [&, idx]()
								{
									call_subscrtransition_dlg(TRANSITION.pg_trans,"Transition Editor: Page Change FX");
								}),
							DummyWidget(),
							//
							DummyWidget(),
							CBOX(TRANSITION.flags,SUBSCR_PAGE_TRANSITION_NOWRAP,"No Wrap",1),
							INFOBTN("If checked, trying to page left from page 0 or"
								" right from the final page will do nothing."),
							//
							DummyWidget(),
							CBOX(TRANSITION.flags,SUBSCR_PAGE_TRANSITION_SWAPMAPSUBSCR,"Swap Map Subscreen",1),
							INFOBTN("If checked, changes between the active and map subscreens.")
						),
						Column(
							Label(text = "Page Button:"),
							_INTBTN_PANEL_HELPER_CAPTURE(TRANSITION.pg_btns, idx)
						)
					)
				)
			));
			++idx;
			#undef TRANSITION
		}
		if (idx < SUBSCRPAGE_MAX_TRANSITIONS)
			tr_pan->add(TabRef(name = to_string(idx),
				Button(text = "Add Transition",
					onPressFunc = [&]()
					{
						local_pageref.transitions.emplace_back();
						refresh_dlg();
					})
			));
		
		tabs.push_back({"Transitions", tr_pan});
	}
	
	for(auto& ref : tabs)
		tpan->add(TabRef(name = ref.first, ref.second)); 
	refr_info();
	return window;
}

void SubscrPageSettings::refr_info()
{
}

bool SubscrPageSettings::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::REFR_INFO:
			refr_info();
			break;
		case message::OK:
			subscr_edit.cur_page().copy_settings(local_pageref);
			return true;
		case message::CANCEL:
			return true;
	}
	return false;
}

