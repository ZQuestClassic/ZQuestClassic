#include "headerdlg.h"
#include "info.h"
#include "set_password.h"
#include <gui/builder.h>

extern zquestheader header;
extern bool saved;
void call_header_dlg()
{
	char alphastr[64] = {0};
	sprintf(alphastr, " %s", header.getAlphaVerStr());
	char zver_str[256];
	sprintf(zver_str,"%d.%02X (Build %d)%s",header.zelda_version>>8,header.zelda_version&0xFF,header.build,alphastr);
	std::string startvals[5] = { std::string(header.version), std::string(header.minver), std::string(header.title), std::string(header.author), std::to_string(header.quest_number) };
	HeaderDialog(header.getVerStr(), startvals,
		[](std::string_view vals[4])
		{
			saved = false;

			vals[0].copy(header.version, 16);
			header.version[vals[0].size()] = 0;
			vals[1].copy(header.minver, 16);
			header.minver[vals[1].size()] = 0;
			vals[2].copy(header.title, 64);
			header.title[vals[2].size()] = 0;
			vals[3].copy(header.author, 64);
			header.author[vals[3].size()] = 0;
		}).show();
}

HeaderDialog::HeaderDialog(std::string verstr, std::string initVals[4], std::function<void(std::string_view[4])> setVals):
	verstr(verstr), setVals(setVals)
{
	for (int32_t q = 0; q < 4; ++q)
		vals[q] = initVals[q];
}

#define HEADER_TEXTFIELD_WID 9_em

std::shared_ptr<GUI::Widget> HeaderDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;

	return Window(
		title = "Header",
		onClose = message::CANCEL,
		Column(
			Row(
				hAlign = 0.5,
				Label(text = "Quest made in version:", hAlign = 0.0),
				Label(noHLine = true, text = verstr, hAlign = 1.0)
			),
			Rows<2>(
				Column(
					Rows<6>(
						Label(text = "Quest Ver:", rightPadding = 0_px, hAlign = 1.0),
						questRev = TextField(width = HEADER_TEXTFIELD_WID, rightPadding = 0_px, maxLength = 16, text = vals[0]),
						Button(width = 2_em, leftPadding = 0_px, forceFitH = true, text = "?",
							onPressFunc = []()
							{
								InfoDialog("Quest Version","The version number of your quest. This is stored in save files, and is used for comparing with 'Min. Ver'").show();
							}),
						//
						DummyWidget(),
						DummyWidget(),
						DummyWidget(),
						//
						Label(text = "Min. Ver:", rightPadding = 0_px, hAlign = 1.0),
						minRev = TextField(width = HEADER_TEXTFIELD_WID, rightPadding = 0_px, maxLength = 16, text = vals[1]),
						Button(width = 2_em, leftPadding = 0_px, forceFitH = true, text = "?",
							onPressFunc = []()
							{
								InfoDialog("Min Version","If a save file of your quest was saved with a 'Quest Ver' lower than this value, it will not be allowed to load. Useful for preventing loading of saves that would be broken by changes to the quest.").show();
							}),
						//
						DummyWidget(),
						DummyWidget(),
						DummyWidget(),
						//
						Label(text = "Title:", rightPadding = 0_px, hAlign = 1.0),
						titlestr = TextField(
							width = HEADER_TEXTFIELD_WID,
							rightPadding = 0_px,
							maxLength = 64,
							text = vals[2],
							onValueChanged = message::TITLE
						),
						DummyWidget(),
						//
						Label(text = "Author:", rightPadding = 0_px, hAlign = 1.0),
						author = TextField(
							width = HEADER_TEXTFIELD_WID,
							rightPadding = 0_px,
							maxLength = 64,
							text = vals[3],
							onValueChanged = message::AUTHOR
						),
						DummyWidget(),
						//
						titleLabel = Label(noHLine = true,
							colSpan = 3,
							forceFitW = true,
							framed = true,
							height = 3_em,
							vPadding = 6_px,
							leftMargin = DEFAULT_PADDING,
							text = vals[2],
							textAlign = 1
						),
						//
						authorLabel = Label(noHLine = true,
							colSpan = 3,
							forceFitW = true,
							framed = true,
							height = 3_em,
							vPadding = 6_px,
							rightMargin = DEFAULT_PADDING,
							text = vals[3],
							textAlign = 1
						)
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
			std::string_view newVals[4] = {
				questRev->getText(), minRev->getText(), titlestr->getText(),
				author->getText()
			};
			setVals(newVals);
		}
		return true;

	case message::CANCEL:
		return true;
	}
	return false;
}
