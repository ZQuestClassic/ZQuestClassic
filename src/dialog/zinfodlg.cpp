#include "zinfodlg.h"
#include <gui/builder.h>
#include <base/gui.h>
#include "base/files.h"
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "dialog/alert.h"
#include "zc_list_data.h"
#include "zinfo.h"

extern zquestheader header;

void call_zinf_dlg()
{
	ZInfoDialog().show();
}

ZInfoDialog::ZInfoDialog() : lzinfo(),
	list_itemclass(GUI::ZCListData::itemclass(true)),
	list_combotype(GUI::ZCListData::combotype(true, true)),
	list_mapflag(GUI::ZCListData::mapflag(numericalFlags, true, true)),
	list_counters(GUI::ZCListData::counters(true, true)),
	list_weapon(GUI::ZCListData::weaptypes(true)),
	list_efamilies(GUI::ZCListData::efamilies(true)),
	list_level_items(GUI::ZCListData::level_items(true, true))
{}

static bool extzinf;
static size_t zinftab = 0;
static zinfo tmp_zinfo;
static bool loaded_zi = false;
std::shared_ptr<GUI::Widget> ZInfoDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	if(loaded_zi)
	{
		lzinfo.copyFrom(tmp_zinfo);
		loaded_zi = false;
		tmp_zinfo.clear();
	}
	else lzinfo.copyFrom(ZI); //Load ZInfo Data
	
	static int32_t selic = 0, selct = 0, selmf = 0, selctr = 0, selwpn = 0, seletype = 0, sel_li;
	static char **icnameptr = nullptr, **ichelpptr = nullptr, **ctnameptr = nullptr,
	            **cthelpptr = nullptr, **mfnameptr = nullptr, **mfhelpptr = nullptr,
				**ctrnameptr = nullptr, **wpnptr = nullptr,  **etypeptr = nullptr,
				**li_name_ptr = nullptr, **li_abbr_ptr = nullptr, **li_help_string_ptr = nullptr;
	extzinf = header.external_zinfo;
	icnameptr = &(lzinfo.ic_name[selic]);
	ichelpptr = &(lzinfo.ic_help_string[selic]);
	ctnameptr = &(lzinfo.ctype_name[selct]);
	cthelpptr = &(lzinfo.ctype_help_string[selct]);
	mfnameptr = &(lzinfo.mf_name[selmf]);
	mfhelpptr = &(lzinfo.mf_help_string[selmf]);
	ctrnameptr = &(lzinfo.ctr_name[selctr]);
	wpnptr = &(lzinfo.weap_name[selwpn]);
	etypeptr = &(lzinfo.etype_name[seletype]);
	li_name_ptr = &(lzinfo.litem_name[sel_li]);
	li_abbr_ptr = &(lzinfo.litem_abbr[sel_li]);
	li_help_string_ptr = &(lzinfo.litem_help_string[sel_li]);
	std::shared_ptr<GUI::Window> window = Window(
		title = "ZInfo Editor",
		info = "By unchecking 'default' for a type, it will allow entry of a string which will be stored in"
			" the quest's \"zinfo\". If 'External ZInfo' is checked, zinfo will not be saved to the quest file,"
			" but instead to a separate '.zinfo' file. If the external file is not found, it will load default data instead.",
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
						maxwidth = 20_em,
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
						maxwidth = 20_em,
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
						vPadding = 6_px, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all itemclass..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL itemclass names to default!",
									[&](bool ret,bool)
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
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL itemclass helptext to default!",
									[&](bool ret,bool)
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
						maxwidth = 20_em,
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
						maxwidth = 20_em,
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
						vPadding = 6_px, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all combo type..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL combo type names to default!",
									[&](bool ret,bool)
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
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL combo type help text to default!",
									[&](bool ret,bool)
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
						maxwidth = 20_em,
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
						maxwidth = 20_em,
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
						vPadding = 6_px, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all mapflag..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL mapflag names to default!",
									[&](bool ret,bool)
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
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL mapflag help text to default!",
									[&](bool ret,bool)
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
				TabRef(name = "Weapons", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_weapon,
						colSpan = 3, fitParent = true,
						selectedValue = selwpn,
						onSelectFunc = [&](int32_t val)
						{
							selwpn = val;
							wpnptr = &(lzinfo.weap_name[selwpn]);
							fields[FLD_WPN_NAME]->setText((*wpnptr) ? (*wpnptr) : "");
							fields[FLD_WPN_NAME]->setDisabled(!(*wpnptr));
							defcheck[FLD_WPN_NAME]->setChecked(!(*wpnptr));
						}
					),
					Label(text = "Weapon Name:"),
					fields[FLD_WPN_NAME] = TextField(
						maxwidth = 20_em,
						maxLength = 255, text = ((*wpnptr) ? (*wpnptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(wpnptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_WPN_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(wpnptr, nullptr);
								fields[FLD_WPN_NAME]->setText("");
								fields[FLD_WPN_NAME]->setDisabled(true);
							}
							else fields[FLD_WPN_NAME]->setDisabled(false);
						}),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all weapon..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL weapon names to default!",
									[&](bool ret,bool)
									{
										if(ret)
										{
											lzinfo.clear_weap_name();
											fields[FLD_WPN_NAME]->setText("");
											fields[FLD_WPN_NAME]->setDisabled(true);
											defcheck[FLD_WPN_NAME]->setChecked(true);
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
						maxwidth = 20_em,
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
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL Counter names to default!",
									[&](bool ret,bool)
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
				)),
				TabRef(name = "Enemy Types", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_efamilies,
						colSpan = 3, fitParent = true,
						selectedValue = seletype,
						onSelectFunc = [&](int32_t val)
						{
							seletype = val;
							etypeptr = &(lzinfo.etype_name[seletype]);
							fields[FLD_ETYPE_NAME]->setText((*etypeptr) ? (*etypeptr) : "");
							fields[FLD_ETYPE_NAME]->setDisabled(!(*etypeptr));
							defcheck[FLD_ETYPE_NAME]->setChecked(!(*etypeptr));
						}
					),
					Label(text = "Enemy Type Name:"),
					fields[FLD_ETYPE_NAME] = TextField(
						maxwidth = 20_em,
						maxLength = 255, text = ((*etypeptr) ? (*etypeptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(etypeptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_ETYPE_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(etypeptr, nullptr);
								fields[FLD_ETYPE_NAME]->setText("");
								fields[FLD_ETYPE_NAME]->setDisabled(true);
							}
							else fields[FLD_ETYPE_NAME]->setDisabled(false);
						}),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all enemy types..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL enemy type names to default!",
									[&](bool ret,bool)
									{
										if(ret)
										{
											lzinfo.clear_etype_name();
											fields[FLD_ETYPE_NAME]->setText("");
											fields[FLD_ETYPE_NAME]->setDisabled(true);
											defcheck[FLD_ETYPE_NAME]->setChecked(true);
										}
									}).show();
							})
					)
				)),
				TabRef(name = "Level Items", Rows<3>(
					vAlign = 0.0,
					DropDownList(data = list_level_items,
						colSpan = 3, fitParent = true,
						selectedValue = sel_li,
						onSelectFunc = [&](int32_t val)
						{
							sel_li = val;
							li_name_ptr = &(lzinfo.litem_name[sel_li]);
							li_abbr_ptr = &(lzinfo.litem_abbr[sel_li]);
							li_help_string_ptr = &(lzinfo.litem_help_string[sel_li]);
							fields[FLD_LI_NAME]->setText((*li_name_ptr) ? (*li_name_ptr) : "");
							fields[FLD_LI_NAME]->setDisabled(!(*li_name_ptr));
							defcheck[FLD_LI_NAME]->setChecked(!(*li_name_ptr));
							fields[FLD_LI_ABBR]->setText((*li_abbr_ptr) ? (*li_abbr_ptr) : "");
							fields[FLD_LI_ABBR]->setDisabled(!(*li_abbr_ptr));
							defcheck[FLD_LI_ABBR]->setChecked(!(*li_abbr_ptr));
							helplbl[LBL_LI_HELP]->setText((*li_help_string_ptr) ? (*li_help_string_ptr) : "");
							fields[FLD_LI_HELP]->setText((*li_help_string_ptr) ? (*li_help_string_ptr) : "");
							fields[FLD_LI_HELP]->setDisabled(!(*li_help_string_ptr));
							defcheck[FLD_LI_HELP]->setChecked(!(*li_help_string_ptr));
						}
					),
					Label(text = "Level Item Name:"),
					fields[FLD_LI_NAME] = TextField(
						maxwidth = 20_em,
						maxLength = 255, text = ((*li_name_ptr) ? (*li_name_ptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(li_name_ptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_LI_NAME] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(li_name_ptr, nullptr);
								fields[FLD_LI_NAME]->setText("");
								fields[FLD_LI_NAME]->setDisabled(true);
							}
							else fields[FLD_LI_NAME]->setDisabled(false);
						}),
					Label(text = "Level Item Abbreviation:"),
					fields[FLD_LI_ABBR] = TextField(
						maxwidth = 20_em,
						maxLength = 4, text = ((*li_abbr_ptr) ? (*li_abbr_ptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(li_abbr_ptr, str.size() ? str.c_str() : nullptr);
						}),
					defcheck[FLD_LI_ABBR] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(li_abbr_ptr, nullptr);
								fields[FLD_LI_ABBR]->setText("");
								fields[FLD_LI_ABBR]->setDisabled(true);
							}
							else fields[FLD_LI_ABBR]->setDisabled(false);
						}),
					Label(text = "Level Item Help Text:"),
					fields[FLD_LI_HELP] = TextField(
						maxwidth = 20_em,
						maxLength = 65535, text = ((*li_help_string_ptr) ? (*li_help_string_ptr) : ""),
						onValChangedFunc = [&](GUI::TextField::type,std::string_view sv,int32_t)
						{
							std::string str(sv);
							assignchar(li_help_string_ptr, str.size() ? str.c_str() : nullptr);
							helplbl[LBL_LI_HELP]->setText(str);
						}),
					defcheck[FLD_LI_HELP] = Checkbox(text = "Default",
						onToggleFunc = [&](bool state)
						{
							if(state)
							{
								assignchar(li_help_string_ptr, nullptr);
								helplbl[LBL_LI_HELP]->setText("");
								fields[FLD_LI_HELP]->setText("");
								fields[FLD_LI_HELP]->setDisabled(true);
							}
							else fields[FLD_LI_HELP]->setDisabled(false);
						}),
					helplbl[LBL_LI_HELP] = Label(noHLine = true,
						colSpan = 3, forceFitW = true,
						framed = true, height = 6_em,
						vPadding = 6_px, textAlign = 1
					),
					Row(
						padding = 0_px,
						colSpan = 3,
						Label(text = "Reset all level item..."),
						Button(
							text = "Names",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL level item names to default!",
									[&](bool ret,bool)
									{
										if(ret)
										{
											lzinfo.clear_li_name();
											fields[FLD_LI_NAME]->setText("");
											fields[FLD_LI_NAME]->setDisabled(true);
											defcheck[FLD_LI_NAME]->setChecked(true);
										}
									}).show();
							}),
						Button(
							text = "Abbreviations",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL level item abbreviations to default!",
									[&](bool ret,bool)
									{
										if(ret)
										{
											lzinfo.clear_li_abbr();
											fields[FLD_LI_ABBR]->setText("");
											fields[FLD_LI_ABBR]->setDisabled(true);
											defcheck[FLD_LI_ABBR]->setChecked(true);
										}
									}).show();
							}),
						Button(
							text = "Help Text",
							minwidth = 40_px,
							onPressFunc = [&]()
							{
								AlertDialog("Are you sure?",
									"This will clear ALL level item helptext to default!",
									[&](bool ret,bool)
									{
										if(ret)
										{
											lzinfo.clear_li_help();
											helplbl[LBL_LI_HELP]->setText("");
											fields[FLD_LI_HELP]->setText("");
											fields[FLD_LI_HELP]->setDisabled(true);
											defcheck[FLD_LI_HELP]->setChecked(true);
										}
									}).show();
							})
					)
				))
			),
			Row(padding = 0_px,
				Checkbox(text = "External ZInfo",
					checked = extzinf,
					onToggleFunc = [&](bool state)
					{
						extzinf = state;
					}
				),
				Button(
					text = "Save ZInfo",
					minwidth = 40_px,
					onClick = message::SAVE),
				Button(
					text = "Load ZInfo",
					minwidth = 40_px,
					onClick = message::LOAD)
			),
			Row(
				topPadding = 0.5_em,
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
	fields[FLD_WPN_NAME]->setText((*wpnptr) ? (*wpnptr) : "");
	fields[FLD_WPN_NAME]->setDisabled(!(*wpnptr));
	defcheck[FLD_WPN_NAME]->setChecked(!(*wpnptr));
	fields[FLD_ETYPE_NAME]->setText((*etypeptr) ? (*etypeptr) : "");
	fields[FLD_ETYPE_NAME]->setDisabled(!(*etypeptr));
	defcheck[FLD_ETYPE_NAME]->setChecked(!(*etypeptr));
	
	fields[FLD_LI_NAME]->setText((*li_name_ptr) ? (*li_name_ptr) : "");
	fields[FLD_LI_NAME]->setDisabled(!(*li_name_ptr));
	defcheck[FLD_LI_NAME]->setChecked(!(*li_name_ptr));
	fields[FLD_LI_ABBR]->setText((*li_abbr_ptr) ? (*li_abbr_ptr) : "");
	fields[FLD_LI_ABBR]->setDisabled(!(*li_abbr_ptr));
	defcheck[FLD_LI_ABBR]->setChecked(!(*li_abbr_ptr));
	helplbl[LBL_LI_HELP]->setText((*li_help_string_ptr) ? (*li_help_string_ptr) : "");
	fields[FLD_LI_HELP]->setText((*li_help_string_ptr) ? (*li_help_string_ptr) : "");
	fields[FLD_LI_HELP]->setDisabled(!(*li_help_string_ptr));
	defcheck[FLD_LI_HELP]->setChecked(!(*li_help_string_ptr));
	return window;
}

