#ifndef ZC_DIALOG_CHEATCODES_H
#define ZC_DIALOG_CHEATCODES_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/textField.h>
#include <functional>
#include <string_view>

class CheatCodesDialog: public GUI::Dialog<CheatCodesDialog>
{
public:
	enum class message { OK, CANCEL };

	CheatCodesDialog(bool enabled, std::string_view oldCodes[4],
		std::function<void(bool, std::string_view[4])> setCheatCodes);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(message msg);

private:
	bool enabled;
	std::shared_ptr<GUI::Checkbox> enabledCB;
	std::string_view oldCodes[4];
	std::shared_ptr<GUI::TextField> textFields[4];
	std::function<void(bool, std::string_view[4])> setCheatCodes;
};

#endif
