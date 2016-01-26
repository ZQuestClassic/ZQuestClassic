#include "frame.h"
#include "util.h"

namespace GUI
{

ZCGtkFrame::ZCGtkFrame(const std::string& label, ZCGtkWidget* content)
{
    if(label!="")
        frame=gtk_frame_new(label.c_str());
    else
        frame=gtk_frame_new(0);
    g_signal_connect(frame, "destroy", G_CALLBACK(gtk_widget_destroyed), &frame);
    gtk_container_add(GTK_CONTAINER(frame), content->get());
}

GtkWidget* ZCGtkFrame::get()
{
    return frame;
}

}
