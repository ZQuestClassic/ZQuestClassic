#include "setPassword.h"
#include <gui/builder.h>

SetPasswordDialog::SetPasswordDialog(bool useKeyFile,
    std::function<void(std::string_view, bool)> setPassword)
: useKeyFile(useKeyFile), setPassword(setPassword)
{}

std::shared_ptr<gui::Widget> SetPasswordDialog::view()
{
    using namespace gui::builder;
    using namespace gui::props;

    return Window(
        title="Set Password",
        onClose=Message::cancel,
        Column(
            Rows<2>(
                Label(text="Enter new password:"),
                this->pwField=TextField(
                    maxLength=255,
                    onEnter=Message::ok),
                this->saveKeyFileCB=Checkbox(
                    text="&Save key file",
                    checked=this->useKeyFile)
            ),
            Row(
                vPadding=10,
                vAlign=1.0,
                Button(
                    text="&OK",
                    onClick=Message::ok,
                    hPadding=30),
                Button(
                    text="&Cancel",
                    onClick=Message::cancel,
                    hPadding=30)
            )
        )
    );
}

bool SetPasswordDialog::handleMessage(SetPasswordDialog::Message msg)
{
    switch(msg)
    {
    case Message::ok:
        setPassword(pwField->getText(), saveKeyFileCB->getChecked());
        [[fallthrough]]
    case Message::cancel:
    default:
        return true;
    }
}
