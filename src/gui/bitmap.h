#ifndef _ZC_GUI_BITMAP_H_
#define _ZC_GUI_BITMAP_H_

#include "widget.h"
#include "../guiBitmapRenderer.h"
#include "../scoped_ptr.h"
struct BITMAP;
struct MouseInput;

namespace GUI
{

class BitmapWidget: public Widget
{
public:
    virtual ~BitmapWidget() {}
    virtual void setHandleMouseInput(bool handle)=0;
};

class BitmapBackend
{
public:
    BitmapBackend(): widget(0) {}
    virtual ~BitmapBackend() {}
    
    /// Initializes the renderer and tells the widget what input to forward.
    virtual void initialize()=0; // const?
    
    /// Redraws the bitmap. Called 60 times per second.
    virtual void redraw()=0; // const?
    
    /// Called on mouse input if enabled.
    virtual void onMouseInput(const MouseInput& mouse) {}
    
    /// Returns the current scaling factor. Needed to adjust mouse input.
    // This might return a float or double at some point.
    inline int getScale() const
    {
        return renderer->getScale();
    }
    
    /// Returns an Allegro BITMAP.
    inline BITMAP* getBitmap()
    {
        return renderer->getBitmap();
    }
    
    /// Returns pixel data that can be used to construct a GdkPixbuf.
    inline void* getData()
    {
        return renderer->getData();
    }
    
    /// Gets the size of the bitmap in pixels.
    inline void getSize(int& width, int& height) const
    {
        renderer->getActualSize(width, height);
    }
    
    inline void setWidget(BitmapWidget* w)
    {
        widget=w;
    }
    
    inline void setRenderer(GUIBitmapRenderer* r)
    {
        renderer.reset(r);
    }
    
protected:
    BitmapWidget* widget;
    scoped_ptr<GUIBitmapRenderer> renderer;
};

}

#endif

