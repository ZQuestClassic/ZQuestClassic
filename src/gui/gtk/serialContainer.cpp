#include "serialContainer.h"
#include "util.h"

namespace GUI
{

namespace
{

// Used to fix space allocation when an expanding widget is added to a container
// that previously contained only fixed-size widgets.
void repackChildren(GtkWidget* child, gpointer box)
{
    gtk_box_set_child_packing(GTK_BOX(box), child, FALSE, FALSE, 0, GTK_PACK_START);
}

}

ZCGtkSerialContainer::ZCGtkSerialContainer(bool v, bool es):
    vertical(v),
    evenlySpaced(es),
    hasExpanders(false)
{
    box=gtk_box_new(v ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL, 5);
    g_signal_connect(box, "destroy", G_CALLBACK(gtk_widget_destroyed), &box);
    gtk_box_set_homogeneous(GTK_BOX(box), es);
}

ZCGtkSerialContainer::~ZCGtkSerialContainer()
{
    for(int i=0; i<contents.size(); i++)
        delete contents[i];
}

void ZCGtkSerialContainer::add(Widget* newWidget)
{
    ZCGtkWidget* w=dynamic_cast<ZCGtkWidget*>(newWidget);
    contents.push_back(w);
    
    gboolean isExpander=vertical ? w->fillsHeight() : w->fillsWidth();
    GtkWidget* child=w->get();
    
    // If any widgets in the container expand to fill available space,
    // all extra space should be allocated to those widgets. Otherwise,
    // extra space should be distributed evenly if this is an evenly-
    // spaced container and unused otherwise.
    
    if(evenlySpaced)
        gtk_box_pack_start(GTK_BOX(box), child, TRUE, isExpander, 0);
    else if(hasExpanders)
        gtk_box_pack_start(GTK_BOX(box), child, isExpander, isExpander, 0);
    else
    {
        if(isExpander)
        {
            hasExpanders=true;
            gtk_container_foreach(GTK_CONTAINER(box), repackChildren, box);
            gtk_box_pack_start(GTK_BOX(box), child, TRUE, TRUE, 0);
        }
        else
            gtk_box_pack_start(GTK_BOX(box), child, FALSE, FALSE, 0);
    }
}

void ZCGtkSerialContainer::setController(ZCGtkController* c)
{
    ZCGtkWidget::setController(c);
    for(int i=0; i<contents.size(); i++)
        contents[i]->setController(c);
}

bool ZCGtkSerialContainer::fillsWidth() const
{
    return true;
}

bool ZCGtkSerialContainer::fillsHeight() const
{
    return true;
}

GtkWidget* ZCGtkSerialContainer::get()
{
    return box;
}

}
