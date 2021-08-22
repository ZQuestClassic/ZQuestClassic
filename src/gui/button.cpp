#include "button.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

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
        D_EXIT, // flags - D_EXIT needed to send messages
                // (TODO: this will need changed for toggle buttons)
        0, 0, // d1, d2
        (void*)text.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

int Button::getMessage()
{
    return message;
}

}
