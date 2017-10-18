#ifndef _ZC_GUI_TEXTFIELD_H_
#define _ZC_GUI_TEXTFIELD_H_

#include "widget.h"
#include <string>

namespace GUI
{

/// Base class for text fields.
class TextField: public Widget
{
public:
    virtual ~TextField() {}
    
    /// Set the text field's current text.
    virtual void setText(const std::string& text)=0;
    
    /// Get the text field's current text.
    virtual std::string getText() const=0;
};

}

#endif
