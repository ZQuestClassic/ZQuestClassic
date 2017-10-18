#ifndef _GUI_SERIALCONTAINER_H_
#define _GUI_SERIALCONTAINER_H_

#include "widget.h"

namespace GUI
{

/// Base class for rows and columns.
class SerialContainer: public Widget
{
public:
    /// Add a new widget at the end of the container.
    virtual void add(Widget* comp)=0;
};

}

#endif
