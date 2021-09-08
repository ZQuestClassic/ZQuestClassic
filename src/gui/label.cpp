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

Label::Label(): text(), contX(0), contY(0), contW(0), contH(0)
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
        Widget::arrange(contX, contY, contW, contH);
        alDialog->x=x;
        alDialog->w=getWidth();
    }
}

void Label::setVisible(bool visible)
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

void Label::arrange(int cx, int cy, int cw, int ch)
{
    // Hang on to these in case the text is changed and
    // the label needs repositioned.
    contX=cx;
    contY=cy;
    contW=cw;
    contH=ch;
    Widget::arrange(cx, cy, cw, ch);
}

void Label::realize(DialogRunner& runner)
{
    alDialog=runner.push(shared_from_this(), DIALOG {
        jwin_text_proc,
        x, y, getWidth(), getHeight(),
        fgColor, bgColor,
        0, // key
        getFlags(), // flags
        0, 0, // d1, d2
        (void*)text.c_str(), FONT, nullptr // dp, dp2, dp3
    });
}

}
