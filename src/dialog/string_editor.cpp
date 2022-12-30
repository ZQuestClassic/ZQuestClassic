#include "string_editor.h"
#include "zc_sys.h"
#include "base/zsys.h"
#include "editbox.h"
#include "EditboxNew.h"
#include "info.h"
#include "subscr.h"
#include <gui/builder.h>
#include "zc_list_data.h"

extern bool saved;
extern MsgStr* MsgStrings;
extern char msgbuf[MSGBUF_SIZE];
extern char namebuf[9];
extern word msg_count;
extern DIALOG editmsg_help_dlg[];
extern std::map<int32_t, int32_t> msglistcache;
extern std::string helpstr;

std::string run_scc_dlg(MsgStr const* ref);
char* encode_msg_str(std::string const& message);
std::string parse_msg_str(std::string const& s);
void strip_trailing_spaces(char *str);
void strip_trailing_spaces(std::string& str);
int32_t addtomsglist(int32_t index, bool allow_numerical_sort = true);
int32_t msg_at_pos(int32_t pos);
const char *msgslist(int32_t index, int32_t *list_size);

void call_stringedit_dialog(size_t ind, int32_t templateID, int32_t addAfter)
{
	StringEditorDialog(ind, templateID, addAfter).show();
}

extern const char *msgfont_str[font_max];
extern const char *shadowstyle_str[sstsMAX];

GUI::ListData createShadowTypesListData()
{
	std::vector<std::string> strings;

	for(auto q = 0; q < sstsMAX; ++q)
	{
		strings.push_back(shadowstyle_str[q]);
	}

	return GUI::ListData(strings);
}

StringEditorDialog::StringEditorDialog(size_t ind, int32_t templateID, int32_t addAfter)
	: strIndex(ind), tmpMsgStr(MsgStrings[ind]),
	list_font(GUI::ZCListData::fonts()),
	list_nextstr(GUI::ListData::nullData()),
	list_shtype(createShadowTypesListData()),
	addAfter(addAfter)
{
	::ListData msgs_list(msgslist, &font);
	list_nextstr = GUI::ListData(msgs_list, 0);
	
	if(ind == msg_count) //new str
	{
		if(templateID > 0 && templateID < msg_count)
			tmpMsgStr.copyStyle(MsgStrings[templateID]);
		tmpMsgStr.s = "";
	}
}

//{ Macros
#define NUM_FIELD(member,_min,_max) \
TextField( \
	type = GUI::TextField::type::INT_DECIMAL, fitParent = true, \
	low = _min, high = _max, val = tmpMsgStr.member, \
	onValChangedFunc = [&](GUI::TextField::type,std::string_view,int32_t val) \
	{ \
		tmpMsgStr.member = val; \
	})

#define CHECKB(member, bit, txt) \
Checkbox( \
	fitParent = true, text = txt, \
	checked = (tmpMsgStr.member & bit), \
	onToggleFunc = [&](bool state) \
	{ \
		SETFLAG(tmpMsgStr.member,bit,state); \
	} \
) \

#define DDL(member, lister) \
DropDownList(data = lister, \
	fitParent = true, \
	selectedValue = tmpMsgStr.member, \
	onSelectFunc = [&](int32_t val) \
	{ \
		tmpMsgStr.member = val; \
	} \
)

#define COLOR_FIELD(member) \
ColorSel(fitParent = true, \
	val = tmpMsgStr.member, \
	onValChangedFunc = [&](byte val) \
	{ \
		tmpMsgStr.member = val; \
	})

#define TILE_FIELD(memTile, memCSet) \
SelTileSwatch( \
	tile = tmpMsgStr.memTile, \
	cset = (tmpMsgStr.memCSet & 0x0F), \
	onSelectFunc = [&](int32_t t, int32_t c, int32_t,int32_t) \
	{ \
		tmpMsgStr.memTile = t; \
		tmpMsgStr.memCSet &= 0xF0; \
		tmpMsgStr.memCSet |= c&0x0F; \
	})

