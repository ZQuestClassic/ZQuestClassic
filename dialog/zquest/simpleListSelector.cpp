#include "simpleListSelector.h"
#include <gui/factory.h>
#include <boost/bind.hpp>

SimpleListSelector::SimpleListSelector(const std::string& t,
  const boost::function<std::string(int)>& strF, int num, bool em, int init):
    editMode(em),
    title(t),
    stringFunc(strF),
    numItems(num),
    selected(-1),
    list(0),
    initialSelection(init)
{
}

SimpleListSelector::SimpleListSelector(const std::string& t,
  const boost::function<std::string(int)>& strF, int num,
  const boost::function<void(int)>& helpF, bool em, int init):
    editMode(em),
    title(t),
    stringFunc(strF),
    helpFunc(helpF),
    numItems(num),
    selected(-1),
    list(0),
    initialSelection(init)
{
}

#define CB(func) boost::bind(&SimpleListSelector::func, this)
#define CB1(func) boost::bind(&SimpleListSelector::func, this, _1)

GUI::Widget* SimpleListSelector::createDialog(const GUI::WidgetFactory& f)
{
    GUI::SerialContainer* buttons=f.buttonRow();
    buttons->add(f.button(editMode ? "&Edit" : "O&K", CB(onOK)));
    if(helpFunc)
        buttons->add(f.button("Help", CB(onHelp)));
    buttons->add(f.button(editMode ? "&Done" : "&Cancel", CB(onCancel)));
    
    GUI::Window* win=f.window(title,
      f.column(GUI::Contents(
        list=f.list(new GUI::ListData(stringFunc, CB(getNumItems))),
        buttons
      ))
    );
    
    list->setSelected(initialSelection);
    list->setOnDoubleClick(CB1(onDoubleClick));
    selected=-1;
    
    return win;
}

void SimpleListSelector::onDoubleClick(int item)
{
    onOK();
}

void SimpleListSelector::onOK()
{
    selected=list->getSelected();
    shutDown();
}

void SimpleListSelector::onCancel()
{
    selected=-1;
    shutDown();
}

void SimpleListSelector::onHelp()
{
    helpFunc(list->getSelected());
}

int SimpleListSelector::getNumItems() const
{
    return numItems;
}
