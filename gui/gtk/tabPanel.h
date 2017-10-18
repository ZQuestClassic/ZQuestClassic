#ifndef _ZC_GUI_GTK_TABPANEL_H_
#define _ZC_GUI_GTK_TABPANEL_H_

#include <gui/tabPanel.h>
#include "widget.h"
#include <vector>

namespace GUI
{

class ZCGtkTabPanel: public TabPanel, public ZCGtkWidget
{
public:
    ZCGtkTabPanel();
    ~ZCGtkTabPanel();
    void addTab(const std::string& name, Widget* contents);
    void setController(ZCGtkController* c);
    GtkWidget* get();
    
private:
    GtkWidget* tabPanel;
    std::vector<ZCGtkWidget*> contents;
};

}

#endif
