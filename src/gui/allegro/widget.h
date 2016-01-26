#ifndef _ZC_GUI_ALLEGRO_WIDGET_H_
#define _ZC_GUI_ALLEGRO_WIDGET_H_

#include "../widget.h"
struct DIALOG;

namespace GUI
{

class AllegroDialogController;
class AllegroGUIRenderer;

/// Base class for dialog widgets implemented using Allegro DIALOGs.
class AllegroWidget: public Widget
{

public:
    virtual ~AllegroWidget() {}
    
    /// Get the dimensions this widget would prefer to use.
    /** prefWidth and prefHeight are both input and output. The values
     *  passed in are the maximum size available. The output values may
     *  still be higher, however.
     */
    virtual void getPreferredSize(int& prefWidth, int& prefHeight) {}
    
    virtual void onScrollH(int amount, int left, int right) {}
    virtual void onScrollV(int amount, int top, int bottom) {}
    
    /// Sets the size and position of this widget.
    /** Containers will arrange their contents accordingly. */
    virtual void setSizeAndPos(int x, int y, int width, int height)=0;
    
    /// Get the number of DIALOGs this widget uses.
    virtual int getDataSize() const { return 1; }
    
    /// Load DIALOG data into dlgArray[index].
    /** \return The number of DIALOGs used (the same number as getDataSize()). */
    virtual int realize(DIALOG dlgArray[], int index) { return 0; }
    
    /// Informs the widget that the DIALOG array is no longer valid.
    virtual void unrealize() {}
    
    /// Provides a pointer to the dialog controller.
    virtual void setController(AllegroDialogController* controller)=0;
    
    /// Whether this widget expands and contracts to fill the available width.
    virtual bool widthIsFlexible() const { return false; }
    
    /// Whether this widget expands and contracts to fill the available height.
    virtual bool heightIsFlexible() const { return false; }
    
    virtual void draw(AllegroGUIRenderer& renderer) const=0;
    
    virtual void setActive(bool val) {}
    
    virtual bool isActive() { return true; }
};

}

#endif
