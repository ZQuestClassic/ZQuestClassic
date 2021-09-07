#include "label.h"
#include "common.h"
#include "dialog.h"
#include "dialogRunner.h"
#include "../jwin.h"
#include "../zquest.h"
#include <utility>

#define FONT sized(nfont, lfont_l)

namespace gui
{

Label::Label(): text()
{
    setPreferredHeight(Size::pixels(text_height(FONT)));
}

void Label::setText(std::string newText)
{
    int oldX, oldW;
    if(alDialog)
    {
        oldX=alDialog->x;
        oldW=alDialog->w;
    }

    int textW=text_length(FONT, newText.c_str());
    setPreferredWidth(Size::pixels(textW));
    text=std::move(newText);

    if(alDialog)
    {
        // TODO This assumes right-alignment! There's only one place
        // this is used currently, and it's right for that.
        alDialog->x+=(oldW-getWidth());
        alDialog->w=getWidth();
    }
}

void Label::realize(DialogRunner& runner)
{
    alDialog=runner.push(shared_from_this(), DIALOG {
        jwin_text_proc,
        x, y, getWidth(), getHeight(),
        fgColor, bgColor,
        0, // key
        D_NEW_GUI, // flags
        0, 0, // d1, d2
        (void*)text.c_str(), FONT, nullptr // dp, dp2, dp3
    });
}

}
