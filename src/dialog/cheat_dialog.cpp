#include "cheat_dialog.h"
#include "zc/zc_sys.h"
#include "base/zsys.h"
#include <gui/builder.h>

extern ZCHEATS zcheats;
extern int32_t cheat, maxcheat;
extern bool get_debug();
void call_setcheat_dialog()
{
	if(cheat < 1 && maxcheat < 1 && !zcheats.flags && !get_debug() && DEVLEVEL < 2)
		return;
	SetCheatDialog().show();
}

SetCheatDialog::SetCheatDialog()
{}

#define CHEATRADIO(ind) \
radios[ind] = Radio( \
	checked = (ind==cheat),	\
	disabled = (ind > maxcheat), \
	text = std::to_string(ind), \
	indx = ind \
)

std::shared_ptr<GUI::Widget> SetCheatDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using GUI::Props::indx;

	return Window(
		title = "Cheats",
		onClose = message::CANCEL,
		Column(
			Row(padding = 0_px,
				Rows<2>(padding = 0_px,
					field = TextField(maxLength = 40, focused = true),
					Button(text = "Check", onClick = message::CHECK),
					errlabel = Label(colSpan = 2, textAlign = 1, fitParent = true, text = "")
				),
				Column(
					CHEATRADIO(0),
					CHEATRADIO(1),
					CHEATRADIO(2),
					CHEATRADIO(3),
					CHEATRADIO(4)
				)
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

bool SetCheatDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::CHECK:
		{
			std::string code;
			code.assign(field->getText());
			if(!code.size())
			{
				errlabel->setText("");
				return false;
			}
			int32_t found = 0;
			for(auto q = 0; q < 4; ++q)
			{
				if(!strcmp(code.c_str(), zcheats.codes[q]))
				{
					if(q+1 > maxcheat) maxcheat = q+1;
					for(auto r = zc_min(maxcheat,4); r > 0; --r)
					{
						radios[r]->setDisabled(false);
					}
					found = q+1;
					break;
				}
			}
			if(found)
			{
				field->setText("");
				char buf[80];
				sprintf(buf, "Cheat level %d unlocked!",found);
				errlabel->setText(buf);
				for(auto q = 0; q < 5; ++q)
					radios[q]->setChecked(q==found);
			}
			else
				errlabel->setText("Invalid code!");
			return false;
		}
		case message::OK:
			for(auto q = 0; q < 5; ++q)
			{
				if(radios[q]->getChecked())
				{
					cheat = q;
					if(cheat > maxcheat) cheat = maxcheat;
					break;
				}
			}
			return true;

		case message::CANCEL:
		default:
			return true;
	}
}
