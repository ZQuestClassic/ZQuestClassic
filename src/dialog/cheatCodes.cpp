#include "cheatCodes.h"
#include <gui/builder.h>

CheatCodesDialog::CheatCodesDialog(bool enabled,
    std::string_view oldCodes[4],
    std::function<void(bool, std::string_view[4])> setCheatCodes)
: enabled(enabled)
, oldCodes { oldCodes[0], oldCodes[1], oldCodes[2], oldCodes[3] }
, setCheatCodes(setCheatCodes)
{}

std::shared_ptr<gui::Widget> CheatCodesDialog::view()
{
    using namespace gui::builder;
    using namespace gui::props;

    return Window(
        title="Cheat Codes",
        onClose=Message::cancel,
        Column(
            this->enabledCB=CheckBox(
                hAlign=0.95,
                text="Enable Cheats",
                checked=this->enabled),
            Columns<5>(
                Label(
                    hPadding=10,
                    text="Level"),
                Label(text="1"),
                Label(text="2"),
                Label(text="3"),
                Label(text="4"),

                Label(
                    hAlign=0.05,
                    text="Code"),
                this->textFields[0]=TextField(
                    maxLength=40,
                    text=this->oldCodes[0],
                    onEnter=Message::ok),
                this->textFields[1]=TextField(
                    maxLength=40,
                    text=this->oldCodes[1],
                    onEnter=Message::ok),
                this->textFields[2]=TextField(
                    maxLength=40,
                    text=this->oldCodes[2],
                    onEnter=Message::ok),
                this->textFields[3]=TextField(
                    maxLength=40,
                    text=this->oldCodes[3],
                    onEnter=Message::ok)
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

bool CheatCodesDialog::handleMessage(CheatCodesDialog::Message msg)
{
    switch(msg)
    {
    case Message::ok:
        {
            std::string_view newCodes[4]={
                textFields[0]->getText(), textFields[1]->getText(),
                textFields[2]->getText(), textFields[3]->getText()
            };
            setCheatCodes(enabledCB->getChecked(), newCodes);
        }
        [[fallthrough]]
    case Message::cancel:
    default:
        return true;
    }
}
