#ifndef _ZC_MESSAGERENDERER_H_
#define _ZC_MESSAGERENDERER_H_

#include "fontClass.h"
struct BITMAP;
struct MsgStr;

class MessageRenderer
{
public:
    MessageRenderer();
    ~MessageRenderer();
    
    /// Draws the message frame and loads the text settings.
    /** \param msg A pointer to the message; if this is null, the renderer
     *             will set itself up to render DMap intros
     */
    void initialize(const MsgStr* msg);
    
    /// Loads the data to render a message continuation
    /** \param msg A pointer to the message; this must not be null
     */
    void loadContinuationStyle(const MsgStr* msg);
    
    /// Check if a word will fit on the current line.
    /** \param wordWidth The width of the word in pixels with no added space
     *  \param numChars The number of characters in the word
     */
    bool wordFits(int wordWidth, int numChars) const;
    void putChar(char c);
    void draw(BITMAP* target, int x, int y);
    
    inline void startNewLine()
    {
        cursorX=0;
        cursorY+=font.getHeight()+vSpace;
    }
    
    inline void setColor(int newColor)
    {
        color=newColor;
    }
    
    inline void setFont(int newFontID)
    {
        font=Font::getByID(newFontID);
    }
    
    inline bool atLineStart() const
    {
        return cursorX==0;
    }
    
    inline Font getFont() const
    {
        return font;
    }
    
private:
    BITMAP* fullBmp; /// The frame is drawn here
    BITMAP* textBmp; /// Sub-bitmap of fullBmp where text is drawn
    Font font;
    int color;
    int hSpace, vSpace;
    int cursorX, cursorY;
};

#endif
