#ifndef _ZC_GUI_GTK_WINDOW_H_
#define _ZC_GUI_GTK_WINDOW_H_

#include <gui/window.h>
#include "widget.h"
#include <scoped_ptr.h>
#include <string>
#include <vector>

namespace GUI
{

class ZCGtkWindow: public Window, public ZCGtkWidget
{
public:
    ZCGtkWindow(const std::string& title, ZCGtkWidget* contents);
    ~ZCGtkWindow();
    void setMenu(Menu* menu) {} // TODO...
    void setOnClose(boost::function<bool()> func);
    void setController(ZCGtkController* c);
    GtkWidget* get();
    
private:
    GtkWidget* window;
    GtkAccelGroup* accelerators;
    scoped_ptr<ZCGtkWidget> contents;
    boost::function<bool()> onCloseFunc;
    
    static void windowClosed(GtkWidget*, ZCGtkWindow* window);
    static gboolean keyPressed(GtkWidget*, GdkEvent* event, ZCGtkWindow* window);
    static gboolean deleteEvent(GtkWidget*, GdkEvent*, ZCGtkWindow* window);
};

}

#endif
