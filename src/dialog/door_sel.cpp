#include "door_sel.h"
#include "gui/builder.h"
#include "gui/key.h"
#include "alert.h"
#include "zc_list_data.h"
#include "zq/render.h"
#include "zq/zq_misc.h"
#include "zq/zquest.h"

using std::string;
using std::to_string;

extern bool saved;

void set_dlg_active(bool active)
{
	pause_dlg_tint(active);
	zq_set_screen_never_freeze(active);
}
#define DOORSEL_INFOBTN(inf) \
Button(forceFitH = true, text = "?", \
	onClick = message::REFR_INFO, \
	onPressFunc = [=]() \
	{ \
		set_dlg_active(false); \
		displayinfo("Info",inf); \
		set_dlg_active(true); \
	})

bool call_doorseldialog()
{
	set_dlg_active(true);
	Map.StartListCommand();
	DoorSelDialog(*Map.CurrScr()).show();
	set_dlg_active(false);
	return true;
}

DoorSelDialog::DoorSelDialog(mapscr const& ref) :
	screen(ref),
	list_doortypes(GUI::ZCListData::doortypes()),
	list_doorsets(GUI::ZCListData::doorsets())
{
	door_combo_set = (old_door_combo_set = screen.door_combo_set);
	for (int q = 0; q < 4; q++)
		doors[q] = screen.door[q];;
}

void DoorSelDialog::set_doors()
{
	//Recover the combos that were there previously!
	Map.RevokeListCommand();
	Map.StartListCommand();
	Map.DoSetDCSCommand(door_combo_set);
	for (int q = 0; q < 4; q++)
		Map.DoPutDoorCommand(q,doors[q],door_combo_set!=old_door_combo_set);
	refresh(rMAP | rNOCURSOR);
}

std::shared_ptr<GUI::Widget> DoorSelDialog::DoorDDL(byte dir)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return DropDownList(
		data = list_doortypes,
		selectedValue = doors[dir],
		minwidth = 6_em,
		onSelectFunc = [&, dir](int32_t val)
		{
			doors[dir] = val;
			set_doors();
		}
	);
}

std::shared_ptr<GUI::Widget> DoorSelDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using namespace GUI::Key;

	window = Window(
		title = "Select Door",
		onClose = message::CANCEL,
		Column(
			Label(text = "Note: This only applies to 'NES Dungeon style' screens!"),
			Column(
				DoorDDL(up),
				Row(DoorDDL(left), DoorDDL(right)),
				DoorDDL(down)
			),
			Rows<2>(
				Label(text = "Door Combo Set"),
				DropDownList(
					data = list_doorsets,
					selectedValue = door_combo_set,
					fitParent = true,
					onSelectFunc = [&](int32_t val) {
						door_combo_set = val;
						set_doors();
					}
				)
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
			for (int q = 0; q < 4; q++)
				doors[q] = dNONE;
			set_doors();
			rerun_dlg = true;
			return true;
		}
		case message::OK:
		{
			Map.FinishListCommand();
			saved = false;
			return true;
		}
		case message::CANCEL:
		default:
		{
			Map.RevokeListCommand();
			return true;
		}
	}
}

