#ifndef _ZC_GUI_GTK_CHECKBOX_H_
#define _ZC_GUI_GTK_CHECKBOX_H_

#include <gui/checkbox.h>
#include "widget.h"
#include <string>

namespace GUI
{

class ZCGtkCheckbox: public Checkbox, public ZCGtkWidget
{
public:
    ZCGtkCheckbox(const std::string& text);
    bool getValue() const;
    void setValue(bool newValue);
    void setCallback(boost::function<void(bool)> func);
    GtkWidget* get();
    
private:
    GtkWidget* checkbox;
    boost::function<void(bool)> onToggleFunc;
    
    static void onToggle(GtkToggleButton*, ZCGtkCheckbox* cb);
};

}

#endif
