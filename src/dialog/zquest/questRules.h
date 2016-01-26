#ifndef _ZC_DIALOG_ZQUEST_QUESTRULES_H_
#define _ZC_DIALOG_ZQUEST_QUESTRULES_H_

#include "../../gui/dialog.h"
#include <vector>

namespace GUI
{
    class Checkbox;
}

class QuestRulesEditor: public GUI::Dialog
{
private:
    std::vector<GUI::Checkbox*> checkboxes;
    
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    void onOK();
};

#endif
