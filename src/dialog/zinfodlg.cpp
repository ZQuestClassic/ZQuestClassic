#include "zinfodlg.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"

extern zquestheader header;

void call_zinf_dlg()
{
	ZInfoDialog().show();
}

ZInfoDialog::ZInfoDialog(): lzinfo(),
	list_itemclass(GUI::ListData::itemclass(true))
{
	lzinfo.copyFrom(ZI);
}

static bool extzinf;
std::shared_ptr<GUI::Widget> ZInfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	static int32_t selic = 0;
	static char** icnameptr = nullptr;
	static char** ichelpptr = nullptr;
	extzinf = header.external_zinfo;
	icnameptr = &(lzinfo.ic_name[selic]);
	ichelpptr = &(lzinfo.ic_help_string[selic]);
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZInfo Editor",
		info = "By unchecking 'default' for an itemclass, it will allow entry of a string which will be stored in"
			" the quest's \"zinfo\". If 'External ZInfo' is checked, zinfo will not be saved to the quest file,"
			" but instead to a separate '.zinfo' file. If the external file is not found, it will load default data instead.",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			TabPanel(
				TabRef(name = "Itemclass", Rows<3>(
					DropDownList(data = list_itemclass,
						colSpan = 3, fitParent = true,
						selectedValue = selic,
						onSelectFunc = [&](int32_t val)
						{
							selic = val;
							icnameptr = &(lzinfo.ic_name[selic]);
							ichelpptr = &(lzinfo.ic_help_string[selic]);
							fields[0]->setText((*icnameptr) ? (*icnameptr) : "");
							fields[0]->setDisabled(!(*icnameptr));
							defcheck[0]->setChecked(!(*icnameptr));
							helplbl->setText((*ichelpptr) ? (*ichelpptr) : "");
							fields[1]->setText((*ichelpptr) ? (*ichelpptr) : "");
							fields[1]->setDisabled(!(*ichelpptr));
							defcheck[1]->setChecked(!(*ichelpptr));
						}
					),
					Label(text = "Itemclass Name:"),
					fields[0] = TextField(
						maxLength = 255, text = ((*icnameptr) ? (*icnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(icnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[0] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(icnameptr, nullptr);
								fields[0]->setText("");
								fields[0]->setDisabled(true);
							}
							else fields[0]->setDisabled(false);
						}),
					Label(text = "Itemclass Help Text:"),
					fields[1] = TextField(
						maxLength = 65535, text = ((*ichelpptr) ? (*ichelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(ichelpptr, str.size() ? str.c_str() : nullptr);
							helplbl->setText(str);
						}),
					defcheck[1] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ichelpptr, nullptr);
								helplbl->setText("");
								fields[1]->setText("");
								fields[1]->setDisabled(true);
							}
							else fields[1]->setDisabled(false);
						}),
					helplbl = Label(noHLine = true,
						colSpan = 3, forceFitW = true,
						framed = true, height = 6_em,
						vPadding = 4_spx, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all itemclass..."),
						Button(
							text = "Names",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								lzinfo.clear_ic_name();
								fields[0]->setText("");
								fields[0]->setDisabled(true);
								defcheck[0]->setChecked(true);
							}),
						Button(
							text = "Help Text",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								lzinfo.clear_ic_help();
								helplbl->setText("");
								fields[1]->setText("");
								fields[1]->setDisabled(true);
								defcheck[1]->setChecked(true);
							})
					)
				)),
				TabRef(name = "2", DummyWidget())
			),
			Checkbox(text = "External ZInfo",
				checked = extzinf,
				onToggleFunc = [&](bool state)
				{
					extzinf = state;
				}
			),
			Row(
				topPadding = 0.5_em,
				vAlign = 1.0,
				spacing = 2_em,
				Button(
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL)
			)
		)
	);
	
	fields[0]->setText((*icnameptr) ? (*icnameptr) : "");
	fields[0]->setDisabled(!(*icnameptr));
	defcheck[0]->setChecked(!(*icnameptr));
	helplbl->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[1]->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[1]->setDisabled(!(*ichelpptr));
	defcheck[1]->setChecked(!(*ichelpptr));
	return window;
}

bool ZInfoDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
			ZI.copyFrom(lzinfo);
			header.external_zinfo = extzinf;
			saved = false;
			[[fallthrough]];
		case message::CANCEL:
		default:
			lzinfo.clear(); //ensure memory cleared
			return true;
	}
}

