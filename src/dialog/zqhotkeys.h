#ifndef ZQ_DIALOG_HOTKEYS_H
#define ZQ_DIALOG_HOTKEYS_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "zq/zq_hotkey.h"

class ZQHotkeyDialog: public GUI::Dialog<ZQHotkeyDialog>
{
public:
	enum class message { OK, CANCEL, RELOAD };

	ZQHotkeyDialog(bool* confirm);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bool* confirm_ptr;
	std::shared_ptr<GUI::Label> lbls[ZQKEY_MAX][2];
	
	void load_labels();
};

#endif

