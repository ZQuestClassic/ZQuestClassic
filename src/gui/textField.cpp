#include "textField.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zdefs.h"
#include "../zquest.h"
#include <algorithm>
#include <cstring>
#include <utility>
#include <cassert>

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
    // This probably could be handled with less allocating and copying...
    if(maxLength==0 && newText.size()>0)
        setMaxLength(newText.size());
    newText.copy(buffer.get(), maxLength);
    buffer[std::min(maxLength-1, newText.size())]='\0';
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
        D_NEW_GUI, // flags
        static_cast<int>(maxLength), 0, // d1, d2
        (void*)buffer.get(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

int TextField::onEvent(int event, MessageDispatcher sendMessage)
{
    assert(event==ngeENTER);
    if(message<0)
        return -1;

    if(maxLength>0)
        sendMessage(message, std::string_view(buffer.get()));
    else
        sendMessage(message, std::string_view(""));
    return -1;
}

}
