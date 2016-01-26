#include "window.h"
#include "controller.h"
#include "util.h"
#include <gui/key.h>

namespace GUI
{

void ZCGtkWindow::windowClosed(GtkWidget*, ZCGtkWindow* window)
{
    // Currently, a window is always a top-level widget,
    // and it's the only thing that can be.
    window->window=0;
    window->getController()->shutDown();
}

gboolean ZCGtkWindow::keyPressed(GtkWidget*, GdkEvent* event, ZCGtkWindow* window)
{
    KeyInput keys;
    if(translateKeyEvent(event, keys))
        return window->getController()->onKeyPressed(keys);
    else
        return FALSE;
}

gboolean ZCGtkWindow::deleteEvent(GtkWidget*, GdkEvent*, ZCGtkWindow* window)
{
    if(!window->onCloseFunc)
        return FALSE;
    if(window->onCloseFunc())
        return FALSE;
    return TRUE;
}

ZCGtkWindow::ZCGtkWindow(const std::string& title, ZCGtkWidget* c):
    window(0),
    contents(c)
{
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
    gtk_window_set_type_hint (GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
    g_signal_connect(window, "delete-event", G_CALLBACK(deleteEvent), this);
    g_signal_connect(window, "key-press-event", G_CALLBACK(keyPressed), this);
    g_signal_connect(window, "destroy", G_CALLBACK(windowClosed), this);
    
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    gtk_container_add(GTK_CONTAINER(window), c->get());
}

ZCGtkWindow::~ZCGtkWindow()
{
}

void ZCGtkWindow::setOnClose(boost::function<bool()> func)
{
    onCloseFunc=func;
}

void ZCGtkWindow::setController(ZCGtkController* c)
{
    ZCGtkWidget::setController(c);
    contents->setController(c);
}

GtkWidget* ZCGtkWindow::get()
{
    return window;
}

}
