#include "checkbox.h"
#include "common.h"
#include "dialog.h"
#include "dialogRunner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>

namespace gui
{

Checkbox::Checkbox(): checked(false), text(),
    boxPlacement(BoxPlacement::right), alDialog(), message(-1)
{
    height=text_height(lfont_l)+2;
    width=12;
}

void Checkbox::setText(std::string newText)
{
    width=text_length(lfont_l, newText.c_str())+12;
    text=std::move(newText);
}

void Checkbox::setBoxPlacement(BoxPlacement bp)
{
    boxPlacement=bp;
}

void Checkbox::setChecked(bool value)
{
    checked=value;
}

bool Checkbox::getChecked()
{
    return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void Checkbox::realize(DialogRunner& runner)
{
    alDialog=runner.push(shared_from_this(), DIALOG {
        jwin_checkfont_proc,
        x, y, width, height,
        fgColor, bgColor,
        getAccelKey(text),
        D_NEW_GUI|(checked ? D_SELECTED : 0), // flags
        static_cast<int>(boxPlacement), 0, // d1, d2,
        (void*)text.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

int Checkbox::onEvent(int event, MessageDispatcher sendMessage)
{
    assert(event==ngeTOGGLE);
    if(message>=0)
        sendMessage(message, (alDialog->flags&D_SELECTED)!=0);
    return -1;
}

}
