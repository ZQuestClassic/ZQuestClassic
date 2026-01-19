#include "string_editor.h"
#include "base/scc.h"
#include "base/zsys.h"
#include "base/msgstr.h"
#include "dialog/stringcontrolcode.h"
#include "zq/zquest.h"
#include "info.h"
#include "subscr.h"
#include "gui/editbox.h"
#include <gui/builder.h>
#include "zc_list_data.h"
#include <fmt/ranges.h>

void mark_save_dirty();
extern char msgbuf[MSGBUF_SIZE];
extern char namebuf[9];
extern word msg_count;
extern std::map<int32_t, int32_t> msglistcache;
extern std::string helpstr;

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
	list_sfx(GUI::ZCListData::sfxnames(true)),
	list_shtype(createShadowTypesListData())
{
	if(ind == msg_count) //new str
	{
		if(templateID > 0 && templateID < msg_count)
			tmpMsgStr.copyStyle(MsgStrings[templateID]);
		tmpMsgStr.setFromAsciiEncoding("");
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

#define DDL(member, lister, maxw) \
DropDownList(data = lister, \
	fitParent = true, maxwidth = maxw, \
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
//}

void StringEditorDialog::onTextUpdated(std::string text)
{
	msgStrWarnings = tmpMsgStr.setFromAsciiEncoding(std::move(text));
	warnings_button->setText(fmt::format("Warnings ({})", msgStrWarnings.size()));
	warnings_button->setDisabled(msgStrWarnings.empty());
}

void StringEditorDialog::updateCurrentSCC(int32_t cursor_start, int32_t cursor_end)
{
	size_t commands_index = 0;
	bool found_scc = false;

	for (auto segment_type : tmpMsgStr.parsed_msg_str.segment_types)
	{
		if (segment_type == ParsedMsgStr::SegmentType::Command)
		{
			auto& command = tmpMsgStr.parsed_msg_str.commands[commands_index++];
			// -1 to make the last character (usually the trailing space) not count, so that if the
			// cursor is at the very end then insertion can still happen.
			size_t command_end = command.start + command.length - 1;
			if (cursor_start >= command.start && cursor_start < command_end)
			{
				if (cursor_end == -1 || cursor_end < command_end)
					found_scc = true;

				break;
			}
		}
	}

	scc_wizard_button->setText(found_scc ? "Update SCC" : "Insert SCC");
	currentSCCIndex = found_scc ? commands_index - 1 : -1;
}

static size_t stred_tab_1 = 0;
bool sorted_fontdd = true;
std::shared_ptr<GUI::Widget> StringEditorDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	using GUI::Props::indx;
	sorted_fontdd = zc_get_config("zquest","stringed_sorted_font",1);

	tmpMsgStr.ensureAsciiEncoding();
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
					// preview->setText(foo); // I think this is not used...
					onTextUpdated(std::move(foo));
				},
				onCursorChangedFunc = [&](GUI::TextField::type, int32_t start, int32_t end)
				{
					updateCurrentSCC(start, end);
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
				scc_wizard_button = Button(
					text = "Insert SCC", forceFitH = true,
					onPressFunc = [&]()
					{
						bool old_fontsort = sorted_fontdd;
						std::string scc = run_scc_dlg(&tmpMsgStr, currentSCCIndex);
						if(old_fontsort != sorted_fontdd)
						{
							font_dd->setListData(sorted_fontdd ? list_font : list_font_order);
							fontsort_cb->setChecked(sorted_fontdd);
						}
						if(scc.empty())
							return;
						std::string fullstr;
						fullstr.assign(str_field->getText());

						std::string outstr;
						if (currentSCCIndex == -1)
						{
							// inserting
							int32_t pos = str_field->get_str_selected_pos();
							int32_t endpos = str_field->get_str_selected_endpos();
							size_t len = str_field->get_str_pos();
							if (pos > 0 && fullstr.size() != 0 && fullstr[pos - 1] == '\\')
								scc = " " + scc;

							if(endpos>-1)
								outstr = fullstr.substr(0, pos) + scc + fullstr.substr(zc_min(len, endpos+1));
							else
								outstr = fullstr.substr(0, pos) + scc + fullstr.substr(pos);
							str_field->set_str_selected_pos(pos + scc.length());
						}
						else
						{
							// replacing
							auto& command = tmpMsgStr.parsed_msg_str.commands[currentSCCIndex];
							outstr = fullstr.substr(0, command.start) + scc + fullstr.substr(command.start + command.length);
							str_field->set_str_selected_pos(command.start + scc.size() - 1);
						}

						str_field->setText(outstr);
						str_field->setFocused(true);
						preview->setText(outstr);
						onTextUpdated(std::move(outstr));
						updateCurrentSCC(str_field->get_str_selected_pos(), -1);
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
							preview->setText(tmp);
							str_field->setText(tmp);
							onTextUpdated(std::move(tmp));
						}
						else InfoDialog("Error","No text found on clipboard").show();
					})
			),
			Rows<2>(
				font_dd = DDL(font, sorted_fontdd ? list_font : list_font_order, 500_px),
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
					fitParent = true, maxwidth = 500_px,
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
			Rows_Columns<3, 4>(
				//Col 1
				Label(text = "X:", hAlign = 1.0),
				NUM_FIELD(x,-512,512),
				INFOBTN("The left X position of the textbox"),
				Label(text = "Y:", hAlign = 1.0),
				NUM_FIELD(y,-512,512),
				INFOBTN("The top Y position of the textbox"),
				Label(text = "Layer:", hAlign = 1.0),
				NUM_FIELD(drawlayer,0,7),
				INFOBTN("The layer to draw the textbox on"),
				DummyWidget(),
				DummyWidget(),
				DummyWidget(),
				//Col 2
				Label(text = "Width:", hAlign = 1.0),
				NUM_FIELD(w,8,512),
				INFOBTN("The width of the textbox in pixels"),
				Label(text = "Height:", hAlign = 1.0),
				NUM_FIELD(h,8,512),
				INFOBTN("The height of the textbox in pixels"),
				Label(text = "HSpace:", hAlign = 1.0),
				NUM_FIELD(hspace,0,128),
				INFOBTN("The extra space between letters, in pixels"),
				Label(text = "VSpace:", hAlign = 1.0),
				NUM_FIELD(vspace,0,128),
				INFOBTN("The extra space between lines, in pixels"),
				//Col 3
				Label(text = "Top Margin:", hAlign = 1.0),
				NUM_FIELD(margins[0],0,255),
				INFOBTN("The extra space between the top of the textbox and the text, in pixels"),
				Label(text = "Bottom Margin:", hAlign = 1.0),
				NUM_FIELD(margins[1],0,255),
				INFOBTN("The extra space between the bottom of the textbox and the text, in pixels"),
				Label(text = "Left Margin:", hAlign = 1.0),
				NUM_FIELD(margins[2],0,255),
				INFOBTN("The extra space between the left of the textbox and the text, in pixels"),
				Label(text = "Right Margin:", hAlign = 1.0),
				NUM_FIELD(margins[3],0,255),
				INFOBTN("The extra space between the right of the textbox and the text, in pixels")
			),
			Rows<3>(
				Label(text = "SFX:", hAlign = 1.0),
				DDL(sfx, list_sfx, 500_px),
				INFOBTN("This SFX plays as the message is typed out")
			),
			Row(
				Rows<2>(
					INFOBTN("This string is typed out in the same textbox as the"
						" string before it, rather than starting a fresh textbox."),
					CHECKB(stringflags, STRINGFLAG_CONT, "Is continuation of previous string"),
					INFOBTN("The text will wrap when it reaches the right margin."),
					CHECKB(stringflags, STRINGFLAG_WRAP, "Text wraps around bounding box"),
					INFOBTN("The background tile will use a full rectangle"
						" instead of a frame if this is checked."),
					CHECKB(stringflags, STRINGFLAG_FULLTILE, "Full Tiled Background"),
					INFOBTN("The background is drawn transparently."),
					CHECKB(stringflags, STRINGFLAG_TRANS_BG, "Transparent BG"),
					INFOBTN("The foreground (text) is drawn transparently."),
					CHECKB(stringflags, STRINGFLAG_TRANS_FG, "Transparent FG")
				),
				Rows<3>(
					INFOBTN("Which type of shadow the text will have."
						"'Shadowed' types have a shadow behind the text,"
						" while 'Shadow' types replace the text entirely with a shadow."),
					Label(text = "Shadow Type", hAlign = 1.0),
					DDL(shadow_type, list_shtype, 200_px),
					INFOBTN("The color of the shadow."),
					Label(text = "Shadow Color", hAlign = 1.0),
					COLOR_FIELD(shadow_color),
					INFOBTN("The top-left tile of the background. If 'Full Tiled Background'"
						" is enabled, a rectangle of tiles the size of the message box is used."
						" Otherwise, a 2x2 block is treated as a 'frame'."),
					Label(text = "Background:", hAlign = 1.0),
					TILE_FIELD(tile, cset)
				)
			)
		)),
		TabRef(name = "Portrait", Column(
			Rows_Columns<2, 2>(
				Label(text = "X:", hAlign = 1.0),
				NUM_FIELD(portrait_x,0,255),
				Label(text = "Y:", hAlign = 1.0),
				NUM_FIELD(portrait_y,0,255),
				Label(text = "Tile Width:", hAlign = 1.0),
				NUM_FIELD(portrait_tw,0,16),
				Label(text = "Tile Height:", hAlign = 1.0),
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
						util::open_web_link("https://docs.zquestclassic.com/tutorials/message_strings");
					}),
				warnings_button = Button(
					text = "Warnings (0)",
					minwidth = 90_px,
					onPressFunc = [&]()
					{
						std::string text = fmt::format("{}", fmt::join(msgStrWarnings, "\n"));
						do_box_edit(text, "Message String Warnings", true, true);
					})
			)
		));

	onTextUpdated(str);

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
			mark_save_dirty();
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
