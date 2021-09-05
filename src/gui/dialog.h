#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "widget.h"
#include "dialogMessage.h"
#include "dialogRef.h"
#include "showDialog.h"
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

    /* Creates and initializes the dialog's window. */
    virtual std::shared_ptr<Widget> view()=0;

    void show()
    {
        showDialog(*static_cast<T*>(this));
    }

    /* Subclasses must implement one of these two. Don't implement both.
    bool handleMessage(T msg, MessageArg arg)
    bool handleMessage(T msg)
    */
};

}

#endif
