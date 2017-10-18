#include "bitmap.h"
#include "util.h"
#include <gui/mouse.h>

namespace GUI
{

gboolean ZCGtkBitmap::redraw(GtkWidget* widget, cairo_t* cr, ZCGtkBitmap* bmp)
{
    bmp->backend->redraw();
    GtkStyleContext* ctx=gtk_widget_get_style_context(widget);
    gtk_render_icon(ctx, cr, bmp->pixbuf, 0, 0);
    return TRUE;
}

gboolean ZCGtkBitmap::mouseInput(GtkWidget*, GdkEvent* event, BitmapBackend* bb)
{
    // The return value shouldn't make any difference; the bitmap will be
    // the only thing under the mouse that might be interested in it.
    MouseInput mi;
    if(translateMouseEvent(event, bb->getScale(), mi))
    {
        bb->onMouseInput(mi);
        return TRUE;
    }
    return FALSE;
}

gboolean ZCGtkBitmap::makeDirty(gpointer data)
{
    RedrawData* rd=static_cast<RedrawData*>(data);
    if(rd->valid)
    {
        gtk_widget_queue_draw(rd->image);
        return TRUE;
    }
    else
    {
        delete rd;
        return FALSE;
    }
}

ZCGtkBitmap::ZCGtkBitmap(BitmapBackend* be):
    backend(be),
    pixbuf(0),
    image(0),
    events(0),
    frame(0),
    handleMouseInput(false),
    mousePressedHandler(0),
    mouseReleasedHandler(0),
    mouseMotionHandler(0),
    redrawData(0),
    redrawEventID(0)
{
    backend->setWidget(this);
    backend->initialize();
    
    int width, height;
    guchar* data=static_cast<guchar*>(backend->getData());
    backend->getSize(width, height);
    
    pixbuf=gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, FALSE, 8,
      width, height, width*3, 0, 0);
    image=gtk_image_new_from_pixbuf(pixbuf);
    g_signal_connect(image, "destroy", G_CALLBACK(gtk_widget_destroyed), &image);
    
    redrawData=new RedrawData(image);
    g_signal_connect(image, "draw", G_CALLBACK(redraw), this);
    // Redraw every 17 ms. Not exactly 60 FPS, but it should do.
    redrawEventID=g_timeout_add(17, makeDirty, redrawData);
    
    events=gtk_event_box_new();
    gtk_widget_add_events(events,
      GDK_POINTER_MOTION_MASK|GDK_SCROLL_MASK|
      GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(events, "destroy", G_CALLBACK(gtk_widget_destroyed), &events);
    
    frame=gtk_frame_new(0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_halign(frame, GTK_ALIGN_START);
    gtk_widget_set_valign(frame, GTK_ALIGN_START);
    g_signal_connect(frame, "destroy", G_CALLBACK(gtk_widget_destroyed), &frame);
    
    gtk_container_add(GTK_CONTAINER(events), image);
    gtk_container_add(GTK_CONTAINER(frame), events);
    
    if(handleMouseInput)
    {
        mousePressedHandler=g_signal_connect(events,
          "button-press-event",G_CALLBACK(mouseInput), backend.get());
        mouseReleasedHandler=g_signal_connect(events,
          "button-release-event", G_CALLBACK(mouseInput), backend.get());
        mouseMotionHandler=g_signal_connect(events,
          "motion-notify-event", G_CALLBACK(mouseInput), backend.get());
        mouseWheelHandler=g_signal_connect(events,
          "scroll-event", G_CALLBACK(mouseInput), backend.get());
    }
}

ZCGtkBitmap::~ZCGtkBitmap()
{
    // This should probably be handled when the image is destroyed,
    // but I had serious problems getting that to work.
    // I don't think locking is needed here, but I'm not certain.
    if(redrawEventID>0)
    {
        if(events)
            g_source_remove(redrawEventID);
        redrawEventID=0;
    }
    if(redrawData)
        redrawData->valid=false;
}

void ZCGtkBitmap::setHandleMouseInput(bool handle)
{
    handleMouseInput=handle;
    
    if(events)
    {
        // Does setting this repeatedly cause problems?
        // Will that ever happen, anyway?
        if(handle)
        {
            mousePressedHandler=g_signal_connect(events,
              "button-press-event",G_CALLBACK(mouseInput), backend.get());
            mouseReleasedHandler=g_signal_connect(events,
              "button-release-event", G_CALLBACK(mouseInput), backend.get());
            mouseMotionHandler=g_signal_connect(events,
              "motion-notify-event", G_CALLBACK(mouseInput), backend.get());
            mouseWheelHandler=g_signal_connect(events,
              "scroll-event", G_CALLBACK(mouseInput), backend.get());
        }
        else
        {
            if(mousePressedHandler!=0)
            {
                g_signal_handler_disconnect(events, mousePressedHandler);
                mousePressedHandler=0;
            }
            if(mouseReleasedHandler!=0)
            {
                g_signal_handler_disconnect(events, mouseReleasedHandler);
                mouseReleasedHandler=0;
            }
            if(mouseMotionHandler!=0)
            {
                g_signal_handler_disconnect(events, mouseMotionHandler);
                mouseMotionHandler=0;
            }
            if(mouseWheelHandler!=0)
            {
                g_signal_handler_disconnect(events, mouseWheelHandler);
                mouseWheelHandler=0;
            }
        }
    }
}

GtkWidget* ZCGtkBitmap::get()
{
    return frame;
}

}
