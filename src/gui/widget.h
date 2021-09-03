#ifndef ZC_GUI_WIDGET_H
#define ZC_GUI_WIDGET_H

#include "dialogEvent.h"
#include "../zc_alleg.h"
#include <memory>
#include <vector>

namespace gui
{

class DialogRunner;

class Widget: public std::enable_shared_from_this<Widget>
{
public:
    Widget();
    virtual ~Widget() {}

    void setHPadding(int amount);
    void setVPadding(int amount);
    void setHAlign(float align);
    void setVAlign(float align);

    /* Set the widget's width and height to their preferred values.
     * This doesn't need to be implemented if they're set already
     * before the dialog is realized.
     */
    virtual void calculateSize() {}

    /* Size and position the widget within the available space.
     * This is very quick and dirty; all of the sizing and positioning stuff
     * needs a lot of work.
     */
    virtual void arrange(int contX, int contY, int contW, int contH);

    /* Creates DIALOGs for this widget and any children. */
    virtual void realize(DialogRunner& runner)=0;

    /* Returns the message this type sends on an event cast to an int.
     * The default value when no message is set should be negative.
     */
    virtual int getMessage();

    /* This function is called when an event occurs (e.g. a button is clicked
     * or a list selection is changed). It should send the appropriate message
     * through the provided function. event is a NewGuiEvent (jwin.h).
     * If this function returns a D_* value, it will immediately be returned
     * from the backend proc. If this returns -1, the proc will keep going.
     */
    virtual int onEvent(int event, MessageDispatcher sendMessage);

    int getTotalWidth() const { return width+hPadding; }
    int getTotalHeight() const { return height+vPadding; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // A more convenient name for events with no arguments.
    static constexpr EventArg noArg=std::monostate();

    int x, y, width, height;
    int fgColor, bgColor;
    int hPadding, vPadding;
    float hAlign, vAlign;
};

}

#endif
