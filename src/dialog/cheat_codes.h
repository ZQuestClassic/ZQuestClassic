#ifndef ZC_DIALOG_CHEATCODES_H_
#define ZC_DIALOG_CHEATCODES_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_cheats_dlg();

/* Note: The string_views passed to setCheatCodes will include
 * null terminators.
 */
class CheatCodesDialog: public GUI::Dialog<CheatCodesDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	CheatCodesDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Checkbox> enabledCB;
	std::shared_ptr<GUI::TextField> textFields[4];
};

#endif
