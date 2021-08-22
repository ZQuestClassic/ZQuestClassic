#include "textField.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zdefs.h"
#include "../zquest.h"
#include <algorithm>
#include <cstring>
#include <utility>

namespace gui
{

TextField::TextField(): buffer(nullptr), maxLength(0), message(-1)
{
    width=8;
    height=text_height(lfont_l)+8;
    fgColor=vc(12);
    bgColor=vc(1);
}

std::string_view TextField::getText()
{
    if(maxLength==0)
        return "";
    return buffer.get();
}

void TextField::setText(std::string_view newText)
{
    // Would it be better to set an appropriate length?
    assert(buffer);
    newText.copy(buffer.get(), maxLength);
}

void TextField::setMaxLength(size_t newMax)
{
    assert(newMax>0);
    if(newMax==maxLength)
        return;

    auto newBuffer=std::make_unique<char[]>(newMax+1);
    if(maxLength>0)
    {
        strncpy(newBuffer.get(), buffer.get(), std::min(maxLength, newMax));
        newBuffer[newMax-1]='\0';
    }
    else
        newBuffer[0]='\0';

    buffer=std::move(newBuffer);
    maxLength=newMax;

    width=8+text_length(lfont_l, "N")*std::min<size_t>(newMax, 32);
}

void TextField::realize(DialogRunner& runner)
{
    assert(maxLength>0);

    runner.push(shared_from_this(), DIALOG {
        jwin_edit_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        (message>=0) ? D_EXIT : 0, // flags -  D_EXIT needed to send messages
        static_cast<int>(maxLength), 0, // d1, d2
        (void*)buffer.get(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

int TextField::getMessage()
{
    return message;
}

}
