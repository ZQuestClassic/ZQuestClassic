#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "widget.h"
#include "dialogEvent.h"
#include "dialogRef.h"
#include "../zc_alleg.h"
#include "../gui.h"
#include <memory>
#include <vector>

namespace gui
{

template<typename T>
class Dialog
{
public:
    virtual ~Dialog() {}

    /* Creates and initializes the dialog's window.
     */
    virtual std::shared_ptr<Widget> view()=0;

    virtual bool handleMessage(T msg, EventArg arg)
    {
        return handleMessage(msg);
    }

    /* Handles a message. Returns true if the dialog should close.
     */
    virtual bool handleMessage(T msg)
    {
        return handleMessage(msg,  std::monostate());
    }

    void show();
};

class DialogRunner
{
public:
    DialogRunner();

    template<typename T>
    void run(Dialog<T>& dlg)
    {
        sendMessage=[&dlg, this](int message, EventArg arg)
        {
            this->done=this->done ||
                dlg.handleMessage(static_cast<T>(message), arg);
        };

        realize(dlg.view());
        while(!done)
        {
            int ret=zc_popup_dialog(alDialog.data(), -1);
            if(ret<0 || done)
                break;
            int msg=widgets[ret]->getMessage();
            assert(msg>=0);
            if(dlg.handleMessage(static_cast<T>(msg)))
                break;
        }
    }

    /* Add a DIALOG and connect it to its owner.
     * This should always be called as
     * runner.push(shared_from_this(), DIALOG { ... });
     * Returns a DialogRef that can be used as a reference to the
     * newly added DIALOG.
     */
    DialogRef push(std::shared_ptr<Widget> owner, DIALOG dlg);

    /* Returns a DialogRef that can be used as a reference to the
     * most recently added DIALOG.
     */
    DialogRef getAllegroDialog();

private:
    MessageDispatcher sendMessage;
    std::vector<DIALOG> alDialog;
    std::vector<std::shared_ptr<Widget>> widgets;
    bool done;

    /* Sets up the DIALOG array for a dialog so that it can be run.
     */
    void realize(std::shared_ptr<Widget> root);

    friend class DialogRef;
    friend int dialog_proc(int msg, DIALOG *d, int c);
};


template<typename T>
void Dialog<T>::show()
{
    auto dr=DialogRunner();
    dr.run(*this);
}

}

#endif
