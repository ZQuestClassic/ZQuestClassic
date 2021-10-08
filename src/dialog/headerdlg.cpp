#include "headerdlg.h"
#include "info.h"
#include "set_password.h"
#include <gui/builder.h>

extern zquestheader header;
extern bool saved;
void call_header_dlg()
{
	char alphastr[64];
	if ( header.new_version_id_alpha ) { sprintf(alphastr, " (Alpha %d)", header.new_version_id_alpha); }
	else if ( header.new_version_id_beta ) { sprintf(alphastr, " (Beta %d)", header.new_version_id_beta); }
	else if ( header.new_version_id_gamma ) { sprintf(alphastr, " (Gamma %d)", header.new_version_id_gamma); }
	else if ( header.new_version_id_release ) { sprintf(alphastr, " (Release %d)", header.new_version_id_release); }
	char zver_str[256];
	sprintf(zver_str,"%d.%02X (Build %d)%s",header.zelda_version>>8,header.zelda_version&0xFF,header.build,alphastr);
	std::string startvals[5] = { std::string(header.version), std::string(header.minver), std::string(header.title), std::string(header.author), std::to_string(header.quest_number) };
	HeaderDialog(std::string(zver_str), startvals,
		[](std::string_view vals[5])
		{
			saved = false;

			vals[0].copy(header.version, 9);
			vals[1].copy(header.minver, 9);
			vals[2].copy(header.title, 64);
			vals[3].copy(header.author, 64);
			char tmp[8] = {0};
			vals[4].copy(tmp, 7);
			header.quest_number=atoi(tmp);
		}).show();
}

HeaderDialog::HeaderDialog(std::string verstr, std::string initVals[5], std::function<void(std::string_view[5])> setVals):
	setVals(setVals), verstr(verstr)
{
	for (int q = 0; q < 5; ++q)
		vals[q] = initVals[q];
}

std::shared_ptr<GUI::Widget> HeaderDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Header",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			Row(
				hAlign = 0.5,
				Label(text = "Quest Made in ZQ Version:", hAlign = 0.0),
				Label(text = verstr, hAlign = 1.0)
			),
			Rows<2>(
				Column(
					width = 140_spx,
					Rows<3>(
						Button(width = 2_em, text = "?", hAlign = 1.0, onPressFunc = []()
						{
							InfoDialog("Quest Version","The version number of your quest. This is stored in save files, and is used for comparing with 'Min. Ver'").show();
						}),
						Label(text = "Quest Ver:", hAlign = 0.0),
						questRev = TextField(width = 8_em, hAlign = 0.0, maxLength = 9, text = vals[0]),
						Button(width = 2_em, text = "?", hAlign = 1.0, onPressFunc = []()
						{
							InfoDialog("Min Version","If a save file of your quest was saved with a 'Quest Ver' lower than this value, it will not be allowed to load. Useful for preventing loading of saves that would be broken by changes to the quest.").show();
						}),
						Label(text = "Min. Ver:", hAlign = 0.0),
						minRev = TextField(width = 8_em, hAlign = 0.0, maxLength = 9, text = vals[1]),
						DummyWidget(),
						Label(text = "Title:", hAlign = 0.0),
						titlestr = TextField(
							width = 8_em,
							hAlign = 1.0,
							maxLength = 64,
							text = vals[2],
							onValueChanged = message::TITLE
						)
					),
					titleLabel = Label(
						fitParent = true,
						framed = true,
						height = 3_em,
						vPadding = 4_spx,
						leftMargin = 2_em+4_spx,
						text = vals[2],
						textAlign = 1
					)
				),
				Column(
					width = 140_spx,
					Rows<3>(
						Label(text = "Quest Num:", hAlign = 0.0),
						questNum = TextField(width = 8_em, hAlign = 1.0, maxLength = 9, text = vals[4]),
						Button(width = 2_em, text = "?", hAlign = 0.0, onPressFunc = []()
						{
							InfoDialog("Quest Progression Number","This value is used by module-based quests, such as '1st.qst'. Unless you know what you are doing, leave this at '0'!").show();
						}),
						DummyWidget(),
						DummyWidget(),
						Button(width = 2_em, visible = false, hAlign = 0.0), //dummy, for spacing
						Label(text = "Author:", hAlign = 0.0),
						author = TextField(
							width = 8_em,
							hAlign = 1.0,
							maxLength = 64,
							text = vals[3],
							onValueChanged = message::AUTHOR
						),
						DummyWidget()
					),
					authorLabel = Label(
						fitParent = true,
						framed = true,
						height = 3_em,
						vPadding = 4_spx,
						rightMargin = 2_em+4_spx,
						text = vals[3],
						textAlign = 1
					)
				)
			),
			Button(text = "Change Password", onPressFunc = call_password_dlg),
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					focused = true,
					text = "OK",
					onClick = message::OK),
				Button(
					text = "Cancel",
					onClick = message::CANCEL)
			)
		)
	);
}

bool HeaderDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
	case message::TITLE:
	{
		std::string foo;
		foo.assign(titlestr->getText());
		titleLabel->setText(foo);
		return false;
	}
	case message::AUTHOR:
	{
		std::string foo;
		foo.assign(author->getText());
		authorLabel->setText(foo);
		return false;
	}
	case message::OK:
		{
			std::string_view newVals[5] = {
				questRev->getText(), minRev->getText(), titlestr->getText(),
				author->getText(), questNum->getText()
			};
			setVals(newVals);
		}
		return true;

	case message::CANCEL:
	default:
		return true;
	}
}
