#include "editableText.h"
#include "common.h"
#include "renderer.h"
#include <allegro.h>
#include <algorithm>

using std::max;
using std::min;

namespace GUI
{

EditableText::EditableText(FONT* f):
    font(f),
    height(text_height(f))
{
}

void EditableText::deleteText(int start, int end)
{
    start=max(start, 0);
    end=min(end, (int)text.length());
    text.erase(start, end-start);
}

void EditableText::putChar(int character, int pos)
{
    if((character<32) || (character>255))
        return;
    
    text.insert(pos, 1, character);
}

void EditableText::draw(AllegroGUIRenderer& renderer, int x, int y, int width,
  int cursorPos, int highlightPos, bool active) const
{
    renderer.setOrigin(x, y);
    
    renderer.drawText(text, 1, 0, font, pal.editText);
    
    if(active)
    {
        int cursorX=text_length(font, text.substr(0, cursorPos).c_str());
        renderer.drawLine(cursorX, 1, cursorX, height-1, pal.editText);
    }
    
    if(highlightPos!=cursorPos)
    {
        int start=min(cursorPos, highlightPos);
        int end=max(cursorPos, highlightPos);
        int hlStartX=text_length(font, text.substr(0, start).c_str());
        int hlEndX=text_length(font, text.substr(0, end).c_str());
        const GUIColor& bg=active ? pal.editHL : pal.editInactiveHL;
        renderer.fillRect(hlStartX, 0, hlEndX-hlStartX, height-1, bg);
        renderer.drawText(text.substr(start, end-start), hlStartX+1, 0, font, pal.editTextHL);
    }
}

} // Namespace
