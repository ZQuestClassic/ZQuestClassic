#include "topLevel.h"
#include "dialog.h"
#include "dialogRunner.h"
#include "../jwin.h"
#include "../zc_alleg.h"
#include <utility>

namespace gui
{

int TopLevelWidget::proc(int msg, DIALOG* d, int c)
{
    if(msg==MSG_XCHAR)
    {
        unsigned short actual=(key_shifts&0x07)|(c&0xFF00);
        if(actual==d->d1)
            // Abusing the mechanism here slightly...
            return new_gui_event(d, (NewGuiEvent)d->d2);
    }
    return D_O_K;
}

void TopLevelWidget::addShortcuts(
    std::initializer_list<KeyboardShortcut>&& scList)
{
    if(shortcuts.empty())
        shortcuts=std::move(scList);
    else
    {
        shortcuts.reserve(scList.size());
        for(auto& sc: scList)
            shortcuts.push_back(sc);
    }
}

void TopLevelWidget::realizeKeys(DialogRunner& runner)
{
    for(int i=0; i<shortcuts.size(); i++)
    {
        runner.push(shared_from_this(), DIALOG {
            proc,
            0, 0, 0, 0, // x, y, w, h
            0, 0, // fg, bg
            0, // key - Allegro ignores shift, so we're using MSG_XCHAR instead
            0, // flags
            shortcuts[i].key, -(i+1), // d1, d2
            this, nullptr, nullptr // dp, dp2, dp3
        });
    }
}

int TopLevelWidget::onEvent(int event, MessageDispatcher sendMessage)
{
    if(event<0)
        sendMessage(shortcuts.at(-event-1).message, noArg);

    return -1;
}

}
