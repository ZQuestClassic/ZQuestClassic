#include "cheatEditor.h"
#include "../../gui/factory.h"
#include "../../zdefs.h" // for ZCHEATS
#include <boost/bind.hpp>
#include <cstring>

extern ZCHEATS zcheats;
extern bool saved;

#define CB(func) boost::bind(&CheatEditor::func, this)

GUI::Widget* CheatEditor::createDialog(const GUI::WidgetFactory& f)
{
    GUI::Window* win=f.window("Cheat codes",
      f.column(6,
        enabledCB=f.checkbox("Cheats &enabled"),
        f.row(2, f.text("Level 1"), code1=f.textField(40)),
        f.row(2, f.text("Level 2"), code2=f.textField(40)),
        f.row(2, f.text("Level 3"), code3=f.textField(40)),
        f.row(2, f.text("Level 4"), code4=f.textField(40)),
        f.buttonRow(2,
          f.button("O&K", CB(onOK)),
          f.button("&Cancel", CB(shutDown))
        )
      )
    );
    
    enabledCB->setValue((zcheats.flags&1)!=0);
    code1->setText(zcheats.codes[0]);
    code2->setText(zcheats.codes[1]);
    code3->setText(zcheats.codes[2]);
    code4->setText(zcheats.codes[3]);
    
    return win;
}

void CheatEditor::onOK()
{
    zcheats.flags=enabledCB->getValue() ? 1 : 0;
    std::strcpy(zcheats.codes[0], code1->getText().c_str());
    std::strcpy(zcheats.codes[1], code2->getText().c_str());
    std::strcpy(zcheats.codes[2], code3->getText().c_str());
    std::strcpy(zcheats.codes[3], code4->getText().c_str());
    
    shutDown();
    saved = false;
}
