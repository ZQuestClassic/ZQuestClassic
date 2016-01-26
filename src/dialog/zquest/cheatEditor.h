#ifndef _ZC_DIALOG_ZQUEST_CHEATEDITOR_H_
#define _ZC_DIALOG_ZQUEST_CHEATEDITOR_H_

#include "../../gui/dialog.h"

namespace GUI
{
    class Checkbox;
    class TextField;
}

class CheatEditor: public GUI::Dialog
{
private:
    GUI::Checkbox* enabledCB;
    GUI::TextField* code1;
    GUI::TextField* code2;
    GUI::TextField* code3;
    GUI::TextField* code4;
    
    GUI::Widget* createDialog(const GUI::WidgetFactory& f);
    void onOK();
};

#endif
