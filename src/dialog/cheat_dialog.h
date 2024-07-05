#ifndef ZC_DIALOG_CHEATDLG_H_
#define ZC_DIALOG_CHEATDLG_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/radio.h>
#include <functional>
#include <string_view>

void call_setcheat_dialog();

class SetCheatDialog: public GUI::Dialog<SetCheatDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, CHECK };

	SetCheatDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TextField> field;
	std::shared_ptr<GUI::Radio> radios[5];
	std::shared_ptr<GUI::Label> errlabel;
};

#endif
