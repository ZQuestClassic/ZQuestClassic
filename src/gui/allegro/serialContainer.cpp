#include "serialContainer.h"

namespace GUI
{

AllegroSerialContainer::~AllegroSerialContainer()
{
    for(int i=0; i<contents.size(); i++)
        delete contents[i];
}

void AllegroSerialContainer::add(Widget* w)
{
    AllegroWidget* aw=dynamic_cast<AllegroWidget*>(w);
    contents.push_back(aw);
    int ww, wh;
    aw->getPreferredSize(ww, wh);
    contentSizes.push_back(std::make_pair(ww, wh));
}

int AllegroSerialContainer::getDataSize() const
{
    int total=0;
    for(int i=0; i<contents.size(); i++)
        total+=contents[i]->getDataSize();
    return total;
}

int AllegroSerialContainer::realize(DIALOG dlgArray[], int index)
{
    int added=0;
    for(int i=0; i<contents.size(); i++)
        added+=contents[i]->realize(dlgArray, index+added);
    return added;
}

void AllegroSerialContainer::unrealize()
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->unrealize();
}

void AllegroSerialContainer::setController(AllegroDialogController* controller)
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->setController(controller);
}

void AllegroSerialContainer::setActive(bool val)
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->setActive(val);
}

void AllegroSerialContainer::draw(AllegroGUIRenderer& renderer) const
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->draw(renderer);
}

void AllegroSerialContainer::onScrollH(int amount, int left, int right)
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->onScrollH(amount, left, right);
}

void AllegroSerialContainer::onScrollV(int amount, int top, int bottom)
{
    for(int i=0; i<contents.size(); i++)
        contents[i]->onScrollV(amount, top, bottom);
}

int AllegroSerialContainer::getGreatestContentWidth() const
{
    int ret=0;
    for(int i=0; i<contentSizes.size(); i++)
    {
        if(contentSizes[i].first>ret)
            ret=contentSizes[i].first;
    }
    return ret;
}

int AllegroSerialContainer::getGreatestContentHeight() const
{
    int ret=0;
    for(int i=0; i<contentSizes.size(); i++)
    {
        if(contentSizes[i].second>ret)
            ret=contentSizes[i].second;
    }
    return ret;
}

int AllegroSerialContainer::getTotalContentWidth() const
{
    int ret=0;
    for(int i=0; i<contentSizes.size(); i++)
        ret+=contentSizes[i].first;
    return ret;
}

int AllegroSerialContainer::getTotalContentHeight() const
{
    int ret=0;
    for(int i=0; i<contentSizes.size(); i++)
        ret+=contentSizes[i].second;
    return ret;
}


}
