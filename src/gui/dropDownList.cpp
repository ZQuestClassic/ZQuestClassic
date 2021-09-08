#include "dropDownList.h"
#include "common.h"
#include "dialog.h"
#include "dialogRunner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>

#define FONT sized(nfont, lfont_l)
#define FONT_PTR sized(&nfont, &lfont_l)

namespace gui
{

DropDownList::DropDownList():
    jwinListWrapper(jwinListWrapperFunc, FONT_PTR, this),
    listData(nullptr), selectedIndex(0), selectedValue(0), message(-1)
{
    setPreferredWidth(20_em);
    if(is_large) // This has to be exactly right to look good
        setPreferredHeight(21_px);
    else
        setPreferredHeight(16_px);
    fgColor=jwin_pal[jcTEXTFG];
    bgColor=jwin_pal[jcTEXTBG];
}

void DropDownList::setListData(const ::gui::ListData& newListData)
{
    listData=&newListData;
}

void DropDownList::setSelectedValue(int value)
{
    selectedValue=value;
    selectedIndex=-1;
    if(alDialog)
    {
        setIndex();
        alDialog->d1=alDialog->d2=selectedIndex;
    }
}

void DropDownList::setSelectedIndex(int index)
{
    selectedIndex=index;
}

int DropDownList::getSelectedValue() const
{
    if(alDialog)
    {
        int index=alDialog->d1;
        return listData->value(index);
    }
    else
        return selectedValue;
}

void DropDownList::setIndex()
{
    // Find a valid selection. We'll take the first thing with a matching
    // value. If nothing matches exactly, take the one that's closest to
    // but not greater than the selected value. If everything's greater,
    // just go with index 0.
    selectedIndex=0;
    int bestSoFar=listData->value(0);
    for(auto i=0; i<listData->size(); i++)
    {
        int value=listData->value(i);
        if(value==selectedValue)
        {
            selectedIndex=i;
            return;
        }
        else if(value>bestSoFar && value<selectedValue)
            selectedIndex=i;
    }
}

void DropDownList::setVisible(bool visible)
{
    Widget::setVisible(visible);
    if(alDialog)
    {
        if(visible)
            alDialog->flags&=~D_HIDDEN;
        else
            alDialog->flags|=D_HIDDEN;
    }
}

const char* DropDownList::jwinListWrapperFunc(int index, int* size, void* owner)
{
    DropDownList* cb=static_cast<DropDownList*>(owner);
    if(index>=0)
        return cb->listData->listEntry(index).c_str();
    else
    {
        *size=cb->listData->size();
        return nullptr;
    }
}

void DropDownList::realize(DialogRunner& runner)
{
    // An empty list might logically be valid, but there's currently
    // no way to get a value from it.
    assert(listData);
    assert(listData->size()>0);
    if(selectedIndex<0)
        setIndex();

    alDialog=runner.push(shared_from_this(), DIALOG {
        jwin_droplist_proc,
        x, y, getWidth(), getHeight(),
        fgColor, bgColor,
        0, // key
        getFlags(), // flags
        selectedIndex, selectedIndex, // d1, d2,
        &jwinListWrapper, nullptr, nullptr // dp, dp2, dp3
    });
}

int DropDownList::onEvent(int event, MessageDispatcher sendMessage)
{
    assert(event==ngeCHANGE_SELECTION);
    if(message>=0)
        sendMessage(message, listData->value(alDialog->d1));
    return -1;
}

}
