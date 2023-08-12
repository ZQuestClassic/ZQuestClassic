#ifndef ZC_DIALOG_CHEATKEYS_H
#define ZC_DIALOG_CHEATKEYS_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/list_data.h>
#include <functional>
#include <string_view>
#include "zc/cheats.h"

class CheatKeysDialog: public GUI::Dialog<CheatKeysDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, RELOAD };

	CheatKeysDialog(bool* confirm);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bool* confirm_ptr;
	std::shared_ptr<GUI::Label> lbls[Cheat::Last][2];
	
	void load_labels();
};

#endif

