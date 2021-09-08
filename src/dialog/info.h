#ifndef ZC_DIALOG_INFO_H
#define ZC_DIALOG_INFO_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/textField.h>
#include <initializer_list>
#include <string>
#include <string_view>

// A basic dialog that just shows some lines of text and a close button.
class InfoDialog: public gui::Dialog<InfoDialog>
{
public:
    using Message=int;

    InfoDialog(std::string title, std::string text);
    InfoDialog(std::string title, std::vector<std::string_view> lines);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message);

private:
    std::string dlgTitle;
    std::string dlgText;
};

#endif
