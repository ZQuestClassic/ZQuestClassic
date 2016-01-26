#include "widget.h"

namespace GUI
{

ZCGtkWidget::ZCGtkWidget():
    controller(0)
{
}

ZCGtkWidget::~ZCGtkWidget()
{
}

bool ZCGtkWidget::fillsWidth() const
{
    return false;
}

bool ZCGtkWidget::fillsHeight() const
{
    return false;
}

}
