#ifndef ZC_GUI_WIDGET_H
#define ZC_GUI_WIDGET_H

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

    int getTotalWidth() const { return width+hPadding; }
    int getTotalHeight() const { return height+vPadding; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    int x, y, width, height;
    int fgColor, bgColor;
    int hPadding, vPadding;
    float hAlign, vAlign;
};

}

#endif
