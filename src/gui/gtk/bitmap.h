#ifndef _ZC_GUI_GTK_BITMAP_H_
#define _ZC_GUI_GTK_BITMAP_H_

#include <gui/bitmap.h>
#include "widget.h"
#include <scoped_ptr.h>

namespace GUI
{

class ZCGtkBitmap: public BitmapWidget, public ZCGtkWidget 
{
public:
    ZCGtkBitmap(BitmapBackend* backend);
    ~ZCGtkBitmap();
    void setHandleMouseInput(bool handle);
    GtkWidget* get();
    
private:
    struct RedrawData
    {
        RedrawData(GtkWidget* w): valid(true), image(w) {}
        bool valid;
        GtkWidget* image;
    };
    
    GdkPixbuf* pixbuf;
    GtkWidget* image;
    GtkWidget* events;
    GtkWidget* frame;
    scoped_ptr<BitmapBackend> backend;
    bool handleMouseInput;
    gulong mousePressedHandler, mouseReleasedHandler, mouseMotionHandler,
      mouseWheelHandler;
    RedrawData* redrawData;
    guint redrawEventID;
    
    static gboolean redraw(GtkWidget* widget, cairo_t* cr, ZCGtkBitmap* bmp);
    static gboolean makeDirty(gpointer data);
    static gboolean mouseInput(GtkWidget* widget, GdkEvent* event, BitmapBackend* bb);
};

}

#endif
