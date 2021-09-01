#include "checkbox.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

namespace gui
{

Checkbox::Checkbox(): checked(false), text(),
    boxPlacement(BoxPlacement::right), alDialog()
{
    height=text_height(lfont_l);
    width=12;
}

void Checkbox::setText(std::string newText)
{
    width=text_length(lfont_l, newText.c_str())+12;
    text=std::move(newText);
}

void Checkbox::setBoxPlacement(BoxPlacement bp)
{
    boxPlacement=bp;
}

void Checkbox::setChecked(bool value)
{
    checked=value;
}

bool Checkbox::getChecked()
{
    return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void Checkbox::realize(DialogRunner& runner)
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
