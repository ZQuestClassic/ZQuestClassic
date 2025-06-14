#include "dmap_lister.h"
#include "edit_dmap.h"
#include "gui/key.h"
#include "info.h"
#include "alert.h"
#include "gui/builder.h"
#include "zc_list_data.h"
#include <fmt/format.h>
#include <base/dmap.h>
#include <functional>
#include <string_view>

#define buttonwidth 6_em
#define labelwidth 12_em

extern bool saved;

void call_dmaplisterdialog()
{
	DMapListerDialog().show();
}

DMapListerDialog::DMapListerDialog() :
	list_dmaps(GUI::ZCListData::dmaps())
{}

std::shared_ptr<GUI::Widget> DMapListerDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	dmaplist = List(
		data = list_dmaps,
		selectedIndex = 0
	),

	window = Window(
		title = "Select DMap",
		info = "Select a DMap to edit"
		"\nPress 'Del' to delete the selected DMap"
		"\nPress 'C' to copy the selected DMap"
		"\nPress 'V' to paste the copied DMap",
		shortcuts = {
			GUI::Key::Del = message::CLEAR,
			GUI::Key::C = message::COPY,
			GUI::Key::V = message::PASTE
		},
		Column(
			Row(vPadding = 0_px, fitParent = true,
				Checkbox(text = "Alphabetized",
					hAlign = 0.0,
					checked = alphabetized,
					onToggleFunc = [&](bool state)
					{
						alphabetized = state;
						if (alphabetized)
							list_dmaps.alphabetize();
						else
						list_dmaps.valsort();
					})
				),
			dmaplist,
			Row(
				DMapFrame(Dmap = dmaplist->getSelectedIndex()),
				Column(
					l_map = Label(text = fmt::format("Map: {}", DMaps[dmaplist->getSelectedIndex()].map),
						textAlign = 0, width = labelwidth),
					l_level = Label(text = fmt::format("Level: {}", DMaps[dmaplist->getSelectedIndex()].level),
						textAlign = 0, width = labelwidth),
					l_copied = Label(text = fmt::format("Copied DMap: {}", copied, -1 ? ' ' : copied),
						textAlign = 0, width = labelwidth)
				)
			),
			Row(
				Button(text = "Edit", onClick = message::EDIT, width = buttonwidth),
				Button(text = "Done", onClick = message::DONE, width = buttonwidth)
			)
		)
	);

	return window;
}

bool DMapListerDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	int32_t index = dmaplist->getSelectedIndex();
	switch (msg.message)
	{
	case message::CLEAR:
	{
		bool doclear = false;
		AlertDialog("Are you sure?",
			"Do you really want to completely clear this DMap!",
			[&](bool ret, bool)
			{
				doclear = ret;
			}).show();
		if (doclear)
		{
			DMaps[index].clear();
			rerun_dlg = true;
			return true;
		}
	}
	case message::COPY:
	{
		copied = index;
		l_copied->setText(fmt::format("Copied: {}",index));
	}
	case message::PASTE:
	{
		if (copied == -1)
			return false; //nothing to paste
		DMaps[index] = DMaps[copied];
		rerun_dlg = true;
		return true;
	}
	case message::EDIT:
		call_editdmap_dialog(index);
		rerun_dlg = true;
		return true;
	case message::DONE:
		return true;
	}
	return false;
}