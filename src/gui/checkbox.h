#ifndef _ZC_GUI_CHECKBOX_H_
#define _ZC_GUI_CHECKBOX_H_

#include "widget.h"
#include <boost/function.hpp>

namespace GUI
{

/// Base class for checkboxes.
class Checkbox: public Widget
{
public:
    virtual ~Checkbox() {}
    
    /// Get the current state of the checkbox.
    virtual bool getValue() const=0;
    
    /// Check or uncheck the checkbox.
    virtual void setValue(bool newValue)=0;
    
    /// Set a function to be called when the box is checked or unchecked.
    virtual void setCallback(boost::function<void(bool)> func)=0;
};

}

#endif
