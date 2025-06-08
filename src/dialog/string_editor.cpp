#include "string_editor.h"
#include "base/zsys.h"
#include "base/msgstr.h"
#include "zq/zquest.h"
#include "info.h"
#include "subscr.h"
#include "gui/editbox.h"
#include <gui/builder.h>
#include "zc_list_data.h"

extern bool saved;
extern char msgbuf[MSGBUF_SIZE];
extern char namebuf[9];
extern word msg_count;
extern std::map<int32_t, int32_t> msglistcache;
extern std::string helpstr;

std::string run_scc_dlg(MsgStr const* ref);
std::string parse_to_legacy_msg_str_encoding(std::string const& s);
void strip_trailing_spaces(char *str);
void strip_trailing_spaces(std::string& str);
int32_t addtomsglist(int32_t index, bool allow_numerical_sort = true);
int32_t msg_at_pos(int32_t pos);
const char *msgslist(int32_t index, int32_t *list_size);

void call_stringedit_dialog(size_t ind, int32_t templateID, int32_t addAfter)
{
	StringEditorDialog(ind, templateID, addAfter).show();
}

extern std::string msgfont_str[font_max];
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
	: strIndex(ind), addAfter(addAfter),
	tmpMsgStr(MsgStrings[ind]),
	list_nextstr(GUI::ZCListData::strings(false)),
	list_font(GUI::ZCListData::fonts(false,true,true)),
	list_font_order(GUI::ZCListData::fonts(false,true,false)),
	list_shtype(createShadowTypesListData())
{
	if(ind == msg_count) //new str
	{
		if(templateID > 0 && templateID < msg_count)
			tmpMsgStr.copyStyle(MsgStrings[templateID]);
		tmpMsgStr.setFromLegacyEncoding("");
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
)

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
bool sorted_fontdd = true;
std::shared_ptr<GUI::Widget> StringEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using GUI::Props::indx;
	sorted_fontdd = zc_get_config("zquest","stringed_sorted_font",1);
	
	std::string str = tmpMsgStr.serialize();
	const char* start_text = str.c_str();
	std::shared_ptr<GUI::TabPanel> tpan = TabPanel(ptr = &stred_tab_1,
		TabRef(name = "String", Column(
			str_field = TextField(fitParent = true,
				forceFitW = true, height = 2_px+(3*(1_em+2_px)),
				maxLength = MSGBUF_SIZE - 1, type = GUI::TextField::type::TEXT,
				text = start_text, focused = !stred_tab_1,
				onValChangedFunc = [&](GUI::TextField::type,std::string_view v,int32_t)
				{
					std::string foo;
					foo.assign(v);
					preview->setText(foo);
					tmpMsgStr.setFromLegacyEncoding(parse_to_legacy_msg_str_encoding(foo));
				}
			),
			preview = MsgPreview(data = &tmpMsgStr, indx = strIndex, text = start_text),
			Row(padding = 0_px,
				Label(text = "Hero Name Preview:"),
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
						bool old_fontsort = sorted_fontdd;
						std::string scc = run_scc_dlg(&tmpMsgStr);
						if(old_fontsort != sorted_fontdd)
						{
							font_dd->setListData(sorted_fontdd ? list_font : list_font_order);
							fontsort_cb->setChecked(sorted_fontdd);
						}
						if(scc.empty())
							return;
						std::string fullstr;
						fullstr.assign(str_field->getText());
						int32_t pos = str_field->get_str_selected_pos();
						int32_t endpos = str_field->get_str_selected_endpos();
						size_t len = str_field->get_str_pos();
						std::string outstr;
						if(endpos>-1)
							outstr = fullstr.substr(0, pos) + scc + fullstr.substr(zc_min(len, endpos+1));
						else
							outstr = fullstr.substr(0, pos) + scc + fullstr.substr(pos);
						str_field->setText(outstr);
						preview->setText(outstr);
						tmpMsgStr.setFromLegacyEncoding(parse_to_legacy_msg_str_encoding(outstr));
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
							tmpMsgStr.setFromLegacyEncoding(parse_to_legacy_msg_str_encoding(tmp));
						}
						else InfoDialog("Error","No text found on clipboard").show();
					})
			),
			Rows<2>(
				font_dd = DDL(font, sorted_fontdd ? list_font : list_font_order),
				fontsort_cb = Checkbox(
					text = "Font Sort",
					checked = sorted_fontdd,
					onToggleFunc = [&](bool state)
					{
						sorted_fontdd = !sorted_fontdd;
						font_dd->setListData(sorted_fontdd ? list_font : list_font_order);
						zc_set_config("zquest","stringed_sorted_font",sorted_fontdd?1:0);
					}
				),
				nextstr_dd = DropDownList(data = list_nextstr,
					fitParent = true,
					selectedValue = tmpMsgStr.nextstring,
					onSelectFunc = [&](int32_t val)
					{
						tmpMsgStr.nextstring = val;
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
					minwidth = 90_px,
					onClick = message::OK),
				Button(
					text = "Cancel",
					minwidth = 90_px,
					onClick = message::CANCEL),
				Button(
					text = "Help",
					minwidth = 90_px,
					onPressFunc = []()
					{
						if(std::filesystem::exists("docs/zstrings.txt"))
							do_box_edit(fopen("docs/zstrings.txt","r"), "String Control Codes", true);
						else if(std::filesystem::exists("zstrings.txt"))
							do_box_edit(fopen("zstrings.txt","r"), "String Control Codes", true);
						else
							Z_error_fatal("File Missing: zstrings.txt.");
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
		{
			if(addAfter != -1)
				MsgStrings[strIndex].listpos = msg_count;
		}
		return true;
	}
	return false;
}
