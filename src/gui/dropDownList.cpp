#include "dropDownList.h"
#include "../jwin.h"
#include "../zquest.h"

namespace gui
{

DropDownList::DropDownList(): jwinListWrapper(jwinListWrapperFunc, &lfont_l, this),
    listData(nullptr), selectedIndex(0), selectedValue(0), message(0)
{
    width=200;
    height=text_height(lfont_l)+8;
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

const char* DropDownList::jwinListWrapperFunc(int index, int* size, void* owner)
{
    DropDownList* cb=static_cast<DropDownList*>(owner);
    if(index>=0)
    {
        return cb->listData->listEntry(index).c_str();
    }
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
                break;
            }
            else if(value>bestSoFar && value<selectedValue)
                selectedIndex=i;
        }
    }

    runner.push(shared_from_this(), DIALOG {
        jwin_droplist_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        message>=0 ? D_EXIT : 0, // flags
        selectedIndex, 0, // d1, d2,
        &jwinListWrapper, nullptr, nullptr // dp, dp2, dp3
    });
    alDialog=runner.getAllegroDialog();
}

int DropDownList::getMessage()
{
    return message;
}

}
