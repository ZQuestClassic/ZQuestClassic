#include "textBox.h"
#include <iostream>
namespace GUI
{

ZCGtkTextBox::ZCGtkTextBox(float width, float height):
    textBox(0),
    scrollPane(0)
{
    textBox=gtk_text_view_new();
    g_signal_connect(textBox, "destroy", G_CALLBACK(gtk_widget_destroyed), &textBox);
    
    scrollPane=gtk_scrolled_window_new(
      gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(textBox)), // Unnecessary?
      gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(textBox)));
    gtk_widget_set_size_request(scrollPane, 800, 600);
    g_signal_connect(scrollPane, "destroy", G_CALLBACK(gtk_widget_destroyed), &scrollPane);
    gtk_container_add(GTK_CONTAINER(scrollPane), textBox);
}

void ZCGtkTextBox::setText(const std::string& text)
{
    GtkTextBuffer* buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textBox));
    gtk_text_buffer_set_text(buffer, text.c_str(), -1);
}

std::string ZCGtkTextBox::getText() const
{
    GtkTextBuffer* buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textBox));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
}

bool ZCGtkTextBox::fillsWidth() const
{
    return true;
}

bool ZCGtkTextBox::fillsHeight() const
{
    return true;
}

GtkWidget* ZCGtkTextBox::get()
{
    return scrollPane;
}

}
