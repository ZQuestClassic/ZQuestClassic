#ifndef _ZC_GUI_GTK_SERIALCONTAINER_H_
#define _ZC_GUI_GTK_SERIALCONTAINER_H_

#include <gui/serialContainer.h>
#include "widget.h"
#include <vector>

namespace GUI
{

class ZCGtkSerialContainer: public SerialContainer, public ZCGtkWidget
{
public:
    ZCGtkSerialContainer(bool vertical, bool evenlySpaced);
    ~ZCGtkSerialContainer();
    void add(Widget* newWidget);
    void setController(ZCGtkController* c);
    bool fillsWidth() const;
    bool fillsHeight() const;
    GtkWidget* get();
    
private:
    GtkWidget* box;
    const bool vertical, evenlySpaced;
    bool hasExpanders;
    std::vector<ZCGtkWidget*> contents;
};

}

#endif
