#ifndef ZC_DIALOG_ZINFODLG_H
#define ZC_DIALOG_ZINFODLG_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/label.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>

void call_zinf_dlg();

class ZInfoDialog: public GUI::Dialog<ZInfoDialog>
{
public:
	enum class message { OK, CANCEL };

	ZInfoDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TextField> fields[2];
	std::shared_ptr<GUI::Label> helplbl;
	std::shared_ptr<GUI::Checkbox> defcheck[2];
	zinfo lzinfo;
	GUI::ListData list_itemclass;
};

#endif