bool load_zi(zinfo& tzi)
{
	static EXT_LIST extlist[] =
	{
		{ "ZInfo Files (*.zinfo)", "zinfo" },
		// { "Quest Files (*.qst)", "qst" }, //! Maybe todo? bleh
		{ NULL, NULL }
	};
	
	if(!prompt_for_existing_file_compat("Load File","",extlist,filepath,true))
		return false;
	PACKFILE *inf=pack_fopen_password(temppath, F_READ, "");
	if(!inf) return false;
	bool fail = readzinfo(inf, tzi, header)!=0;
	pack_fclose(inf);
	return !fail;
}

bool save_zi(zinfo const& tzi)
{
	if(!prompt_for_new_file_compat("Save ZInfo (.zinfo)","zinfo",NULL,filepath,true))
        return false;
        
    if(exists(temppath))
    {
        if(jwin_alert("Confirm Overwrite",temppath,"already exists.","Write over existing file?","&Yes","&No",'y','n',get_zc_font(font_lfont))==2)
        {
            return false;
        }
    }
    
	PACKFILE *inf = pack_fopen_password(temppath, F_WRITE, "");
	if(!inf) return false;
	if(writezinfo(inf,tzi)!=0)
	{
		pack_fclose(inf);
		return false;
	}
	pack_fclose(inf);
	return true;
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
			lzinfo.clear(); //ensure memory cleared
			return true;
		case message::SAVE:
		{
			save_zi(lzinfo);
			return false;
		}
		case message::LOAD:
		{
			bool r = false;
			AlertDialog("Are you sure?",
				"This will overwrite all info fields!",
				[&](bool ret,bool)
				{
					r = ret;
				}).show();
			if(r)
			{
				if(load_zi(tmp_zinfo))
				{
					loaded_zi = true;
					rerun_dlg = true;
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

