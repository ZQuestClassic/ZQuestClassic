#include "label.h"
#include "dialog.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

namespace gui
{

Label::Label(): text()
{
    height=text_height(lfont_l);
}

void Label::setText(std::string newText)
{
    width=text_length(lfont_l, newText.c_str());
    text=std::move(newText);
}

void Label::realize(DialogRunner& runner)
{
    runner.push(shared_from_this(), DIALOG {
        jwin_text_proc,
        x, y, width, height,
        fgColor, bgColor,
        0, // key
        0, // flags
        0, 0, // d1, d2
        (void*)text.c_str(), (void*)lfont_l, nullptr // dp, dp2, dp3
    });
}

}
