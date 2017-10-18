#include "list.h"
#include "column.h"
#include "common.h"
#include "factory.h"
#include "renderer.h"
#include "text.h"

namespace GUI
{

AllegroList::AllegroList(const ListData* ld, const AllegroWidgetFactory& awf):
    data(ld),
    AllegroScrollingPane(dynamic_cast<AllegroWidget*>(awf.column())),
    widgetFactory(awf),
    selected(-1)
{
    int size=ld->size();
    for(int i=0; i<size; i++)
    {
        AllegroText* text=
          static_cast<AllegroText*>(widgetFactory.text(ld->get(i)));
        text->setList(this);
        text->setListNum(i);
        // Ambiguous base class...
        static_cast<AllegroColumn*>(content)->add(
          static_cast<StandardAllegroWidget*>(text));
    }
    
    if(size>0)
        setSelected(0);
}

AllegroList::~AllegroList()
{
    delete data;
}

void AllegroList::setSelected(int num)
{
    if(selected==num)
        return;
    
    AllegroColumn* col=static_cast<AllegroColumn*>(content);
    AllegroText* text;
    
    if(selected>=0)
    {
        text=static_cast<AllegroText*>(col->getWidget(selected));
        text->setSelected(false);
    }
    
    selected=num;
    text=static_cast<AllegroText*>(col->getWidget(num));
    text->setSelected(true);
}

void AllegroList::changeSelection(int num)
{
    setSelected(num);
    if(onSelectionChanged)
        onSelectionChanged(num);
}

void AllegroList::doubleClick(int num)
{
    // The selection should already be set from the first click
    if(onDoubleClick)
        onDoubleClick(num);
}

void AllegroList::getPreferredSize(int& prefWidth, int& prefHeight)
{
    AllegroScrollingPane::getPreferredSize(prefWidth, prefHeight);
    prefHeight=120;
    if(prefWidth>400)
        prefWidth=400;
}

void AllegroList::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.fillRect(0, 0, width-7, height, pal.editBG);
    renderer.drawRect(0, 0, width-7, height, pal.winBodyText);
    AllegroScrollingPane::draw(renderer);
}

}
