#include "cheat_codes.h"
#include <gui/builder.h>

CheatCodesDialog::CheatCodesDialog(bool enabled,
	std::string_view oldCodes[4],
	std::function<void(bool, std::string_view[4])> setCheatCodes):
		enabled(enabled),
		oldCodes { oldCodes[0], oldCodes[1], oldCodes[2], oldCodes[3] },
		setCheatCodes(setCheatCodes)
{}

std::shared_ptr<GUI::Widget> CheatCodesDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title="Cheat Codes",
		onEnter=message::OK,
		onClose=message::CANCEL,
		Column(
			this->enabledCB=Checkbox(
				hAlign=1.0,
				text="&Enable Cheats",
				checked=this->enabled),
			Columns<5>(
				Label(text="Level"),
				Label(text="1"),
				Label(text="2"),
				Label(text="3"),
				Label(text="4"),

				Label(
					hAlign=0.05,
					text="Code"),
				this->textFields[0]=TextField(
					maxLength=40,
					text=this->oldCodes[0],
					focused=true),
				this->textFields[1]=TextField(
					maxLength=40,
					text=this->oldCodes[1]),
				this->textFields[2]=TextField(
					maxLength=40,
					text=this->oldCodes[2]),
				this->textFields[3]=TextField(
					maxLength=40,
					text=this->oldCodes[3])
			),
			Row(
				vAlign=1.0,
				Button(
					text="OK",
					onClick=message::OK),
				Button(
					text="Cancel",
					onClick=message::CANCEL)
			)
		)
	);
}

bool CheatCodesDialog::handleMessage(message msg)
{
	switch(msg)
	{
	case message::OK:
		{
			std::string_view newCodes[4]={
				textFields[0]->getText(), textFields[1]->getText(),
				textFields[2]->getText(), textFields[3]->getText()
			};
			setCheatCodes(enabledCB->getChecked(), newCodes);
		}
		return true;

	case message::CANCEL:
	default:
		return true;
	}
}
