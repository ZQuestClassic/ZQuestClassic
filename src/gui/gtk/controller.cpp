#include "controller.h"
#include "../widget.h"
#include <glib-2.0/glib.h>

namespace GUI
{

/*
~ZCGtkDialogController()
{
    // shut down if running, maybe?
}
*/

void ZCGtkController::setDialogRoot(Widget* r)
{
    ZCGtkWidget* w=dynamic_cast<ZCGtkWidget*>(r);
    root.reset(w);
    w->setController(this);
}

void ZCGtkController::initialize()
{
    gtk_widget_show_all(root->get());
    gtk_main();
}

void ZCGtkController::update()
{
    // Nothing to do here?
}

void ZCGtkController::shutDown()
{
    // This might have been called by the window, so it may have closed already
    GtkWidget* w=root->get();
    if(w)
        gtk_widget_destroy(w);
    root.reset();
    
    // Does this need to be conditional?
    gtk_main_quit();
}

bool ZCGtkController::isOpen()
{
    // Pretty sure this will never be called while a dialog is actually open...
    return false;
}

}
