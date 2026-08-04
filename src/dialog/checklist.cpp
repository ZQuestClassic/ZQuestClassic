#include "checklist.h"
#include "base/containers.h"
#include <gui/builder.h>
#include <utility>

bool call_checklist_dialog(string const& title,
	vector<string> const& flagnames, bitstring& flags)
{
	bool ret = false;
	ChecklistDialog(title, flagnames, flags, ret).show();
	return ret;
}

ChecklistDialog::ChecklistDialog(string const& title,
	vector<string> const& flagnames, bitstring& flags, bool& confirm):
	d_title(title), flagnames(flagnames), flags(flags), confirm(confirm)
{}

std::shared_ptr<GUI::Widget> ChecklistDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	// Aim for columns as evenly sized as possible, up to 10 entries per column.
	size_t per_col = 10;
	{
		auto numflags = flagnames.size();
		size_t best_per_col = 0;
		size_t best_remainder = numflags;
		for (size_t q = vbound(int(numflags/2), 4, 10); q > 3; --q)
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
	}
	auto grid = GUI::Grid::columns(per_col);
	for(uint q = 0; q < flagnames.size(); ++q)
	{
		grid->add(Checkbox(text = flagnames[q], hAlign = 0.0,
			checked = flags.get(q),
			onToggleFunc = [&,q](bool state)
			{
				flags.set(q, state);
			}));
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

