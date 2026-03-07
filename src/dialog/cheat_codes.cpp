#include "cheat_codes.h"
#include "externs.h"
#include <gui/builder.h>

extern ZCHEATS zcheats;
void call_cheats_dlg()
{
	CheatCodesDialog().show();
}

CheatCodesDialog::CheatCodesDialog()
{}

std::shared_ptr<GUI::Widget> CheatCodesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Cheat Codes",
		onClose = message::CANCEL,
		Column(
			Row(
				INFOBTN("If cheats are not enabled, the 'Cheat' menu"
					" will not appear at all, so no codes can be entered."
					"\nWhen using Test Mode to test a quest, cheats are"
					" automatically enabled at level 4 for you, regardless"
					" of the settings here."),
				enabledCB = Checkbox(
					text = "&Enable Cheats",
					checked = zcheats.flags&1)
			),
			Rows<3>(
				Label(text = "Level"),
				Label(text = "Code", colSpan = 2),
				
				Label(text = "1"),
				textFields[0] = TextField(
					maxLength = 40,
					text = zcheats.codes[0],
					focused = true),
				INFOBTN("Refill life, bombs, arrows, money, magic"),
				
				Label(text = "2"),
				textFields[1] = TextField(
					maxLength = 40,
					text = zcheats.codes[1]),
				INFOBTN("Change max life, max bombs, max magic, toggle invincibility"),
				
				Label(text = "3"),
				textFields[2] = TextField(
					maxLength = 40,
					text = zcheats.codes[2]),
				INFOBTN("Edit Hero Data (most Init Data fields are editable, including owned items/counters)"),
				
				Label(text = "4"),
				textFields[3] = TextField(
					maxLength = 40,
					text = zcheats.codes[3]),
				INFOBTN("Full Debug Cheats; warp to locations, toggle layer visibility, show hitboxes, trigger secrets, etc.")
			),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
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
}

bool CheatCodesDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			SETFLAG(zcheats.flags, 1, enabledCB->getChecked());
			for(auto q = 0; q < 4; ++q)
			{
				std::string str(textFields[q]->getText());
				strcpy(zcheats.codes[q], str.c_str());
			}
			return true;
		}

		case message::CANCEL:
			return true;
	}
	return false;
}
