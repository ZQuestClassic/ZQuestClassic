#include "dialogRunner.h"
#include "common.h"
#include "../gui.h"
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
    auto* dr=static_cast<DialogRunner*>(d->dp);
    if(msg==MSG_NEW_GUI_EVENT)
    {
        int ret=dr->widgets[d->d1]->onEvent(c, dr->sendMessage);
        if(dr->done)
            return D_EXIT;
        else
        {
            dr->redrawPending=true;
            return ret;
        }
    }
    else if(msg==MSG_IDLE && dr->redrawPending)
    {
        // In the old system, many things sent messages by closing the dialog,
        // and the return value from do_zqdialog() became the message.
        // Some widgets don't have code to indicate that they need redrawn
        // since the dialog would be closed and reopened in that case.
        dr->redrawPending=false;
        return D_REDRAW;
    }
    else
        return D_O_K;
}

DialogRunner::DialogRunner(): redrawPending(false), done(false)
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

    if(is_large)
        root->arrange(0, 0, 800, 600);
    else
        root->arrange(0, 0, 320, 240);
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

void DialogRunner::runInner(std::shared_ptr<Widget> root)
{
    realize(root);

    int ret=0;
    while(!done && ret>=0)
        ret=zc_popup_dialog(alDialog.data(), -1);
}

DialogRef DialogRunner::getAllegroDialog()
{
    return DialogRef(this, alDialog.size()-1);
}

size_t DialogRunner::size() const
{
    return alDialog.size();
}

}
