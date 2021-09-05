#ifndef ZC_GUI_DIALOGRUNNER_H
#define ZC_GUI_DIALOGRUNNER_H

#include "dialog.h"
#include "dialogEvent.h"

namespace gui
{

class DialogRunner
{
public:
    DialogRunner();

    template<typename T>
    void runWithArg(T& dlg)
    {
        sendMessage=[&dlg, this](int message, EventArg arg)
        {
            this->done=this->done ||
                dlg.handleMessage(static_cast<typename T::Message>(message), arg);
        };

        runInner(dlg.view());
    }

    template<typename T>
    void runWithoutArg(T& dlg)
    {
        sendMessage=[&dlg, this](int message, EventArg)
        {
            this->done=this->done ||
                dlg.handleMessage(static_cast<typename T::Message>(message));
        };

        runInner(dlg.view());
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
    bool redrawPending, done;

    /* Sets up the DIALOG array for a dialog so that it can be run.
     */
    void realize(std::shared_ptr<Widget> root);

    void runInner(std::shared_ptr<Widget> root);

    friend class DialogRef;
    friend int dialog_proc(int msg, DIALOG *d, int c);
};

}

#endif
