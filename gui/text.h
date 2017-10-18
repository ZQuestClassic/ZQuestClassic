#ifndef _ZC_GUI_TEXT_H_
#define _ZC_GUI_TEXT_H_

#include "widget.h"
#include <string>

namespace GUI
{

/// Base class for text.
class Text: public Widget
{
public:
    virtual ~Text() {}
    
    /// Set the text to be displayed.
    virtual void setText(const std::string& newText)=0;
};

}

#endif
