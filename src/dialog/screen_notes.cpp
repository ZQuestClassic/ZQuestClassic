#include "screen_notes.h"
#include <gui/builder.h>
#include <base/new_menu.h>
#include "alert.h"
#include "zc_list_data.h"
#include "base/mapscr.h"
#include "zq/zquest.h"
#include "zq/zq_class.h"
#include "gui/editbox.h"
#include "qst.h"
#include <fmt/format.h>
#include <utility>
#include <sstream>

void edit_screen_notes(mapscr* scr, int map, int screen)
{
	ScreenNotesInfo(scr,map,screen).edit();
}

void browse_screen_notes()
{
	BrowseNotesDialog().show();
}

void ScreenNotesInfo::update_blank()
{
	blank = notes().empty() || !scr || !scr->valid;
}

string const& ScreenNotesInfo::notes() const
{
	static const string nullstr = "";
	return scr ? scr->usr_notes : nullstr;
}

bool ScreenNotesInfo::edit()
{
	if(do_box_edit(scr->usr_notes, fmt::format("Notes ({})", title()), true, false, true))
	{
		update_blank();
		scr->valid |= mVALID;
		saved = false;
		return true;
	}
	return false;
}

string ScreenNotesInfo::title() const
{
	return fmt::format("{},0x{:02X}", map+1, screen);
}
string ScreenNotesInfo::str() const
{
	return fmt::format("({}): {}", title(), scr ? notes() : "[NULL]");
}

BrowseNotesDialog::BrowseNotesDialog()
{
	selected_val = 0;
	infos.reserve(map_count*MAPSCRS);
	for(int m = 0; m < map_count; ++m)
		for(int s = 0; s < MAPSCRS; ++s)
			infos.emplace_back(Map.AbsoluteScr(m,s),m,s);
}

std::shared_ptr<GUI::Widget> BrowseNotesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;
	
	string lowfilter = filter;
	lowerstr(lowfilter);
	lister.clear();
	size_t indx = 0;
	bool nofilter = filter.empty();
	bool isblank = true;
	bool resel = true;
	for(ScreenNotesInfo& inf : infos)
	{
		if(!inf.isBlank())
		{
			isblank = false;
			bool add = nofilter;
			if(!add)
			{
				string lowernotes = inf.notes();
				lowerstr(lowernotes);
				add = lowernotes.find(lowfilter) != string::npos;
			}
			if(add)
			{
				if(indx == selected_val)
					resel = false;
				lister.add(inf.str(), indx);
			}
		}
		++indx;
	}
	
	if(isblank)
	{
		InfoDialog("Error","No Notes Found!").show();
		return nullptr;
	}
	if(lister.empty())
	{
		selected_val = -1;
		resel = false;
		lister.add("[NO RESULTS]", -1);
	}
	else if(selected_val < 0)
	{
		selected_val = lister.getValue(0);
		resel = false;
	}
	
	window = Window(
		title = "Browse Notes",
		onClose = message::OK,
		hPadding = 0_px,
		Column(
			hPadding = 0_px, 
			Row(
				widgList = List(data = lister, isABC = true,
					selectedValue = selected_val,
					fitParent = true,
					onSelectFunc = [&](int32_t val)
					{
						if(selected_val != val)
						{
							selected_val = val;
							if(auto inf = sel_info())
							{
								prevlbl->setText(inf->notes());
								editB->setDisabled(false);
								gotoB->setDisabled(inf->map == Map.getCurrMap() && inf->screen == Map.getCurrScr());
							}
							else
							{
								prevlbl->setText("");
								editB->setDisabled(true);
								gotoB->setDisabled(true);
							}
						}
					},
					onDClick = message::EDIT),
				prevlbl = Label(text = "",
					width = 30_em, minheight = 30_em,
					fitParent = true)
			),
			Row(padding = 0_px,
				Row(padding = 0_px,
					Button(
						text = "C", height = 24_px,
						rightPadding = 0_px,
						onPressFunc = [&]()
						{
							filter.clear();
							refresh_dlg();
						}),
					TextField(
						maxwidth = 15_em, maxLength = 255,
						text = filter, focused = true,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view str,int32_t)
						{
							filter = str;
							refresh_dlg();
						})
				),
				Button(
					text = "OK",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::OK),
				editB = Button(
					text = "Edit",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::EDIT),
				gotoB = Button(
					text = "Goto",
					topPadding = 0.5_em,
					minwidth = 90_px,
					onClick = message::GOTO)
			)
		)
	);
	
	if(resel)
	{
		widgList->setIndex();
		selected_val = widgList->getSelectedValue();
	}
	editB->setDisabled(true);
	gotoB->setDisabled(true);
	if(selected_val < 0)
		prevlbl->setText("No matching notes found!");
	else if(auto inf = sel_info())
	{
		editB->setDisabled(false);
		gotoB->setDisabled(inf->map == Map.getCurrMap() && inf->screen == Map.getCurrScr());
		prevlbl->setText(inf->notes());
	}
	
	return window;
}

ScreenNotesInfo* BrowseNotesDialog::sel_info()
{
	if(selected_val < 0 || selected_val >= infos.size() || lister.empty())
		return nullptr;
	return &infos[selected_val];
}
ScreenNotesInfo const* BrowseNotesDialog::sel_info() const
{
	if(selected_val < 0 || selected_val >= infos.size() || lister.empty())
		return nullptr;
	return &infos[selected_val];
}
bool BrowseNotesDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			return true;
		case message::EDIT:
			if(auto inf = sel_info())
				if(inf->edit())
					rerun_dlg = true;
			return rerun_dlg;
		case message::GOTO:
			if(auto inf = sel_info())
			{
				Map.setCurrMap(inf->map);
				Map.setCurrScr(inf->screen);
				return true;
			}
			return false;
	}
	return false;
}

