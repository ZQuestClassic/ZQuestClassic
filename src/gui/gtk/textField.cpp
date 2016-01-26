#include "textField.h"
#include "util.h"

namespace GUI
{

ZCGtkTextField::ZCGtkTextField(int maxLength)
{
    textField=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(textField), maxLength);
    
    // TODO: Do something better with the size
    gtk_widget_set_size_request(textField, 256, -1);
    g_signal_connect(textField, "destroy", G_CALLBACK(gtk_widget_destroyed), &textField);
}

void ZCGtkTextField::setText(const std::string& text)
{
    gtk_entry_set_text(GTK_ENTRY(textField), text.c_str());
}

std::string ZCGtkTextField::getText() const
{
    return gtk_entry_get_text(GTK_ENTRY(textField));
}

bool ZCGtkTextField::fillsWidth() const
{
    return true;
}

GtkWidget* ZCGtkTextField::get()
{
    return textField;
}

}
