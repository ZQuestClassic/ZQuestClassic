#ifndef _ZC_GUI_TEXTBOX_H_
#define _ZC_GUI_TEXTBOX_H_

#include "widget.h"
#include <string>

namespace GUI
{
    
class TextBox: public Widget
{
public:
    virtual ~TextBox() {}
    virtual void setText(const std::string& text)=0;
    virtual std::string getText() const=0;
};

}

#endif
