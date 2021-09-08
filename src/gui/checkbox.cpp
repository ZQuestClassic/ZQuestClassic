#include "checkbox.h"
#include "common.h"
#include "dialog.h"
#include "dialogRunner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>

#define FONT sized(nfont, lfont_l)

namespace gui
{

Checkbox::Checkbox(): checked(false), text(),
    boxPlacement(BoxPlacement::left), alDialog(), message(-1)
{
    setPreferredHeight(Size::largePixels(text_height(FONT)+2));
    setPreferredWidth(Size::largePixels(13));
}

void Checkbox::setText(std::string newText)
{
    int textWidth=text_length(FONT, newText.c_str());
    setPreferredWidth(Size::largePixels(textWidth+13));
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

void Checkbox::setVisible(bool visible)
{
    Widget::setVisible(visible);
    if(alDialog)
    {
        if(visible)
            alDialog->flags&=~D_HIDDEN;
        else
            alDialog->flags|=D_HIDDEN;
    }
}

void Checkbox::realize(DialogRunner& runner)
{
    alDialog=runner.push(shared_from_this(), DIALOG {
        jwin_checkfont_proc,
        x, y, getWidth(), getHeight(),
        fgColor, bgColor,
        getAccelKey(text),
        getFlags()|(checked ? D_SELECTED : 0), // flags
        static_cast<int>(boxPlacement), 0, // d1, d2,
        (void*)text.c_str(), FONT, nullptr // dp, dp2, dp3
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
