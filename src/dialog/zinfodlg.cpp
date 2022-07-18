#include "zinfodlg.h"
#include <gui/builder.h>
#include "../jwin.h"
#include "zquest.h"
#include "dialog/alert.h"
#include "zc_list_data.h"

extern zquestheader header;

void call_zinf_dlg()
{
	ZInfoDialog().show();
}

ZInfoDialog::ZInfoDialog(): lzinfo(),
	list_itemclass(GUI::ZCListData::itemclass(true)),
	list_combotype(GUI::ZCListData::combotype(true, true)),
	list_counters(GUI::ZCListData::counters(true, true)),
	list_mapflag(GUI::ZCListData::mapflag(numericalFlags, true, true))
{
	lzinfo.copyFrom(ZI);
}

static bool extzinf;
static size_t zinftab = 0;
std::shared_ptr<GUI::Widget> ZInfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	static int32_t selic = 0, selct = 0, selmf = 0, selctr = 0;
	static char **icnameptr = nullptr, **ichelpptr = nullptr, **ctnameptr = nullptr,
	            **cthelpptr = nullptr, **mfnameptr = nullptr, **mfhelpptr = nullptr,
				**ctrnameptr = nullptr;
	extzinf = header.external_zinfo;
	icnameptr = &(lzinfo.ic_name[selic]);
	ichelpptr = &(lzinfo.ic_help_string[selic]);
	ctnameptr = &(lzinfo.ctype_name[selct]);
	cthelpptr = &(lzinfo.ctype_help_string[selct]);
	mfnameptr = &(lzinfo.mf_name[selmf]);
	mfhelpptr = &(lzinfo.mf_help_string[selmf]);
	ctrnameptr = &(lzinfo.ctr_name[selctr]);
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZInfo Editor",
		info = "By unchecking 'default' for an itemclass, it will allow entry of a string which will be stored in"
			" the quest's \"zinfo\". If 'External ZInfo' is checked, zinfo will not be saved to the quest file,"
			" but instead to a separate '.zinfo' file. If the external file is not found, it will load default data instead.",
		onEnter = message::OK,
		onClose = message::CANCEL,
		Column(
			TabPanel(ptr = &zinftab,
				TabRef(name = "Itemclass", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_itemclass,
						colSpan = 3, fitParent = true,
						selectedValue = selic,
						onSelectFunc = [&](int32_t val)
						{
							selic = val;
							icnameptr = &(lzinfo.ic_name[selic]);
							ichelpptr = &(lzinfo.ic_help_string[selic]);
							fields[FLD_IC_NAME]->setText((*icnameptr) ? (*icnameptr) : "");
							fields[FLD_IC_NAME]->setDisabled(!(*icnameptr));
							defcheck[FLD_IC_NAME]->setChecked(!(*icnameptr));
							helplbl[LBL_IC_HELP]->setText((*ichelpptr) ? (*ichelpptr) : "");
							fields[FLD_IC_HELP]->setText((*ichelpptr) ? (*ichelpptr) : "");
							fields[FLD_IC_HELP]->setDisabled(!(*ichelpptr));
							defcheck[FLD_IC_HELP]->setChecked(!(*ichelpptr));
						}
					),
					Label(text = "Itemclass Name:"),
					fields[FLD_IC_NAME] = TextField(
						maxLength = 255, text = ((*icnameptr) ? (*icnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(icnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_IC_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(icnameptr, nullptr);
								fields[FLD_IC_NAME]->setText("");
								fields[FLD_IC_NAME]->setDisabled(true);
							}
							else fields[FLD_IC_NAME]->setDisabled(false);
						}),
					Label(text = "Itemclass Help Text:"),
					fields[FLD_IC_HELP] = TextField(
						maxLength = 65535, text = ((*ichelpptr) ? (*ichelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(ichelpptr, str.size() ? str.c_str() : nullptr);
							helplbl[LBL_IC_HELP]->setText(str);
						}),
					defcheck[FLD_IC_HELP] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ichelpptr, nullptr);
								helplbl[LBL_IC_HELP]->setText("");
								fields[FLD_IC_HELP]->setText("");
								fields[FLD_IC_HELP]->setDisabled(true);
							}
							else fields[FLD_IC_HELP]->setDisabled(false);
						}),
					helplbl[LBL_IC_HELP] = Label(noHLine = true,
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
											fields[FLD_IC_NAME]->setText("");
											fields[FLD_IC_NAME]->setDisabled(true);
											defcheck[FLD_IC_NAME]->setChecked(true);
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
											helplbl[LBL_IC_HELP]->setText("");
											fields[FLD_IC_HELP]->setText("");
											fields[FLD_IC_HELP]->setDisabled(true);
											defcheck[FLD_IC_HELP]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Combo Type", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_combotype,
						colSpan = 3, fitParent = true,
						selectedValue = selct,
						onSelectFunc = [&](int32_t val)
						{
							selct = val;
							ctnameptr = &(lzinfo.ctype_name[selct]);
							cthelpptr = &(lzinfo.ctype_help_string[selct]);
							fields[FLD_CT_NAME]->setText((*ctnameptr) ? (*ctnameptr) : "");
							fields[FLD_CT_NAME]->setDisabled(!(*ctnameptr));
							defcheck[FLD_CT_NAME]->setChecked(!(*ctnameptr));
							helplbl[LBL_CT_HELP]->setText((*cthelpptr) ? (*cthelpptr) : "");
							fields[FLD_CT_HELP]->setText((*cthelpptr) ? (*cthelpptr) : "");
							fields[FLD_CT_HELP]->setDisabled(!(*cthelpptr));
							defcheck[FLD_CT_HELP]->setChecked(!(*cthelpptr));
						}
					),
					Label(text = "Combo Type Name:"),
					fields[FLD_CT_NAME] = TextField(
						maxLength = 255, text = ((*ctnameptr) ? (*ctnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(ctnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_CT_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ctnameptr, nullptr);
								fields[FLD_CT_NAME]->setText("");
								fields[FLD_CT_NAME]->setDisabled(true);
							}
							else fields[FLD_CT_NAME]->setDisabled(false);
						}),
					Label(text = "Combo Type Help Text:"),
					fields[FLD_CT_HELP] = TextField(
						maxLength = 65535, text = ((*cthelpptr) ? (*cthelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(cthelpptr, str.size() ? str.c_str() : nullptr);
							helplbl[LBL_CT_HELP]->setText(str);
						}),
					defcheck[FLD_CT_HELP] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(cthelpptr, nullptr);
								helplbl[LBL_CT_HELP]->setText("");
								fields[FLD_CT_HELP]->setText("");
								fields[FLD_CT_HELP]->setDisabled(true);
							}
							else fields[FLD_CT_HELP]->setDisabled(false);
						}),
					helplbl[LBL_CT_HELP] = Label(noHLine = true,
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
											fields[FLD_CT_NAME]->setText("");
											fields[FLD_CT_NAME]->setDisabled(true);
											defcheck[FLD_CT_NAME]->setChecked(true);
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
											helplbl[LBL_CT_HELP]->setText("");
											fields[FLD_CT_HELP]->setText("");
											fields[FLD_CT_HELP]->setDisabled(true);
											defcheck[FLD_CT_HELP]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Mapflags", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_mapflag,
						colSpan = 3, fitParent = true,
						selectedValue = selmf,
						onSelectFunc = [&](int32_t val)
						{
							selmf = val;
							mfnameptr = &(lzinfo.mf_name[selmf]);
							mfhelpptr = &(lzinfo.mf_help_string[selmf]);
							fields[FLD_MF_NAME]->setText((*mfnameptr) ? (*mfnameptr) : "");
							fields[FLD_MF_NAME]->setDisabled(!(*mfnameptr));
							defcheck[FLD_MF_NAME]->setChecked(!(*mfnameptr));;
							helplbl[LBL_MF_HELP]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
							fields[FLD_MF_HELP]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
							fields[FLD_MF_HELP]->setDisabled(!(*mfhelpptr));
							defcheck[FLD_MF_HELP]->setChecked(!(*mfhelpptr));
						}
					),
					Label(text = "Mapflag Name:"),
					fields[FLD_MF_NAME] = TextField(
						maxLength = 255, text = ((*mfnameptr) ? (*mfnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(mfnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_MF_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(mfnameptr, nullptr);
								fields[FLD_MF_NAME]->setText("");
								fields[FLD_MF_NAME]->setDisabled(true);
							}
							else fields[FLD_MF_NAME]->setDisabled(false);
						}),
					Label(text = "Mapflag Help Text:"),
					fields[FLD_MF_HELP] = TextField(
						maxLength = 65535, text = ((*mfhelpptr) ? (*mfhelpptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(mfhelpptr, str.size() ? str.c_str() : nullptr);
							helplbl[LBL_MF_HELP]->setText(str);
						}),
					defcheck[FLD_MF_HELP] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(mfhelpptr, nullptr);
								helplbl[LBL_MF_HELP]->setText("");
								fields[FLD_MF_HELP]->setText("");
								fields[FLD_MF_HELP]->setDisabled(true);
							}
							else fields[FLD_MF_HELP]->setDisabled(false);
						}),
					helplbl[LBL_MF_HELP] = Label(noHLine = true,
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
											fields[FLD_MF_NAME]->setText("");
											fields[FLD_MF_NAME]->setDisabled(true);
											defcheck[FLD_MF_NAME]->setChecked(true);
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
											helplbl[LBL_MF_HELP]->setText("");
											fields[FLD_MF_HELP]->setText("");
											fields[FLD_MF_HELP]->setDisabled(true);
											defcheck[FLD_MF_HELP]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Counters", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_counters,
						colSpan = 3, fitParent = true,
						selectedValue = selctr,
						onSelectFunc = [&](int32_t val)
						{
							selctr = val;
							ctrnameptr = &(lzinfo.ctr_name[selctr]);
							fields[FLD_CTR_NAME]->setText((*ctrnameptr) ? (*ctrnameptr) : "");
							fields[FLD_CTR_NAME]->setDisabled(!(*ctrnameptr));
							defcheck[FLD_CTR_NAME]->setChecked(!(*ctrnameptr));
						}
					),
					Label(text = "Counter Name:"),
					fields[FLD_CTR_NAME] = TextField(
						maxLength = 255, text = ((*ctrnameptr) ? (*ctrnameptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(ctrnameptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_CTR_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(ctrnameptr, nullptr);
								fields[FLD_CTR_NAME]->setText("");
								fields[FLD_CTR_NAME]->setDisabled(true);
							}
							else fields[FLD_CTR_NAME]->setDisabled(false);
						}),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all counter..."),
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
											lzinfo.clear_ctr_name();
											fields[FLD_CTR_NAME]->setText("");
											fields[FLD_CTR_NAME]->setDisabled(true);
											defcheck[FLD_CTR_NAME]->setChecked(true);
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
	
	fields[FLD_IC_NAME]->setText((*icnameptr) ? (*icnameptr) : "");
	fields[FLD_IC_NAME]->setDisabled(!(*icnameptr));
	defcheck[FLD_IC_NAME]->setChecked(!(*icnameptr));
	helplbl[LBL_IC_HELP]->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[FLD_IC_HELP]->setText((*ichelpptr) ? (*ichelpptr) : "");
	fields[FLD_IC_HELP]->setDisabled(!(*ichelpptr));
	defcheck[FLD_IC_HELP]->setChecked(!(*ichelpptr));
	fields[FLD_CT_NAME]->setText((*ctnameptr) ? (*ctnameptr) : "");
	fields[FLD_CT_NAME]->setDisabled(!(*ctnameptr));
	defcheck[FLD_CT_NAME]->setChecked(!(*ctnameptr));
	helplbl[LBL_CT_HELP]->setText((*cthelpptr) ? (*cthelpptr) : "");
	fields[FLD_CT_HELP]->setText((*cthelpptr) ? (*cthelpptr) : "");
	fields[FLD_CT_HELP]->setDisabled(!(*cthelpptr));
	defcheck[FLD_CT_HELP]->setChecked(!(*cthelpptr));
	fields[FLD_MF_NAME]->setText((*mfnameptr) ? (*mfnameptr) : "");
	fields[FLD_MF_NAME]->setDisabled(!(*mfnameptr));
	defcheck[FLD_MF_NAME]->setChecked(!(*mfnameptr));
	helplbl[LBL_MF_HELP]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
	fields[FLD_MF_HELP]->setText((*mfhelpptr) ? (*mfhelpptr) : "");
	fields[FLD_MF_HELP]->setDisabled(!(*mfhelpptr));
	defcheck[FLD_MF_HELP]->setChecked(!(*mfhelpptr));
	fields[FLD_CTR_NAME]->setText((*ctrnameptr) ? (*ctrnameptr) : "");
	fields[FLD_CTR_NAME]->setDisabled(!(*ctrnameptr));
	defcheck[FLD_CTR_NAME]->setChecked(!(*ctrnameptr));
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

