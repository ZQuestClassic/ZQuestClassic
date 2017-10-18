#ifndef _ZC_GUI_GTK_WIDGET_H_
#define _ZC_GUI_GTK_WIDGET_H_

#include <gtk/gtk.h>

namespace GUI
{

class ZCGtkController;

// Named to distinguish it from GtkWidget
class ZCGtkWidget
{
public:
    ZCGtkWidget();
    virtual ~ZCGtkWidget();
    virtual inline void setController(ZCGtkController* c) { controller=c; }
    inline ZCGtkController* getController() { return controller; }
    virtual GtkWidget* get()=0;
    
    virtual bool fillsWidth() const;
    virtual bool fillsHeight() const;
    
private:
    ZCGtkController* controller;
};

}

#endif

