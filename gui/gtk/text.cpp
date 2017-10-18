#include "text.h"
#include "util.h"

namespace GUI
{

ZCGtkText::ZCGtkText(const std::string& t)
{
    text=gtk_label_new(t.c_str());
    gtk_label_set_max_width_chars(GTK_LABEL(text), 50); // Maybe? We'll see how it goes.
    gtk_label_set_line_wrap(GTK_LABEL(text), TRUE);
    g_signal_connect(text, "destroy", G_CALLBACK(gtk_widget_destroyed), &text);
}

void ZCGtkText::setText(const std::string& newText)
{
    gtk_label_set_text(GTK_LABEL(text), newText.c_str());
}

GtkWidget* ZCGtkText::get()
{
    return text;
}

}
