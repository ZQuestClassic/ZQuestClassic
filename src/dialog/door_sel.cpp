#include "door_sel.h"
#include "gui/builder.h"
#include "gui/key.h"
#include "alert.h"
#include "zc_list_data.h"

using std::string;
using std::to_string;

extern bool saved;

static bool edited = false;

bool call_doorseldialog()
{
	edited = false;
	DoorSelDialog().show();
	return true;
}

DoorSelDialog::DoorSelDialog() :
	list_doortypes(GUI::ZCListData::doortypes()),
	list_doorsets(GUI::ZCListData::doorsets())
{}

std::shared_ptr<GUI::Widget> DoorSelDialog::DoorDDL(byte dir)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return DropDownList(
		data = list_doortypes,
		selectedValue = doors[dir],
		fitParent = true,
		width = 6_em,
		onSelectFunc = [&](int32_t val)
		{
			doors[dir] = val;
		}
	);
}

std::shared_ptr<GUI::Widget> DoorSelDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	m = Map.CurrScr();
	memcpy(doors, m->door, 4);
	dcs = m->door_combo_set;
	old_dcs = dcs;

	window = Window(
		title = "Select Door",
		onClose = message::CANCEL,
		Column(
			Label(text = "Note: This only apply to 'NES Dungeon style' screens!", vAlign = 0, topPadding = 0.25_em, bottomPadding = 0.5_em),
			//
			DoorDDL(up),
			//
			Row(DoorDDL(left), DoorDDL(right)),
			//
			DoorDDL(down),
			//
			Label(text = "Door Combo Set", vAlign = 0, topPadding = 0.25_em, bottomPadding = 0.5_em),
			//
			DropDownList(
				data = list_doorsets,
				selectedValue = dcs,
				fitParent = true,
				onSelectFunc = [&](int32_t val) {
					dcs = val; 
					if (dcs != old_dcs)
						edited = true;
				}
			),
			Row(hPadding = 1_em,
				Button(width = 90_px, text = "Ok", onClick = message::OK),
				Button(width = 90_px, text = "Cancel", onClick = message::CANCEL),
				Button(width = 90_px, text = "Clear", onClick = message::CLEAR)
			)
		)
	);

	return window;
}

bool DoorSelDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch (msg.message)
	{
		case message::CLEAR:
		{
			for (int q = 0; q < 4; q++) doors[q] = dNONE;
			rerun_dlg = true;
			return false;
			break;
		}
		case message::OK:
		{
			for (int q = 0; q < 4; q++) {
				if(doors[q] != m->door[q]) {
					edited = true;
					break;
				}
			}
			if (edited)
			{
				for (int q = 0; q < 4; q++)
					m->door[q] = doors[q];
				m->door_combo_set = dcs;
				saved = false;
			}
		}
		[[fallthrough]];
		case message::CANCEL:
		default:
		{
			return true;
			break;
		}
	}
}