#include "checklist.h"
#include "base/containers.h"
#include <gui/builder.h>
#include <utility>

bool call_checklist_dialog(string const& title,
	vector<CheckListInfo> const& flagnames, bitstring& flags)
{
	bool ret = false;
	ChecklistDialog(title, flagnames, flags, ret).show();
	return ret;
}
bool call_checklist_dialog(string const& title,
	vector<CheckListInfo> const& flagnames, int32_t& flags)
{
	bitstring bitstr;
	for(int q = 0; q < 32; ++q)
		if(flags & (1<<q))
			bitstr.set(q, true);
	bool ret = false;
	ChecklistDialog(title, flagnames, bitstr, ret).show();
	flags = 0;
	for(int q = 0; q < 32; ++q)
		if(bitstr.get(q))
			flags |= (1<<q);
	return ret;
}

ChecklistDialog::ChecklistDialog(string const& title,
	vector<CheckListInfo> const& flagnames, bitstring& flags, bool& confirm):
	d_title(title), flagnames(flagnames), flags(flags), confirm(confirm)
{}

std::shared_ptr<GUI::Widget> ChecklistDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	auto grid = (flagnames.size() > 5 && flagnames.size() < 20)
		? GUI::Grid::columns((flagnames.size()+1)/2)
		: Columns<10>();
	bool use_info = false;
	for(uint q = 0; q < flagnames.size(); ++q)
		if(!flagnames[q].info.empty())
		{
			use_info = true;
			break;
		}
	for(uint q = 0; q < flagnames.size(); ++q)
	{
		auto const& ref = flagnames[q];
		auto cbox = Checkbox(text = ref.name.empty() ? "--" : ref.name, hAlign = 0.0,
			checked = flags.get(q), disabled = ref.flags & CheckListInfo::DISABLED,
			onToggleFunc = [&,q](bool state)
			{
				flags.set(q, state);
			});
		if(use_info)
			grid->add(Row(padding = 0_px, hAlign = 0.0, ref.info.empty() ? DINFOBTN() : INFOBTN(ref.info), cbox));
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

