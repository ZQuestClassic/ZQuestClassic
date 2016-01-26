#ifndef _ZC_GUI_GTK_CONTROLLER_H_
#define _ZC_GUI_GTK_CONTROLLER_H_

#include <gui/controller.h>
#include "widget.h"
#include <scoped_ptr.h>

namespace GUI
{

class ZCGtkController: public DialogController
{
public:
    void setDialogRoot(Widget* root);
    void initialize();
    void update();
    void shutDown();
    bool isOpen();
    
private:
    scoped_ptr<ZCGtkWidget> root;
};

}

#endif
