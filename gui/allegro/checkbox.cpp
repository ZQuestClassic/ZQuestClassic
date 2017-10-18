#include "checkbox.h"
#include "common.h"
#include "renderer.h"
#include <allegro.h>

extern FONT* zfont;

namespace GUI
{

int AllegroCheckbox::proc(int msg, DIALOG* d, int c)
{
    AllegroCheckbox* cb=static_cast<AllegroCheckbox*>(d->dp);
    
    switch(msg)
    {
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_LPRESS:
    case MSG_KEY:
        cb->toggle();
        break;
        
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
    }
    
    return D_O_K;
}

AllegroCheckbox::AllegroCheckbox(const std::string& t, FONT* f):
    StandardAllegroWidget(),
    text(t),
    font(f),
    checked(false)
{
}

void AllegroCheckbox::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    if((dialog->flags&D_GOTFOCUS)!=0)
        renderer.drawRoundedRect(0, 0, width, height, 7, pal.buttonHL);
    
    renderer.fillRoundedRect(2, 2, 14, 14, 3, pal.editBG);
    renderer.drawRoundedRect(2, 2, 14, 14, 3, pal.outline);
    renderer.drawText(text, 20, (18-text_height(font))/2, font, pal.winBodyText);
    
    if(checked)
    {
        // This sucks
        static const char check[2]={ (char)129, 0 };
        renderer.drawText(check, 5, 6, zfont, pal.winBodyText);
    }
}

void AllegroCheckbox::getPreferredSize(int& prefWidth, int& prefHeight)
{
    FONT* tempFont=::font;
    ::font=font;
    prefWidth=22+gui_strlen(text.c_str());
    ::font=tempFont;
    prefHeight=18;
}

void AllegroCheckbox::setSizeAndPos(int newX, int newY,
  int newWidth, int newHeight)
{
    adjustSizeAndPos(newY, newHeight, 18);
    StandardAllegroWidget::setSizeAndPos(newX, newY, newWidth, newHeight);
}

void AllegroCheckbox::toggle()
{
    checked=!checked;
    if(onValueChanged)
        onValueChanged(checked);
}

int AllegroCheckbox::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroCheckbox>(dlgArray, index, getKey(text));
}

}
