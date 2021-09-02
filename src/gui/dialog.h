#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "widget.h"
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

    /* Handles a message. Returns true if the dialog should close.
     */
    virtual bool handleMessage(T msg)=0;

    void show();
};

class DialogRunner;
// References an item in the DialogRunner's DIALOG array, which isn't safe
// to do directly because it might be reallocated as the vector grows.
// This is used by CheckBox to see if it's been checked.
// Not really a good way of handling it; this is just a temporary class
// to get things working.
class DialogRef
{
public:
    DialogRef();
    DIALOG* operator->();
    const DIALOG* operator->() const;
    operator bool() const;

private:
    DialogRunner* owner;
    size_t index;

    DialogRef(DialogRunner* owner, size_t index);

    friend class DialogRunner;
};

class DialogRunner
{
public:
    template<typename T>
    void run(Dialog<T>& dlg)
    {
        realize(dlg.view());
        while(true)
        {
            int ret=zc_popup_dialog(alDialog.data(), -1);
            if(ret<0)
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
     */
    void push(std::shared_ptr<Widget> owner, DIALOG dlg);

    /* Returns an object that acts as a reference to the last DIALOG added.
     */
    DialogRef getAllegroDialog();

private:
    std::vector<DIALOG> alDialog;
    std::vector<std::shared_ptr<Widget>> widgets;

    /* Gets the DIALOG array for a dialog so that it can be run.
     */
    void realize(std::shared_ptr<Widget> root);

    friend class DialogRef;
};


template<typename T>
void Dialog<T>::show()
{
    auto dr=DialogRunner();
    dr.run(*this);
}

}

#endif
