#include "dummy.h"
#include "common.h"
#include "renderer.h"

extern FONT* lfont;

namespace GUI
{

int DummyAllegroWidget::proc(int msg, DIALOG* d, int c)
{
    return dummyProc(msg, d, c);
}

DummyAllegroWidget::DummyAllegroWidget(int w, int h):
    width(w),
    height(h)
{
}

void DummyAllegroWidget::getPreferredSize(int& prefWidth, int& prefHeight)
{
    prefWidth=width;
    prefHeight=height;
}

int DummyAllegroWidget::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<DummyAllegroWidget>(dlgArray, index);
}

void DummyAllegroWidget::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.drawRect(0, 0, width, height, pal.winBorder);
    renderer.drawText("Dummy", 3, 3, lfont, pal.winBodyText);
}

}
