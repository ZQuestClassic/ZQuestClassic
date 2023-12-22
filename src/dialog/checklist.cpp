#include "checklist.h"
#include "base/containers.h"
#include <gui/builder.h>
#include <utility>

bool call_checklist_dialog(string const& title,
	vector<def_pair<string,string>> const& flagnames, bitstring& flags)
{
	bool ret = false;
	ChecklistDialog(title, flagnames, flags, ret).show();
	return ret;
}

ChecklistDialog::ChecklistDialog(string const& title,
	vector<def_pair<string,string>> const& flagnames, bitstring& flags, bool& confirm):
	d_title(title), flagnames(flagnames), flags(flags), confirm(confirm)
{}

std::shared_ptr<GUI::Widget> ChecklistDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	auto grid = Columns<10>();
	bool use_info = false;
	for(uint q = 0; q < flagnames.size(); ++q)
		if(flagnames[q].second.size())
		{
			use_info = true;
			break;
		}
	for(uint q = 0; q < flagnames.size(); ++q)
	{
		auto const& pair = flagnames[q];
		auto cbox = Checkbox(text = pair.first, hAlign = 0.0,
			checked = flags.get(q),
			onToggleFunc = [&,q](bool state)
			{
				flags.set(q, state);
			});
		if(use_info)
			grid->add(Row(padding = 0_px, hAlign = 0.0, pair.second.empty() ? DINFOBTN() : INFOBTN(pair.second), cbox));
		else grid->add(cbox);
	}
	
	window = Window(
		title = d_title,
		onClose = message::CANCEL,
		padding = 0_px,
		Column(padding = 0_px,
			grid,
			Row(
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL)
			)
		)
	);
	return window;
}

bool ChecklistDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			confirm = true;
			return true;
		case message::CANCEL:
			confirm = false;
			return true;
	}
	return false;
}

