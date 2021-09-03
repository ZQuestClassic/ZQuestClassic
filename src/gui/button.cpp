#include "button.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>
#include <iostream>
namespace gui
{

Button::Button(): text(), message(-1)
{
    height=text_height(lfont_l)+10;
}

void Button::setText(std::string newText)
{
    width=text_length(lfont_l, newText.c_str())+20;
    text=std::move(newText);
}

void Button::realize(DialogRunner& runner)
{
    runner.push(shared_from_this(), DIALOG {
        jwin_button_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        D_NEW_GUI, // flags
        0, 0, // d1, d2
        (void*)text.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

int Button::getMessage()
{
    return message;
}

int Button::onEvent(int event, MessageDispatcher sendMessage)
{
    assert(event==ngeCLICK);
    if(message>=0)
        sendMessage(message, noArg);
    return -1;
}

}
