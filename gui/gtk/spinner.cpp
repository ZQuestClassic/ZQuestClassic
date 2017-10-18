#include "spinner.h"

namespace GUI
{

void ZCGtkSpinner::valueChanged(GtkSpinButton* widget, ZCGtkSpinner* spinner)
{
    if(spinner->onValueChanged)
        spinner->onValueChanged(gtk_spin_button_get_value_as_int(widget));
}

ZCGtkSpinner::ZCGtkSpinner(int minVal, int maxVal):
    spinner(0),
    signalHandlerID(0)
{
    spinner=gtk_spin_button_new_with_range(minVal, maxVal, 1);
    signalHandlerID=g_signal_connect(spinner, "value-changed", G_CALLBACK(valueChanged), this);
    g_signal_connect(spinner, "destroy", G_CALLBACK(gtk_widget_destroyed), &spinner);
}

void ZCGtkSpinner::setValue(int val)
{
    if(signalHandlerID>0)
        g_signal_handler_disconnect(spinner, signalHandlerID);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner), val);
    signalHandlerID=g_signal_connect(spinner, "value-changed", G_CALLBACK(valueChanged), this);
}

int ZCGtkSpinner::getValue() const
{
    return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner));
}

void ZCGtkSpinner::setOnValueChanged(boost::function<void(int)> func)
{
    onValueChanged=func;
}

GtkWidget* ZCGtkSpinner::get()
{
    return spinner;
}

}
