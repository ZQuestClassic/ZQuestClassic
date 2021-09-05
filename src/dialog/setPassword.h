#ifndef ZC_DIALOG_SETPASSWORD_H
#define ZC_DIALOG_SETPASSWORD_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/textField.h>
#include <functional>
#include <string_view>

class SetPasswordDialog: public gui::Dialog<SetPasswordDialog>
{
public:
    enum class Message { ok, cancel };

    SetPasswordDialog(bool useKeyFile,
        std::function<void(std::string_view, bool)> setPassword);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message msg);

private:
    bool useKeyFile;
    std::shared_ptr<gui::Checkbox> saveKeyFileCB;
    std::shared_ptr<gui::TextField> pwField;
    std::function<void(std::string_view, bool)> setPassword;
};

#endif
