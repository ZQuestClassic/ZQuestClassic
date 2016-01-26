#include "buttonRow.h"
#include "util.h"

namespace GUI
{

ZCGtkButtonRow::ZCGtkButtonRow():
    box(0)
{
    box=gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_CENTER);
    gtk_box_set_spacing(GTK_BOX(box), 20);
    g_signal_connect(box, "destroy", G_CALLBACK(gtk_widget_destroyed), &box);
}

ZCGtkButtonRow::~ZCGtkButtonRow()
{
    for(int i=0; i<buttons.size(); i++)
        delete buttons[i];
}

void ZCGtkButtonRow::add(Widget* w)
{
    ZCGtkWidget* btn=dynamic_cast<ZCGtkWidget*>(w);
    buttons.push_back(btn);
    gtk_container_add(GTK_CONTAINER(box), btn->get());
}

void ZCGtkButtonRow::setController(ZCGtkController* c)
{
    ZCGtkWidget::setController(c);
    for(int i=0; i<buttons.size(); i++)
        buttons[i]->setController(c);
}

bool ZCGtkButtonRow::fillsWidth() const
{
    return true;
}

GtkWidget* ZCGtkButtonRow::get()
{
    return box;
}

}
