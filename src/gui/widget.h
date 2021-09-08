#ifndef ZC_GUI_WIDGET_H
#define ZC_GUI_WIDGET_H

#include "common.h"
#include "dialogMessage.h"
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

    /* Set the widget's width, overriding its preferred size.
     * This should not be used by widgets.
     */
    void overrideWidth(Size newWidth);

    /* Set the widget's height, overriding its preferred size.
     * This should not be used by widgets.
     */
    void overrideHeight(Size newHeight);

    void setHPadding(int amount);
    void setVPadding(int amount);
    void setHAlign(float align);
    void setVAlign(float align);

    virtual void setVisible(bool visible);

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

    /* This function is called when an event occurs (e.g. a button is clicked
     * or a list selection is changed). It should send the appropriate message
     * through the provided function. event is a NewGuiEvent (jwin.h).
     * If this function returns a D_* value, it will immediately be returned
     * from the backend proc. If this returns -1, the proc will keep going.
     * This should not return D_EXIT. Instead, the dialog's message handler
     * should return true.
     */
    virtual int onEvent(int event, MessageDispatcher sendMessage);

    /* Returns the width of the widget. This should only be used by widgets. */
    int getWidth() const { return width; }

    /* Returns the height of the widget. This should only be used by widgets. */
    int getHeight() const { return height; }

    /* Returns the width of the widget plus any padding.
     * This should only be used by widgets.
     */
    int getTotalWidth() const { return width+hPadding; }

    /* Returns the height of the widget plus any padding.
     * This should only be used by widgets.
     */
    int getTotalHeight() const { return height+vPadding; }

protected:
    int x, y;
    int fgColor, bgColor;
    int hPadding, vPadding;
    float hAlign, vAlign;

    /* Sets the widget's width if it hasn't been overridden. */
    void setPreferredWidth(Size newWidth);

    /* Sets the widget's width if it hasn't been overridden. */
    void setPreferredHeight(Size newHeight);

    /* Returns flags with which the DIALOG should be initialized. */
    int getFlags();

private:
    enum {
        f_widthOverridden=  0b00000001,
        f_heightOverridden= 0b00000010,
        f_hidden=           0b00000100
    };
    int width, height;
    unsigned char flags;
};

}

#endif
