#include "standardWidget.h"
#include "common.h"
#include <allegro.h>
#include <algorithm>
#include <cstring>

#ifndef zc_max
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))
#endif

namespace GUI
{

StandardAllegroWidget::StandardAllegroWidget():
    dialog(0),
    controller(0),
    x(0),
    y(0),
    width(0),
    height(0),
    hidden(false)
{
}

void StandardAllegroWidget::setSizeAndPos(int newX, int newY,
  int newWidth, int newHeight)
{
    x=newX;
    y=newY;
    width=newWidth;
    height=newHeight;
    
    if(dialog)
    {
        dialog->x=newX;
        dialog->y=newY;
        dialog->w=newWidth;
        dialog->h=newHeight;
    }
}

int StandardAllegroWidget::realizeCommon(DIALOG dlgArray[], int index,
  dialogProc proc, int key)
{
    dialog=&dlgArray[index];
    dialog->proc=proc;
    dialog->x=x;
    dialog->y=y;
    dialog->w=width;
    dialog->h=height;
    dialog->key=key;
    if(hidden)
        dialog->flags|=D_HIDDEN;
    
    return 1;
}

void StandardAllegroWidget::onScrollH(int amount, int left, int right)
{
    x+=amount;
    if(dialog)
    {
        dialog->x=zc_max(x, left);
        if(dialog->x+width<right)
            dialog->w=width;
        else
            dialog->w=right-dialog->x; // may be negative; that's okay
    }
}

void StandardAllegroWidget::onScrollV(int amount, int top, int bottom)
{
    y+=amount;
    if(dialog)
    {
        dialog->y=zc_max(y, top);
        if(dialog->y+height<bottom)
            dialog->h=height;
        else
            dialog->h=bottom-dialog->y;
    }
}

void StandardAllegroWidget::unrealize()
{
    if(dialog)
    {
        std::memset(dialog, 0, sizeof(DIALOG));
        dialog->proc=dummyProc;
        dialog=0;
    }
}

void StandardAllegroWidget::setActive(bool val)
{
    hidden=!val;
    if(dialog)
    {
        if(val)
            dialog->flags&=~D_HIDDEN;
        else
            dialog->flags|=D_HIDDEN;
    }
    // else what?
}

bool StandardAllegroWidget::isActive()
{
    if(!dialog)
        return false;
    return (dialog->flags&D_HIDDEN)==0;
}

} // Namespace
