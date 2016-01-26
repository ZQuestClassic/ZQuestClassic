#ifndef _ZC_GUI_SPINNER_H_
#define _ZC_GUI_SPINNER_H_

#include "widget.h"
#include <boost/function.hpp>

namespace GUI
{

class Spinner: public Widget
{
public:
    virtual ~Spinner() {}
    
    /// Set the spinner's current value.
    /** If a function is set, it will not be called. */
    virtual void setValue(int val)=0;
    
    /// Get the spinner's current value.
    virtual int getValue() const=0;
    
    /// Set a function to call when the spinner's value changes.
    /** The new value will be the argument to the function. */
    virtual void setOnValueChanged(boost::function<void(int)> func)=0;
};

}

#endif

