#include "info.h"
#include <gui/builder.h>
#include <string>

InfoDialog::InfoDialog(std::string_view title,
    const std::initializer_list<const char*>& linesSrc)
: windowTitle(title)
{
    for(auto& line: linesSrc)
        lines.emplace_back(line);
}

InfoDialog::InfoDialog(std::string_view title,
    std::vector<std::string_view> lines)
: windowTitle(title)
, lines(lines)
{}

std::shared_ptr<gui::Widget> InfoDialog::view()
{
    using namespace gui::builder;
    using namespace gui::props;

    auto col=Column(hPadding=30);
    for(auto& line: lines)
        col->add(Label(text=std::string(line)));
    col->add(Button(
        text="Close",
        vPadding=20,
        vAlign=1.0,
        onClick=0));

    return Window(
        title=std::string(windowTitle),
        onClose=0,
        col);
}

bool InfoDialog::handleMessage(Message)
{
    return true;
}
