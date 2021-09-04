#ifndef ZC_GUI_DROPDOWNLIST_H
#define ZC_GUI_DROPDOWNLIST_H

#include "widget.h"
#include "dialogRef.h"
#include "listData.h"
#include "../jwin.h"
#include <vector>

namespace gui
{

class DialogRunner;

class DropDownList: public Widget
{
public:
    DropDownList();
    void setListData(const ::gui::ListData& newListData);
    void setSelectedValue(int value);
    void setSelectedIndex(int index);
    int getSelectedValue() const;
    template<typename T>
    void onSelectionChanged(T m)
    {
        message=static_cast<int>(m);
    }

private:
    // A bit ugly because there was already a ListData struct in jwin
    ::ListData jwinListWrapper;
    const ::gui::ListData* listData;
    int selectedIndex, selectedValue;
    DialogRef alDialog;
    int message;

    // If a value was set rather than an index, find an index to select.
    void setIndex();
    void realize(DialogRunner& runner) override;
    int onEvent(int event, MessageDispatcher sendMessage) override;
    static const char* jwinListWrapperFunc(int index, int* size, void* owner);
};

}

#endif
