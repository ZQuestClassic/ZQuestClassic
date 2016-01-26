#ifndef _ZC_GUI_GTK_SPINNER_H_
#define _ZC_GUI_GTK_SPINNER_H_

#include <gui/spinner.h>
#include "widget.h"

namespace GUI
{

class ZCGtkSpinner: public Spinner, public ZCGtkWidget
{
public:
    ZCGtkSpinner(int minVal, int maxVal);
    void setValue(int val);
    int getValue() const;
    void setOnValueChanged(boost::function<void(int)> func);
    GtkWidget* get();
    
private:
    GtkWidget* spinner;
    boost::function<void(int)> onValueChanged;
    gulong signalHandlerID;
    
    static void valueChanged(GtkSpinButton* widget, ZCGtkSpinner* spinner);
};

}


#endif

