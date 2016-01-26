#ifndef _ZC_GUI_ALLEGRO__EDITABLETEXT_H_
#define _ZC_GUI_ALLEGRO__EDITABLETEXT_H_

#include <string>
struct BITMAP;
struct FONT;

namespace GUI
{

class AllegroGUIRenderer;

/** Editable text for use in text editing widgets */
// The implementation could be more efficient.
// std::string isn't made for this sort of thing.
class EditableText
{
public:
    EditableText(FONT* font);
    void putChar(int character, int pos);
    inline void setText(const std::string& newText) { text.assign(newText); }
    void draw(AllegroGUIRenderer& renderer, int x, int y, int width, int cursorPos, int highlightPos, bool active) const;
    void deleteText(int start, int end);
    inline size_t length() const { return text.length(); }
    inline std::string getText() const { return text; }
    inline int getHeight() const { return height; }
    
private:
    std::string text;
    FONT* font;
    const int height;
};

}

#endif
