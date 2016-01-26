#include "textField.h"
#include "common.h"
#include "renderer.h"
#include "../../zsys.h" // vbound
#include <allegro.h>
#include <algorithm>

extern FONT* lfont_l;

using std::max;
using std::min;

namespace GUI
{

int AllegroTextField::proc(int msg, DIALOG *d, int c)
{
    AllegroTextField* tf=static_cast<AllegroTextField*>(d->dp);
    switch(msg)
    {
    case MSG_WANTFOCUS:
        return D_WANTFOCUS;
        
    case MSG_LPRESS:
        // TODO
        break;
        
    case MSG_CHAR:
    {
        if(tf->onKeyPress(c))
            return D_USED_CHAR;
        break;
    }
    
    case MSG_WHEEL:
        forwardMouseWheel(d, c);
        break;
    }
    
    return D_O_K;
}

AllegroTextField::AllegroTextField(int len):
    StandardAllegroWidget(),
    maxLength(len),
    text(lfont_l),
    cursorPos(0),
    highlightPos(0),
    offset(0)
{
}

void AllegroTextField::getPreferredSize(int& prefWidth, int& prefHeight)
{
    prefWidth=6*maxLength;
    prefHeight=text.getHeight()+12;
}

void AllegroTextField::setText(const std::string& newText)
{
    // TODO: limit length
    if(newText.length()<=maxLength)
        text.setText(newText);
    else
        text.setText(newText.substr(0, maxLength));
    cursorPos=highlightPos=text.length();
}

void AllegroTextField::draw(AllegroGUIRenderer& renderer) const
{
    renderer.setOrigin(x, y);
    renderer.fillRoundedRect(0, 0, width, height, 4, pal.editBG);
    renderer.drawRoundedRect(0, 0, width, height, 4, pal.outline);
    renderer.pushClipRect(3, 4, width-6, height-6);
    text.draw(renderer, x+3+offset, y+4, width-6, cursorPos, highlightPos, (dialog->flags&D_GOTFOCUS)!=0);
    renderer.popClipRect();
}

bool AllegroTextField::onKeyPress(int kp)
{
    // Check for special keys first
    switch(kp>>8)
    {
    case KEY_LEFT:
        moveCursor(cursorPos-1);
        break;
        
    case KEY_RIGHT:
        moveCursor(cursorPos+1);
        break;
        
    case KEY_HOME:
        moveCursor(0);
        break;
        
    case KEY_END:
        moveCursor(text.length());
        break;
        
    case KEY_DEL:
    case KEY_BACKSPACE:
        {
            if(highlightPos!=cursorPos)
                deleteHighlighted();
            else if((kp>>8)==KEY_BACKSPACE)
            {
                text.deleteText(cursorPos-1, cursorPos);
                if(cursorPos>0)
                {
                    cursorPos--;
                    highlightPos--;
                }
            }
            else
                text.deleteText(cursorPos, cursorPos+1);
            
            break;
        }
        
    case KEY_ENTER:
        // TODO
        return true;
        
    case KEY_TAB:
        // Ignore
        return false;
        
    default:
        // Not one of those; is it a text character?
        kp&=0xFF;
        if((kp<32) || (kp>255))
            return false;
        
        if(highlightPos!=cursorPos)
            deleteHighlighted();
        if(text.length()<maxLength)
        {
            text.putChar(kp, cursorPos);
            cursorPos++;
            highlightPos++;
        }
    }
    
    updateOffset();
    return true;
}

void AllegroTextField::moveCursor(int newPos)
{
    cursorPos=vbound(newPos, 0, text.length());
    if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
        highlightPos=cursorPos;
}

void AllegroTextField::deleteHighlighted()
{
    int start=min(cursorPos, highlightPos);
    int end=max(cursorPos, highlightPos);
    text.deleteText(start, end);
    if(highlightPos<cursorPos)
        cursorPos=highlightPos;
    else
        highlightPos=cursorPos;
}

void AllegroTextField::updateOffset()
{
    int cursorX=text_length(lfont_l, text.getText().substr(0, cursorPos).c_str());
    if(cursorX+offset<20) // Cursor is too far left
        offset=min(-cursorX+20, 0);
    else if(cursorX+offset>width-20) // Too far right
        offset=width-cursorX-20;
}

int AllegroTextField::realize(DIALOG dlgArray[], int index)
{
    return realizeAs<AllegroTextField>(dlgArray, index);
}

void AllegroTextField::setSizeAndPos(int newX, int newY,
  int newWidth, int newHeight)
{
    adjustSizeAndPos(newY, newHeight, text.getHeight()+8);
    if(newWidth>6*maxLength)
        newWidth=6*maxLength;
    StandardAllegroWidget::setSizeAndPos(newX, newY, newWidth, newHeight);
}

} // Namespace
