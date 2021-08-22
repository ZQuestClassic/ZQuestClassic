#ifndef ZC_DIALOG_SETPASSWORD_H
#define ZC_DIALOG_SETPASSWORD_H

#include <gui/dialog.h>
#include <gui/checkBox.h>
#include <gui/textField.h>
#include <functional>
#include <string_view>

enum class SetPasswordDialogMessage { ok, cancel };

class SetPasswordDialog: public gui::Dialog<SetPasswordDialogMessage>
{
public:
    using Message=SetPasswordDialogMessage;

    SetPasswordDialog(bool useKeyFile,
        std::function<void(std::string_view, bool)> setPassword);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message msg) override;

private:
    bool useKeyFile;
    std::shared_ptr<gui::CheckBox> saveKeyFileCB;
    std::shared_ptr<gui::TextField> pwField;
    std::function<void(std::string_view, bool)> setPassword;
};

#endif
