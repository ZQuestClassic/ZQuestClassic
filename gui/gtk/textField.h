#ifndef _ZC_GUI_GTK_TEXTFIELD_H_
#define _ZC_GUI_GTK_TEXTFIELD_H_

#include <gui/textField.h>
#include "widget.h"

namespace GUI
{

class ZCGtkTextField: public TextField, public ZCGtkWidget
{
public:
    ZCGtkTextField(int maxLength);
    void setText(const std::string& text);
    std::string getText() const;
    bool fillsWidth() const;
    GtkWidget* get();
    
private:
    GtkWidget* textField;
    
};

}


#endif
