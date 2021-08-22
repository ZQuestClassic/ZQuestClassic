#include "checkBox.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

namespace gui
{

CheckBox::CheckBox(): checked(false), text(),
    boxPlacement(BoxPlacement::right), alDialog()
{
    height=text_height(lfont_l);
    width=12;
}

void CheckBox::setText(std::string newText)
{
    width=text_length(lfont_l, newText.c_str())+12;
    text=std::move(newText);
}

void CheckBox::setBoxPlacement(BoxPlacement bp)
{
    boxPlacement=bp;
}

void CheckBox::setChecked(bool value)
{
    checked=value;
}

bool CheckBox::getChecked()
{
    return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void CheckBox::realize(DialogRunner& runner)
{
    runner.push(shared_from_this(), DIALOG {
        jwin_checkfont_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        checked ? D_SELECTED : 0, // flags
        static_cast<int>(boxPlacement), 0, // d1, d2,
        (void*)text.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
    alDialog=runner.getAllegroDialog();
}

}
