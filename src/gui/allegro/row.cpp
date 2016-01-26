#include "row.h"
#include <allegro.h>

namespace GUI
{

AllegroRow::AllegroRow():
    targetWidth(0),
    numExpanders(0),
    padding(0)
{
}

void AllegroRow::add(Widget* w)
{
    AllegroSerialContainer::add(w);
    //if(dynamic_cast<AllegroWidget*>(w)->heightIsFlexible())
        //numExpanders++;
}

void AllegroRow::getPreferredSize(int& prefWidth, int& prefHeight)
{
    //prefWidth=getTotalContentWidth();
    //prefHeight=getGreatestContentHeight();
    
    prefWidth=0;
    prefHeight=0;
    numExpanders=0;
    padding=0;
    bool lastHadPadding=false;
    
    for(int i=0; i<contents.size(); i++)
    {
        AllegroWidget* c=contents[i];
        int tempW, tempH;
        c->getPreferredSize(tempW, tempH);
        prefWidth+=tempW;
        
        if(false) //c->wantsPadding())
        {
            if(lastHadPadding)
                prefWidth+=16;
            else
                prefWidth+=32;
            tempH+=32;
            lastHadPadding=true;
        }
        else
            lastHadPadding=false;
        
        if(tempH>prefHeight)
            prefHeight=tempH;
        
        if(c->widthIsFlexible())
            numExpanders++;
    }
    
    // If everything wants padding, don't add too much
    if(padding==contents.size())
        padding--;
    
    prefWidth+=padding;
    targetWidth=prefWidth;
}

void AllegroRow::setSizeAndPos(int newX, int newY, int newW, int newH)
{
    bool lastHadPadding=false;
    
    // Divide extra space evenly among expanding widgets, if there are any
    int extraSpace=(newW-targetWidth)/((numExpanders>0) ? numExpanders : contents.size());
    
    for(int i=0; i<contents.size(); i++)
    {
        AllegroWidget* c=contents[i];
        int compW, dummy;
        c->getPreferredSize(compW, dummy);
        if(numExpanders==0 || c->widthIsFlexible())
            compW+=extraSpace;
        
        if(false) //c->wantsPadding())
        {
            if(lastHadPadding)
            {
                c->setSizeAndPos(newX, newY, compW, newH);
                newX+=compW+16;
            }
            else
            {
                c->setSizeAndPos(newX+16, newY, compW, newH);
                newX+=compW+32;
            }
            lastHadPadding=true;
        }
        else
        {
            c->setSizeAndPos(newX, newY, compW, newH);
            newX+=compW;
            lastHadPadding=false;
        }
    }
}

}
