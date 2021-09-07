#include "widget.h"
#include "common.h"
#include "../jwin.h"
#include "../zdefs.h"
#include <cassert>

namespace gui
{

Widget::Widget(): x(0), y(0), width(32), height(32),
    fgColor(vc(14)), bgColor(vc(1)),
    hPadding(sized(2, 3)), vPadding(sized(2, 3)), hAlign(0.5), vAlign(0.5)
{}

void Widget::setHPadding(int amount)
{
    hPadding=amount;
}

void Widget::setVPadding(int amount)
{
    vPadding=amount;
}

void Widget::setHAlign(float align)
{
    hAlign=align;
}

void Widget::setVAlign(float align)
{
    vAlign=align;
}

int Widget::onEvent(int, MessageDispatcher)
{
    assert(false); // This function should have been overridden
    return -1;
}

void Widget::arrange(int contX, int contY, int contW, int contH)
{
    if(width>contW)
        width=contW;
    if(height>contH)
        height=contH;

    auto hExcess=contW-width;
    x=contX+hExcess*hAlign;
    auto vExcess=contH-height;
    y=contY+vExcess*vAlign;
}

}
