#ifndef _ZC_FONTCLASS_H_
#define _ZC_FONTCLASS_H_

#include <allegro.h>

#include <allegro/internal/aintern.h>

class Font
{
public:
    static Font getByID(int id);
    
    Font();
    Font(FONT* f);
    
    inline const FONT* get() const
    {
        return allegroFont;
    }
    
    inline int getWidth(char c) const 
    {
        return allegroFont->vtable->char_length(allegroFont, c);
    }
    
    inline int getHeight() const
    {
        return text_height(allegroFont);
    }
    
private:
    const FONT* allegroFont;
};

#endif
