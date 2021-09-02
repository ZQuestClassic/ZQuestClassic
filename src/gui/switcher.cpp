#include "switcher.h"
#include "../zc_alleg.h"

namespace gui
{

Switcher::Switcher(): visible(0)
{}

void Switcher::add(std::shared_ptr<Widget> child)
{
    widgets.push_back(child);
}

void Switcher::show(size_t index)
{
    if(index==visible)
        return;

    assert(index<widgets.size());

    // May have been set before being realized
    if(dialogs.size()>index)
    {
        dialogs[visible]->flags|=D_HIDDEN;
        dialogs[index]->flags&=~D_HIDDEN;
    }
    visible=index;
}

void Switcher::calculateSize()
{
    int maxW=0, maxH=0;
    for(auto& widget: widgets)
    {
        widget->calculateSize();
        int w=widget->getWidth();
        if(w>maxW)
            maxW=w;
        int h=widget->getHeight();
        if(w>maxH)
            maxH=h;
    }
    width=maxW;
    height=maxH;
}


void Switcher::arrange(int contX, int contY, int contW, int contH)
{
    for(auto& widget: widgets)
        widget->arrange(contX, contY, contW, contH);
}

void Switcher::realize(DialogRunner& runner)
{
    dialogs.clear();
    dialogs.reserve(widgets.size());

    for(size_t i=0; i<widgets.size(); i++)
    {
        widgets[i]->realize(runner);
        auto dlg=runner.getAllegroDialog();
        if(i!=visible)
            dlg->flags|=D_HIDDEN;
        dialogs.push_back(dlg);
    }
}

}
