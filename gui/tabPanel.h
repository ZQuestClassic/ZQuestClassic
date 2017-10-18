#ifndef _ZC_GUI_TABPANEL_H_
#define _ZC_GUI_TABPANEL_H_

#include "widget.h"
#include <string>

namespace GUI
{

/// Base class for tab panels.
class TabPanel: public Widget
{
public:
    virtual ~TabPanel() {}
    
    /// Add a new tab with the given name and contents.
    virtual void addTab(const std::string& name, Widget* contents)=0;
};

}

#endif
