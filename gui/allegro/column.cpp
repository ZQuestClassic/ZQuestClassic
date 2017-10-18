#include "column.h"
#include <allegro.h>

namespace GUI
{

AllegroColumn::AllegroColumn():
    targetHeight(0),
    numExpanders(0),
    padding(0)
{
}

void AllegroColumn::add(Widget* w)
{
    AllegroSerialContainer::add(w);
    //if(dynamic_cast<AllegroWidget*>(w)->heightIsFlexible())
        //numExpanders++;
}

void AllegroColumn::getPreferredSize(int& prefWidth, int& prefHeight)
{
    //prefWidth=getGreatestContentWidth();
    //prefHeight=getTotalContentHeight();
    
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
        prefHeight+=tempH;
        
        if(false) //c->wantsPadding())
        {
            if(lastHadPadding)
                prefHeight+=16;
            else
                prefHeight+=32;
            tempW+=32;
            lastHadPadding=true;
        }
        else
            lastHadPadding=false;
        
        if(tempW>prefWidth)
            prefWidth=tempW;
        
        if(c->heightIsFlexible())
            numExpanders++;
    }
    
    // If everything wants padding, don't add too much
    if(padding==contents.size())
        padding--;
    
    prefHeight+=padding;
    targetHeight=prefHeight;
}

void AllegroColumn::setSizeAndPos(int newX, int newY, int newW, int newH)
{
    bool lastHadPadding=false;
    
    // Divide extra space evenly among expanding widgets
    int extraSpace=(newH-targetHeight)/((numExpanders>0) ? numExpanders : contents.size());
    
    for(int i=0; i<contents.size(); i++)
    {
        AllegroWidget* c=contents[i];
        int compH, dummy;
        c->getPreferredSize(dummy, compH);
        if(numExpanders==0 || c->heightIsFlexible())
            compH+=extraSpace;
        
        if(false) //c->wantsPadding())
        {
            if(lastHadPadding)
            {
                c->setSizeAndPos(newX, newY, newW, compH);
                newY+=compH+16;
            }
            else
            {
                c->setSizeAndPos(newX, newY+16, newW, compH);
                newY+=compH+32;
            }
            lastHadPadding=true;
        }
        else
        {
            c->setSizeAndPos(newX, newY, newW, compH);
            newY+=compH;
            lastHadPadding=false;
        }
    }
}

}
