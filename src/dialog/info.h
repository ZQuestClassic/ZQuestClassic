#ifndef ZC_DIALOG_INFO_H
#define ZC_DIALOG_INFO_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/textField.h>
#include <initializer_list>
#include <string_view>
#include <vector>

// A basic dialog that just shows some lines of text and a close button.
class InfoDialog: public gui::Dialog<int>
{
public:
    using Message=int;

    InfoDialog(std::string_view title,
        const std::initializer_list<const char*>& lines);
    InfoDialog(std::string_view title, std::vector<std::string_view> lines);

    std::shared_ptr<gui::Widget> view() override;
    bool handleMessage(Message) override;

private:
    std::string_view windowTitle;
    std::vector<std::string_view> lines;
};

#endif
