#include "comboBox.h"
#include "common.h"
#include "controller.h"
#include "factory.h"
#include "list.h"
#include "renderer.h"
#include "text.h"
#include <allegro.h>
#include <boost/bind.hpp>

namespace GUI
{

int AllegroComboBox::proc(int msg, DIALOG* d, int c)
{
    AllegroComboBox* cb=static_cast<AllegroComboBox*>(d->dp);
    
    switch(msg)
    {
    case MSG_LPRESS:
        cb->onClick();
        break;
    }
    
    return D_O_K;
}

AllegroComboBox::AllegroComboBox(const ListData* ld, const AllegroWidgetFactory& awf):
    list(static_cast<AllegroList*>(awf.list(ld))),
    text(static_cast<AllegroText*>(awf.text(""))),
    data(ld),
    selected(-1)
{
    list->setOnValueChanged(boost::bind(&AllegroComboBox::selectionChanged, this, _1));
}

AllegroComboBox::~AllegroComboBox()
{
    delete list; // This will also delete the ListData
    delete text;
}

void AllegroComboBox::onClick()
{
    while(gui_mouse_b())
        // Don't open the list immediately or it might get clicked
        rest(1);
    controller->beginSubDialog(list);
}

void AllegroComboBox::selectionChanged(int newSel)
{
    selected=newSel;
    while(gui_mouse_b())
        // Don't let whatever was behind the list get clicked
        rest(1);
    controller->closeSubDialog();
    text->setText(data->get(newSel));
    if(onValueChanged)
        onValueChanged(newSel);
}

void AllegroComboBox::getPreferredSize(int& prefWidth, int& prefHeight)
{
    // Ask the list so it can do whatever calculation it wants...
    list->getPreferredSize(prefWidth, prefHeight);
    
    // And ignore the result, because it's a drop-down list
    text->getPreferredSize(prefWidth, prefHeight);
    if(prefWidth>116)
        prefWidth=120;
    else
        prefWidth+=4;
    prefHeight+=4;
}

int AllegroComboBox::getDataSize() const
{
    // Don't include the list; it runs as a sub-dialog
    return text->getDataSize()+1;
}

int AllegroComboBox::realize(DIALOG dlgArray[], int index)
{
    // Same here
    int used=realizeAs<AllegroComboBox>(dlgArray, index);
    used+=text->realize(dlgArray, index+used);
    return used;
}

void AllegroComboBox::setController(AllegroDialogController* c)
{
    StandardAllegroWidget::setController(c);
    text->setController(c);
    list->setController(c);
}

void AllegroComboBox::setSizeAndPos(int x, int y, int width, int height)
{
    int dummyW=width, dummyH=height;
    text->getPreferredSize(dummyW, dummyH);
    adjustSizeAndPos(y, height, dummyH+4);
    StandardAllegroWidget::setSizeAndPos(x, y, width, height);
    text->setSizeAndPos(x+2, y+2, width-4, height-4);
    list->setSizeAndPos(x, y+height, width, 80);
}

void AllegroComboBox::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.fillRect(0, 0, width, height, pal.editBG);
    renderer.drawRect(0, 0, width, height, pal.winBodyText);
    renderer.fillTriangle(width-14, 4, width-2, 4, width-8, height-4, pal.editText);
    renderer.pushClipRect(2, 2, width-16, height-4);
    text->draw(renderer);
    renderer.popClipRect();
}

void AllegroComboBox::setSelected(int num)
{
    selected=num;
    text->setText(data->get(num));
}

}



