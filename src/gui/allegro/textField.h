#ifndef _ZC_GUI_ALLEGRO_TEXTFIELD_H_
#define _ZC_GUI_ALLEGRO_TEXTFIELD_H_

#include "../textField.h"
#include "standardWidget.h"
#include "editableText.h"
#include <string>

namespace GUI
{
    
class AllegroTextField: public TextField, public StandardAllegroWidget
{
public:
    AllegroTextField(int maxLength);
    void setText(const std::string& text);
    inline std::string getText() const { return text.getText(); }
    void getPreferredSize(int& prefWidth, int& prefHeight);
    void setSizeAndPos(int newX, int newY, int newWidth, int newHeight);
    void draw(AllegroGUIRenderer& renderer) const;
    void putChar(int character, int pos);
    
    bool fillsWidth() const { return true; }
    int realize(DIALOG dlgArray[], int index);
    
    static int proc(int msg, DIALOG* d, int c);
    
private:
    int maxLength;
    EditableText text;
    int cursorPos, highlightPos;
    int offset;
    
    bool onKeyPress(int key);
    void moveCursor(int newPos);
    void deleteHighlighted();
    void updateOffset();
};

}

#endif
