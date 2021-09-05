#include "switcher.h"
#include "dialogRunner.h"
#include "../zc_alleg.h"
#include <iostream>
namespace gui
{

Switcher::Switcher(): visible(0)
{}

void Switcher::add(std::shared_ptr<Widget> child)
{
    children.push_back({ child, 0 });
}

void Switcher::show(size_t index)
{
    if(index==visible)
        return;

    assert(index<children.size());

    // May have been set before being realized
    if(alDialog)
    {
        setChildVisible(visible, false);
        setChildVisible(index, true);
    }
    visible=index;
}

size_t Switcher::getVisible() const
{
    return visible;
}

void Switcher::setChildVisible(size_t index, bool visible)
{
    // XXX This could be problematic if there are multiple things
    // controlling the DIALOGs visibility...
    for(int i=index>0 ? children[index-1].end+1 : 1; i<=children[index].end; i++)
    {
        if(visible)
            alDialog[i].flags&=~D_HIDDEN;
        else
            alDialog[i].flags|=D_HIDDEN;
    }
}

void Switcher::calculateSize()
{
    // The switcher is as large as its largest child.
    int maxW=0, maxH=0;
    for(auto& child: children)
    {
        child.widget->calculateSize();
        int w=child.widget->getWidth();
        if(w>maxW)
            maxW=w;
        int h=child.widget->getHeight();
        if(w>maxH)
            maxH=h;
    }
    width=maxW;
    height=maxH;
}


void Switcher::arrange(int contX, int contY, int contW, int contH)
{
    for(auto& child: children)
        child.widget->arrange(contX, contY, contW, contH);
}

void Switcher::realize(DialogRunner& runner)
{
    // We'll just assume this isn't the first DIALOG.  The DialogRunner
    // itself inserts one, so that shouldn't be possible. Anyway, it should
    // work even if it is the first.
    alDialog=runner.getAllegroDialog();
    int size=runner.size();

    for(size_t i=0; i<children.size(); i++)
    {
        children[i].widget->realize(runner);
        int newSize=runner.size();
        children[i].end=newSize-size;
        std::cout<<"Child "<<i<<" ends at "<<children[i].end<<std::endl;
        if(i!=visible)
            setChildVisible(i, false);
    }
}

}
