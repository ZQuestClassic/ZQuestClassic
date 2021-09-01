#ifndef ZC_DIALOG_CHEATCODES_H
#define ZC_DIALOG_CHEATCODES_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/textField.h>
#include <functional>
#include <string_view>

enum class CheatCodesDialogMessage { ok, cancel };

class CheatCodesDialog: public gui::Dialog<CheatCodesDialogMessage>
{
public:
    using Message=CheatCodesDialogMessage;

    CheatCodesDialog(bool enabled, std::string_view oldCodes[4],
        std::function<void(bool, std::string_view[4])> setCheatCodes);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message msg) override;

private:
    bool enabled;
    std::shared_ptr<gui::Checkbox> enabledCB;
    std::string_view oldCodes[4];
    std::shared_ptr<gui::TextField> textFields[4];
    std::function<void(bool, std::string_view[4])> setCheatCodes;
};

#endif
