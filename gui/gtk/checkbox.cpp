#include "checkbox.h"
#include "util.h"

namespace GUI
{

void ZCGtkCheckbox::onToggle(GtkToggleButton* gtb, ZCGtkCheckbox* cb)
{
    if(cb->onToggleFunc)
        cb->onToggleFunc(gtk_toggle_button_get_active(gtb));
}

ZCGtkCheckbox::ZCGtkCheckbox(const std::string& text)
{
    std::string mod=convertMnemonic(text);
    checkbox=gtk_check_button_new_with_mnemonic(mod.c_str());
    g_signal_connect(checkbox, "destroy", G_CALLBACK(gtk_widget_destroyed), &checkbox);
    g_signal_connect(checkbox, "toggled", G_CALLBACK (onToggle), this);
}

bool ZCGtkCheckbox::getValue() const
{
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox));
}

void ZCGtkCheckbox::setValue(bool newValue)
{
    gtk_toggle_button_set_active(
      GTK_TOGGLE_BUTTON(checkbox),
      newValue ? TRUE : FALSE);
}

void ZCGtkCheckbox::setCallback(boost::function<void(bool)> func)
{
    onToggleFunc=func;
}

GtkWidget* ZCGtkCheckbox::get()
{
    return checkbox;
}

}
