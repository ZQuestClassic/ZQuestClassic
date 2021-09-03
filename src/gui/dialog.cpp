#include "dialog.h"
#include "../jwin.h"

using std::shared_ptr;

namespace gui
{

/* Called from jwin procs when an event occurs.
 * d->d1 is the index of the child responsible.
 * c is the event.
 */
int dialog_proc(int msg, DIALOG *d, int c)
{
    if(msg==MSG_NEW_GUI_EVENT)
    {
        auto* dr=static_cast<DialogRunner*>(d->dp);
        int ret=dr->widgets[d->d1]->onEvent(c, dr->sendMessage);
        if(dr->done)
            return D_EXIT;
        else
            return ret;
    }
    else
        return D_O_K;
}

DialogRunner::DialogRunner(): done(false)
{}

DialogRef DialogRunner::push(shared_ptr<Widget> owner, DIALOG dlg)
{
    widgets.push_back(owner);
    alDialog.push_back(std::move(dlg));
    return DialogRef(this, alDialog.size()-1);
}

void DialogRunner::realize(shared_ptr<Widget> root)
{
    alDialog.push_back({
        dialog_proc, // proc
        0, 0, 0, 0, // x, y, width, height
        0, 0, // fg, bg
        0, // key
        0, // flags
        0, 0, // d1, d2
        this, nullptr, &newGuiMarker // dp, dp2, dp3
    });

    root->arrange(0, 0, 800, 600);
    root->realize(*this);

    alDialog.push_back({
        nullptr, // proc
        0, 0, 0, 0, // x, y, width, height
        0, 0, // fg, bg
        0, // key
        0, // flags
        0, 0, // d1, d2
        nullptr, nullptr, nullptr // dp1, dp2, dp3
    });
}

DialogRef DialogRunner::getAllegroDialog()
{
    return DialogRef(this, alDialog.size()-1);
}

}
