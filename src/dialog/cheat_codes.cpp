#include "cheat_codes.h"
#include <gui/builder.h>

extern ZCHEATS zcheats;
extern bool saved;
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
			enabledCB = Checkbox(
				hAlign = 1.0,
				text = "&Enable Cheats",
				checked = zcheats.flags&1),
			Columns<5>(
				Label(text = "Level"),
				Label(text = "1"),
				Label(text = "2"),
				Label(text = "3"),
				Label(text = "4"),

				Label(
					hAlign = 0.05,
					text = "Code"),
				textFields[0] = TextField(
					maxLength = 40,
					text = zcheats.codes[0],
					focused = true),
				textFields[1] = TextField(
					maxLength = 40,
					text = zcheats.codes[1]),
				textFields[2] = TextField(
					maxLength = 40,
					text = zcheats.codes[2]),
				textFields[3] = TextField(
					maxLength=40,
					text = zcheats.codes[3])
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
