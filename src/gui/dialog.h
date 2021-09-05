#ifndef ZC_GUI_DIALOG_H
#define ZC_GUI_DIALOG_H

#include "showDialog.h"
#include "widget.h"
#include <memory>

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
