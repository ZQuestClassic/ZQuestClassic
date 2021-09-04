#include "window.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <algorithm>
#include <cassert>
#include <utility>

using std::max, std::shared_ptr;

namespace gui
{

Window::Window(): content(nullptr), title(""), closeMessage(-1)
{
    hPadding=0;
    vPadding=0;
}

void Window::setTitle(std::string newTitle)
{
    title=std::move(newTitle);
}

void Window::setContent(shared_ptr<Widget> newContent)
{
    content=newContent;
}

void Window::arrange(int contX, int contY, int contW, int contH)
{
    if(content)
    {
        // First, size the window to its content.
        content->calculateSize();
        width=max(content->getTotalWidth()+12, text_length(lfont_l, title.c_str())+32);
        height=content->getTotalHeight()+36;

        // This will limit the window to the available size.
        Widget::arrange(contX, contY, contW, contH);

        // Then arrange the content with the final size.
        content->arrange(x+6, y+28, width-12, height-36);
    }
    else
    {
        x=contX;
        y=contY;
        width=contW;
        height=contH;
    }
}

void Window::realize(DialogRunner& runner)
{
    runner.push(shared_from_this(), DIALOG {
        jwin_win_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        D_NEW_GUI | (closeMessage ? D_EXIT : 0), // flags,
        0, 0, // d1, d2
        (void*)title.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });

    if(content)
        content->realize(runner);
}

int Window::onEvent(int event, MessageDispatcher sendMessage)
{
    assert(event==ngeCLOSE);
    if(closeMessage>=0)
        sendMessage(closeMessage, noArg);
    return -1;
}

}
