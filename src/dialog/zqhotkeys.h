#ifndef ZQ_DIALOG_HOTKEYS_H_
#define ZQ_DIALOG_HOTKEYS_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/scrolling_pane.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "zq/zq_hotkey.h"

class ZQHotkeyDialog: public GUI::Dialog<ZQHotkeyDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RELOAD, RERUN };

	ZQHotkeyDialog(bool& confirm);
	ZQHotkeyDialog(optional<uint>& favc);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bool* confirm_ptr;
	optional<uint>* favc;
	string searchstring;
	std::shared_ptr<GUI::ScrollingPane> spane;
	std::shared_ptr<GUI::Label> lbls[ZQKEY_MAX][2];
	
	void load_labels();
};

#endif

