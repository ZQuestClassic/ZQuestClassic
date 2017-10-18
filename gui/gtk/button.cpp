#include "button.h"
#include "util.h"

namespace GUI
{

void ZCGtkButton::onClick(GtkButton*, ZCGtkButton* btn)
{
    btn->onClickFunc();
}

ZCGtkButton::ZCGtkButton(const std::string& text, boost::function<void(void)> c):
    onClickFunc(c)
{
    std::string mod=convertMnemonic(text);
    button=gtk_button_new_with_mnemonic(mod.c_str());
    gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(button, GTK_ALIGN_START);
    g_signal_connect(button, "destroy", G_CALLBACK(gtk_widget_destroyed), &button);
    g_signal_connect(button, "clicked", G_CALLBACK(onClick), this);
}

GtkWidget* ZCGtkButton::get()
{
    return button;
}

}
