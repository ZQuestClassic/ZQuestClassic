#ifndef ZC_DIALOG_STRINGEDIT_H_
#define ZC_DIALOG_STRINGEDIT_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/radio.h>
#include <gui/drop_down_list.h>
#include <zq/gui/msgstr_preview.h>
#include <gui/window.h>
#include <functional>
#include <string_view>

void call_stringedit_dialog(size_t ind, int32_t templateID, int32_t addAfter);

class StringEditorDialog: public GUI::Dialog<StringEditorDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	StringEditorDialog(size_t ind, int32_t templateID, int32_t addAfter);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	size_t strIndex;
	int32_t addAfter;
	MsgStr tmpMsgStr;
	
	GUI::ListData list_nextstr, list_font, list_font_order, list_shtype;
	
	std::shared_ptr<GUI::MsgPreview> preview;
	std::shared_ptr<GUI::DropDownList> font_dd, nextstr_dd;
	std::shared_ptr<GUI::Checkbox> fontsort_cb;
	std::shared_ptr<GUI::TextField> str_field;
	std::shared_ptr<GUI::Window> window;
};

#endif
