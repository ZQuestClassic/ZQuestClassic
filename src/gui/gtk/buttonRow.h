#ifndef _ZC_GUI_GTK_BUTTONROW_H_
#define _ZC_GUI_GTK_BUTTONROW_H_

#include <gui/serialContainer.h>
#include "widget.h"
#include <vector>

namespace GUI
{

class ZCGtkButtonRow: public SerialContainer, public ZCGtkWidget
{
public:
    ZCGtkButtonRow();
    ~ZCGtkButtonRow();
    void add(Widget* newWidget);
    void setController(ZCGtkController* c);
    bool fillsWidth() const;
    GtkWidget* get();
    
private:
    GtkWidget* box;
    std::vector<ZCGtkWidget*> buttons;
};

}

#endif
