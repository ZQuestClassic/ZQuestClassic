#include "zinfodlg.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "../zquest.h"
#include "dialog/alert.h"

extern zquestheader header;

void call_zinf_dlg()
{
	ZInfoDialog().show();
}

ZInfoDialog::ZInfoDialog(): lzinfo(),
	list_itemclass(GUI::ListData::itemclass(true)),
	list_combotype(GUI::ListData::combotype(true, true)),
	list_mapflag(GUI::ListData::mapflag(true, true))
{
	lzinfo.copyFrom(ZI);
}

static bool extzinf;
std::shared_ptr<GUI::Widget> ZInfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	static int32_t selic = 0, selct = 0, selmf = 0;
	static char **icnameptr = nullptr, **ichelpptr = nullptr, **ctnameptr = nullptr,
	            **cthelpptr = nullptr, **mfnameptr = nullptr, **mfhelpptr = nullptr;
	extzinf = header.external_zinfo;
	icnameptr = &(lzinfo.ic_name[selic]);
	ichelpptr = &(lzinfo.ic_help_string[selic]);
	ctnameptr = &(lzinfo.ctype_name[selct]);
	cthelpptr = &(lzinfo.ctype_help_string[selct]);
	mfnameptr = &(lzinfo.mf_name[selmf]);
	mfhelpptr = &(lzinfo.mf_help_string[selmf]);
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
							helplbl[0]->setText((*ichelpptr) ? (*ichelpptr) : "");
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
							helplbl[0]->setText(str);
						}),
					defcheck[1] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ichelpptr, nullptr);
								helplbl[0]->setText("");
								fields[1]->setText("");
								fields[1]->setDisabled(true);
							}
							else fields[1]->setDisabled(false);
						}),
					helplbl[0] = Label(noHLine = true,
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
								AlertDialog("Are you sure?",
									"This will clear ALL itemclass names to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_ic_name();
											fields[0]->setText("");
											fields[0]->setDisabled(true);
											defcheck[0]->setChecked(true);
										}
									}).show();
							}),
						Button(
							text = "Help Text",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL itemclass helptext to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_ic_help();
											helplbl[0]->setText("");
											fields[1]->setText("");
											fields[1]->setDisabled(true);
											defcheck[1]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Combo Type", Rows<3>(
					DropDownList(data = list_combotype,
						colSpan = 3, fitParent = true,
						selectedValue = selct,
						onSelectFunc = [&](int32_t val)
						{
							selct = val;
							ctnameptr = &(lzinfo.ctype_name[selct]);
							cthelpptr = &(lzinfo.ctype_help_string[selct]);
							fields[2]->setText((*ctnameptr) ? (*ctnameptr) : "");
							fields[2]->setDisabled(!(*ctnameptr));
							defcheck[2]->setChecked(!(*ctnameptr));
							helplbl[1]->setText((*cthelpptr) ? (*cthelpptr) : "");
							fields[3]->setText((*cthelpptr) ? (*cthelpptr) : "");
							fields[3]->setDisabled(!(*cthelpptr));
							defcheck[3]->setChecked(!(*cthelpptr));
						}
					),
					Label(text = "Combo Type Name:"),
					fields[2] = TextField(
						maxLength = 255, text = ((*ctnameptr) ? (*ctnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(ctnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[2] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ctnameptr, nullptr);
								fields[2]->setText("");
								fields[2]->setDisabled(true);
							}
							else fields[2]->setDisabled(false);
						}),
					Label(text = "Combo Type Help Text:"),
					fields[3] = TextField(
						maxLength = 65535, text = ((*cthelpptr) ? (*cthelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(cthelpptr, str.size() ? str.c_str() : nullptr);
							helplbl[1]->setText(str);
						}),
					defcheck[3] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(cthelpptr, nullptr);
								helplbl[1]->setText("");
								fields[3]->setText("");
								fields[3]->setDisabled(true);
							}
							else fields[3]->setDisabled(false);
						}),
					helplbl[1] = Label(noHLine = true,
						colSpan = 3, forceFitW = true,
						framed = true, height = 6_em,
						vPadding = 4_spx, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all combo type..."),
						Button(
							text = "Names",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL combo type names to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_ctype_name();
											fields[2]->setText("");
											fields[2]->setDisabled(true);
											defcheck[2]->setChecked(true);
										}
									}).show();
							}),
						Button(
							text = "Help Text",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL combo type help text to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_ctype_help();
											helplbl[1]->setText("");
											fields[3]->setText("");
											fields[3]->setDisabled(true);
											defcheck[3]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Mapflags", Rows<3>(
					DropDownList(data = list_mapflag,
						colSpan = 3, fitParent = true,
						selectedValue = selic,
						onSelectFunc = [&](int32_t val)
						{
							selmf = val;
							mfnameptr = &(lzinfo.mf_name[selmf]);
							mfhelpptr = &(lzinfo.mf_help_string[selmf]);
							fields[4]->setText((*mfnameptr) ? (*mfnameptr) : "");
							fields[4]->setDisabled(!(*mfnameptr));
							defcheck[4]->setChecked(!(*mfnameptr));;
							helplbl[2]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
							fields[5]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
							fields[5]->setDisabled(!(*mfhelpptr));
							defcheck[5]->setChecked(!(*mfhelpptr));
						}
					),
					Label(text = "Mapflag Name:"),
					fields[4] = TextField(
						maxLength = 255, text = ((*mfnameptr) ? (*mfnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(mfnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[4] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(mfnameptr, nullptr);
								fields[4]->setText("");
								fields[4]->setDisabled(true);
							}
							else fields[4]->setDisabled(false);
						}),
					Label(text = "Mapflag Help Text:"),
					fields[5] = TextField(
						maxLength = 65535, text = ((*mfhelpptr) ? (*mfhelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(mfhelpptr, str.size() ? str.c_str() : nullptr);
							helplbl[2]->setText(str);
						}),
					defcheck[5] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(mfhelpptr, nullptr);
								helplbl[2]->setText("");
								fields[5]->setText("");
								fields[5]->setDisabled(true);
							}
							else fields[5]->setDisabled(false);
						}),
					helplbl[2] = Label(noHLine = true,
						colSpan = 3, forceFitW = true,
						framed = true, height = 6_em,
						vPadding = 4_spx, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all mapflag..."),
						Button(
							text = "Names",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL mapflag names to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_mf_name();
											fields[4]->setText("");
											fields[4]->setDisabled(true);
											defcheck[4]->setChecked(true);
										}
									}).show();
							}),
						Button(
							text = "Help Text",
							minwidth = 40_lpx,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL mapflag help text to default!",
									[&](bool ret)
									{
										if(ret)
										{
											lzinfo.clear_mf_help();
											helplbl[2]->setText("");
											fields[5]->setText("");
											fields[5]->setDisabled(true);
											defcheck[5]->setChecked(true);
										}
									}).show();
							})
					)
				))
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
	helplbl[0]->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[1]->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[1]->setDisabled(!(*ichelpptr));
	defcheck[1]->setChecked(!(*ichelpptr));
	fields[2]->setText((*ctnameptr) ? (*ctnameptr) : "");
	fields[2]->setDisabled(!(*ctnameptr));
	defcheck[2]->setChecked(!(*ctnameptr));
	helplbl[1]->setText((*cthelpptr) ? (*cthelpptr) : "");
	fields[3]->setText((*cthelpptr) ? (*cthelpptr) : "");
	fields[3]->setDisabled(!(*cthelpptr));
	defcheck[3]->setChecked(!(*cthelpptr));
	fields[4]->setText((*mfnameptr) ? (*mfnameptr) : "");
	fields[4]->setDisabled(!(*mfnameptr));
	defcheck[4]->setChecked(!(*mfnameptr));
	helplbl[2]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
	fields[5]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
	fields[5]->setDisabled(!(*mfhelpptr));
	defcheck[5]->setChecked(!(*mfhelpptr));
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

