#include "checklist.h"
#include "base/containers.h"
#include <gui/builder.h>
#include <utility>
#include <dialog/info.h>

bool call_checklist_dialog(string const& title,
	vector<CheckListInfo> const& flagnames,
	bitstring& flags, std::optional<size_t> per_col)
{
	bool ret = false;
	ChecklistDialog(title, flagnames, flags, ret, per_col).show();
	return ret;
}
bool call_checklist_dialog(string const& title,
	vector<CheckListInfo> const& flagnames,
	int32_t& flags, std::optional<size_t> per_col)
{
	bitstring bitstr;
	for(int q = 0; q < 32; ++q)
		if(flags & (1<<q))
			bitstr.set(q, true);
	bool ret = false;
	ChecklistDialog(title, flagnames, bitstr, ret, per_col).show();
	flags = 0;
	for(int q = 0; q < 32; ++q)
		if(bitstr.get(q))
			flags |= (1<<q);
	return ret;
}

bool call_checklist_dialog(string const& title,
	vector<CheckListInfo> const& flagnames,
	dword& flags, std::optional<size_t> per_col)
{
	return call_checklist_dialog(title, flagnames, (int32_t&)flags, per_col);
}

ChecklistDialog::ChecklistDialog(string const& title,
	vector<CheckListInfo> const& flagnames, bitstring& flags,
	bool& confirm, std::optional<size_t> per_col):
	d_title(title), flagnames(flagnames), flags(flags),
	confirm(confirm), per_col(per_col)
{}

std::shared_ptr<GUI::Widget> ChecklistDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	if (!per_col)
	{
		auto numflags = flagnames.size();
		size_t best_per_col = 0;
		size_t best_remainder = numflags;
		for (size_t q = vbound(numflags/2, 4, 10); q > 3; --q)
		{
			auto rem = numflags % q;
			if (rem < best_remainder)
			{
				best_per_col = q;
				best_remainder = rem;
			}
			if (!rem)
				break;
		}
		if (best_per_col > 0)
			per_col = best_per_col;
		else
			per_col = 10;
	}
	auto grid = GUI::Grid::columns(*per_col);
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
				Button(
					text = "Check All",
					onPressFunc = [&]()
					{
						for(uint q = 0; q < flagnames.size(); ++q)
						{
							auto ref = flagnames[q];
							if(ref.flags & CheckListInfo::DISABLED)
								continue;
							flags.set(q, true);
						}
						refresh_dlg();
					}),
				Button(
					text = "Uncheck All",
					onPressFunc = [&]()
					{
						for(uint q = 0; q < flagnames.size(); ++q)
						{
							auto ref = flagnames[q];
							if(ref.flags & CheckListInfo::DISABLED)
								continue;
							flags.set(q, false);
						}
						refresh_dlg();
					}),
				Button(
					text = "Toggle All",
					onPressFunc = [&]()
					{
						for(uint q = 0; q < flagnames.size(); ++q)
						{
							auto ref = flagnames[q];
							if(ref.flags & CheckListInfo::DISABLED)
								continue;
							flags.toggle(q);
						}
						refresh_dlg();
					})
			),
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