#define IBTN(info) \
Button(forceFitH = true, text = "?", \
	disabled = !(info)[0], \
	onPressFunc = [&]() \
	{ \
		InfoDialog("Info",info).show(); \
	})
//}

static size_t stred_tab_1 = 0;
static int32_t scroll_pos1 = 0;
std::shared_ptr<GUI::Widget> StringEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using GUI::Props::indx;
	
	char* start_text = encode_msg_str(tmpMsgStr.s);
	std::shared_ptr<GUI::TabPanel> tpan;
	if(is_large)
	{
		tpan = TabPanel(ptr = &stred_tab_1,
			TabRef(name = "String", Column(
				str_field = TextField(fitParent = true,
					forceFitW = true, height = 2_px+(3*(1_em+2_px)),
					maxLength = MSGBUF_SIZE, type = GUI::TextField::type::TEXT,
					text = start_text, focused = !stred_tab_1,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view v,int32_t)
					{
						std::string foo;
						foo.assign(v);
						preview->setText(foo);
						tmpMsgStr.s = parse_msg_str(foo);
					}
				),
				preview = MsgPreview(data = &tmpMsgStr, indx = strIndex, text = start_text),
				Row(padding = 0_px,
					Label(text = "Player Name Preview:"),
					TextField(
						type = GUI::TextField::type::TEXT, text = namebuf,
						maxLength = 8,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view v,int32_t)
						{
							std::string foo;
							foo.assign(v);
							strncpy(namebuf, foo.c_str(), 8);
						}
					),
					Button(
						text = "Insert SCC", forceFitH = true,
						onPressFunc = [&]()
						{
							std::string scc = run_scc_dlg(&tmpMsgStr);
							if(scc.empty())
								return;
							std::string fullstr;
							fullstr.assign(str_field->getText());
							size_t pos = str_field->get_str_pos();
							std::string outstr = fullstr.substr(0,pos) + scc + fullstr.substr(pos);
							str_field->setText(outstr);
							preview->setText(outstr);
							tmpMsgStr.s = parse_msg_str(outstr);
						}),
					Button(text = "Copy Text",
						forceFitH = true,
						onPressFunc = [&]()
						{
							set_al_clipboard(preview->getText());
						}),
					Button(text = "Paste Text",
						forceFitH = true,
						onPressFunc = [&]()
						{
							std::string tmp;
							if(get_al_clipboard(tmp))
							{
								str_field->setText(tmp);
								preview->setText(tmp);
								tmpMsgStr.s = parse_msg_str(tmp);
							}
							else InfoDialog("Error","No text found on clipboard").show();
						})
				),
				Rows<2>(
					DDL(font, list_font),
					DummyWidget(),
					nextstr_dd = DropDownList(data = list_nextstr,
						fitParent = true,
						selectedValue = MsgStrings[tmpMsgStr.nextstring].listpos,
						onSelectFunc = [&](int32_t val)
						{
							tmpMsgStr.nextstring = addtomsglist(val);
						}
					),
					Button(text = "Next in List",
						forceFitH = true,
						onPressFunc = [&]()
						{
							if(tmpMsgStr.listpos+1 < msg_count)
							{
								tmpMsgStr.nextstring = tmpMsgStr.listpos + 1;
								nextstr_dd->setSelectedValue(tmpMsgStr.nextstring);
							}
						})
				)
			)),
			TabRef(name = "Attributes", Column(
				Columns<4>(
					//Col 1
					Label(text = "X:", hAlign = 1.0),
					Label(text = "Y:", hAlign = 1.0),
					Label(text = "SFX:", hAlign = 1.0),
					Label(text = "Layer:", hAlign = 1.0),
					NUM_FIELD(x,-512,512),
					NUM_FIELD(y,-512,512),
					NUM_FIELD(sfx,0,255),
					NUM_FIELD(drawlayer,0,7),
					//Col 2
					Label(text = "Width:", hAlign = 1.0),
					Label(text = "Height:", hAlign = 1.0),
					Label(text = "HSpace:", hAlign = 1.0),
					Label(text = "VSpace:", hAlign = 1.0),
					NUM_FIELD(w,8,512),
					NUM_FIELD(h,8,512),
					NUM_FIELD(hspace,0,128),
					NUM_FIELD(vspace,0,128),
					//Col 3
					Label(text = "T. Margin:", hAlign = 1.0),
					Label(text = "B. Margin:", hAlign = 1.0),
					Label(text = "L. Margin:", hAlign = 1.0),
					Label(text = "R. Margin:", hAlign = 1.0),
					NUM_FIELD(margins[0],0,255),
					NUM_FIELD(margins[1],0,255),
					NUM_FIELD(margins[2],0,255),
					NUM_FIELD(margins[3],0,255)
				),
				Row(
					Rows<2>(
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_CONT,
							"Is continuation of previous string"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_WRAP,
							"Text wraps around bounding box"),
						IBTN("The background tile will use a full rectangle"
							" instead of a frame if this is checked."),
						CHECKB(stringflags, STRINGFLAG_FULLTILE,
							"Full Tiled Background"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_TRANS_BG,
							"Transparent BG"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_TRANS_FG,
							"Transparent FG")
					),
					Rows<3>(
						DummyWidget(),
						Label(text = "Shadow Type", hAlign = 1.0),
						DDL(shadow_type, list_shtype),
						DummyWidget(),
						Label(text = "Shadow Color", hAlign = 1.0),
						COLOR_FIELD(shadow_color),
						IBTN("The top-left tile of the background. If 'Full Tiled Background'"
							" is enabled, a rectangle of tiles the size of the message box is used."
							" Otherwise, a 2x2 block is treated as a 'frame'."),
						Label(text = "Background:", hAlign = 1.0),
						TILE_FIELD(tile, cset)
					)
				)
			)),
			TabRef(name = "Portrait", Column(
				Columns<2>(
					Label(text = "X:", hAlign = 1.0),
					Label(text = "Y:", hAlign = 1.0),
					NUM_FIELD(portrait_x,0,255),
					NUM_FIELD(portrait_y,0,255),
					Label(text = "Tile Width:", hAlign = 1.0),
					Label(text = "Tile Height:", hAlign = 1.0),
					NUM_FIELD(portrait_tw,0,16),
					NUM_FIELD(portrait_th,0,14)
				),
				Row(
					Label(text = "Portrait Tile:", hAlign = 1.0),
					TILE_FIELD(portrait_tile, portrait_cset)
				)
			))
		);
	}
	else
	{
		tpan = TabPanel(ptr = &stred_tab_1,
			TabRef(name = "String", Column(
				str_field = TextField(fitParent = true,
					forceFitW = true, height = 2_px+(3*(1_em+2_px)),
					maxLength = MSGBUF_SIZE, type = GUI::TextField::type::TEXT,
					text = start_text, focused = !stred_tab_1,
					onValChangedFunc = [&](GUI::TextField::type,std::string_view v,int32_t)
					{
						std::string foo;
						foo.assign(v);
						preview->setText(foo);
						tmpMsgStr.s = parse_msg_str(foo);
					}
				),
				ScrollingPane(
					ptr = &scroll_pos1,
					height = 48_px, padding = 0_px,
					preview = MsgPreview(data = &tmpMsgStr, indx = strIndex, text = start_text)
				),
				Row(padding = 0_px,
					Label(text = "Player Name Preview:"),
					TextField(
						type = GUI::TextField::type::TEXT, text = namebuf,
						maxLength = 8,
						onValChangedFunc = [&](GUI::TextField::type,std::string_view v,int32_t)
						{
							std::string foo;
							foo.assign(v);
							strncpy(namebuf, foo.c_str(), 8);
						}
					),
					Button(
						text = "Insert SCC", forceFitH = true,
						onPressFunc = [&]()
						{
							std::string scc = run_scc_dlg(&tmpMsgStr);
							if(scc.empty())
								return;
							std::string fullstr;
							fullstr.assign(str_field->getText());
							size_t pos = str_field->get_str_pos();
							std::string outstr = fullstr.substr(0,pos) + scc + fullstr.substr(pos);
							str_field->setText(outstr);
							preview->setText(outstr);
							tmpMsgStr.s = parse_msg_str(outstr);
						}),
					Button(text = "Copy",
						forceFitH = true,
						onPressFunc = [&]()
						{
							set_al_clipboard(preview->getText());
						}),
					Button(text = "Paste",
						forceFitH = true,
						onPressFunc = [&]()
						{
							std::string tmp;
							if(get_al_clipboard(tmp))
							{
								str_field->setText(tmp);
								preview->setText(tmp);
								tmpMsgStr.s = parse_msg_str(tmp);
							}
							else InfoDialog("Error","No text found on clipboard").show();
						})
				),
				Rows<2>(
					DDL(font, list_font),
					DummyWidget(),
					nextstr_dd = DropDownList(data = list_nextstr,
						fitParent = true,
						selectedValue = MsgStrings[tmpMsgStr.nextstring].listpos,
						onSelectFunc = [&](int32_t val)
						{
							tmpMsgStr.nextstring = addtomsglist(val);
						}
					),
					Button(text = "Next in List",
						forceFitH = true,
						onPressFunc = [&]()
						{
							if(tmpMsgStr.listpos+1 < msg_count)
							{
								tmpMsgStr.nextstring = tmpMsgStr.listpos + 1;
								nextstr_dd->setSelectedValue(tmpMsgStr.nextstring);
							}
						})
				)
			)),
			TabRef(name = "Attributes", Column(
				Columns<4>(
					//Col 1
					Label(text = "X:", hAlign = 1.0),
					Label(text = "Y:", hAlign = 1.0),
					Label(text = "SFX:", hAlign = 1.0),
					Label(text = "Layer:", hAlign = 1.0),
					NUM_FIELD(x,-512,512),
					NUM_FIELD(y,-512,512),
					NUM_FIELD(sfx,0,255),
					NUM_FIELD(drawlayer,0,7),
					//Col 2
					Label(text = "Width:", hAlign = 1.0),
					Label(text = "Height:", hAlign = 1.0),
					Label(text = "HSpace:", hAlign = 1.0),
					Label(text = "VSpace:", hAlign = 1.0),
					NUM_FIELD(w,8,512),
					NUM_FIELD(h,8,512),
					NUM_FIELD(hspace,0,128),
					NUM_FIELD(vspace,0,128),
					//Col 3
					Label(text = "T. Margin:", hAlign = 1.0),
					Label(text = "B. Margin:", hAlign = 1.0),
					Label(text = "L. Margin:", hAlign = 1.0),
					Label(text = "R. Margin:", hAlign = 1.0),
					NUM_FIELD(margins[0],0,255),
					NUM_FIELD(margins[1],0,255),
					NUM_FIELD(margins[2],0,255),
					NUM_FIELD(margins[3],0,255)
				),
				Row(
					Rows<2>(
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_CONT,
							"Is cont. of prev. string"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_WRAP,
							"Text wraps"),
						IBTN("The background tile will use a full rectangle"
							" instead of a frame if this is checked."),
						CHECKB(stringflags, STRINGFLAG_FULLTILE,
							"Full Tiled BG"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_TRANS_BG,
							"Transparent BG"),
						DummyWidget(),
						CHECKB(stringflags, STRINGFLAG_TRANS_FG,
							"Transparent FG")
					),
					Rows<3>(
						DummyWidget(),
						Label(text = "Shadow Type", hAlign = 1.0),
						DDL(shadow_type, list_shtype),
						DummyWidget(),
						Label(text = "Shadow Color", hAlign = 1.0),
						COLOR_FIELD(shadow_color),
						IBTN("The top-left tile of the background. If 'Full Tiled Background'"
							" is enabled, a rectangle of tiles the size of the message box is used."
							" Otherwise, a 2x2 block is treated as a 'frame'."),
						Label(text = "Background:", hAlign = 1.0),
						TILE_FIELD(tile, cset)
					)
				)
			)),
			TabRef(name = "Portrait", Column(
				Columns<2>(
					Label(text = "X:", hAlign = 1.0),
					Label(text = "Y:", hAlign = 1.0),
					NUM_FIELD(portrait_x,0,255),
					NUM_FIELD(portrait_y,0,255),
					Label(text = "Tile Width:", hAlign = 1.0),
					Label(text = "Tile Height:", hAlign = 1.0),
					NUM_FIELD(portrait_tw,0,16),
					NUM_FIELD(portrait_th,0,14)
				),
				Row(
					Label(text = "Portrait Tile:", hAlign = 1.0),
					TILE_FIELD(portrait_tile, portrait_cset)
				)
			))
		);
	}
	window = Window(
		title = "String Editor ("+std::to_string(strIndex)+")",
		onClose = message::CANCEL,
		Column(
			tpan,
			Row(
				vAlign = 1.0,
				spacing = 2_em,
				Button(focused = stred_tab_1!=0,
					text = "OK",
					minwidth = 90_lpx,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_lpx,
					onClick = message::CANCEL),
				Button(
					text = "Help",
					minwidth = 90_lpx,
					onPressFunc = []()
					{
						// Show string help
						editmsg_help_dlg[0].dp2= lfont;
						FILE *stringshelpfile = fopen("docs/zstrings.txt", "r");
						if (!stringshelpfile )
						{
							stringshelpfile = fopen("zstrings.txt", "r");
							if ( stringshelpfile )
							{
								editmsg_help_dlg[2].dp = new EditboxModel(helpstr, new EditboxScriptView(&editmsg_help_dlg[2],(is_large?sfont3:font),vc(0),vc(15),BasicEditboxView::HSTYLE_EOTEXT), true, (char *)"zstrings.txt");
							}
							else
							{
								Z_error_fatal("File Missing: zstrings.txt.");
							}
						}
						else
						{
							editmsg_help_dlg[2].dp = new EditboxModel(helpstr, new EditboxScriptView(&editmsg_help_dlg[2],(is_large?sfont3:font),vc(0),vc(15),BasicEditboxView::HSTYLE_EOTEXT), true, (char *)"docs/zstrings.txt");
						}
						editmsg_help_dlg[2].fg = vc(0);
						editmsg_help_dlg[2].bg = vc(15);
						((EditboxModel*)editmsg_help_dlg[2].dp)->doHelp(); // This deletes the EditboxModel too.
					})
			)
		));
	return window;
}

bool StringEditorDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	switch(msg.message)
	{
		case message::OK:
		{
			MsgStrings[strIndex] = tmpMsgStr;
			if(strIndex == msg_count)
			{
				if(addAfter != -1)
				{
					MsgStrings[strIndex].listpos = MsgStrings[addAfter].listpos+1;
					for(int32_t j=msg_count-1; j>=MsgStrings[strIndex].listpos; j--)
					{
						MsgStrings[addtomsglist(j)].listpos++;
					}
					msglistcache.clear();
				}
				
				++msg_count;
				MsgStrings[msg_count].sfx = 18;
				MsgStrings[msg_count].listpos = msg_count;
			}
			saved = false;
		}
		return true;

		case message::CANCEL:
		default:
		{
			if(addAfter != -1)
				MsgStrings[strIndex].listpos = msg_count;
		}
		return true;
	}
}
