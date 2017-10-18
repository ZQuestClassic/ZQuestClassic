#ifndef _MESSAGE_STREAM_H_
#define _MESSAGE_STREAM_H_

/** Used to interpret the string from a MsgStr as message text. Text can be
 * read as individual characters or control code arguments. Trailing spaces
 * are stripped.
 */
class MessageStream
{
public:
    MessageStream(const char* text);
    void reset(const char* text);
    
    inline bool nextIsWhitespace() const
    {
        return (rawText[ptr]==' ' || rawText[ptr]==26); // MSGC_NEWLINE+1
    }
    
    inline char getCharacter()
    {
        return rawText[ptr++];
    }
    
    int getControlCodeArgument();
    
    inline bool atEnd() const
    {
        return ptr>lastNonSpace; // There might be trailing spaces
    }
    
    inline unsigned int tell() const
    {
        return ptr;
    }
    
    inline void seek(unsigned int pos)
    {
        ptr=pos;
    }
    
private:
    const char* rawText;
    int ptr, lastNonSpace;
};

#endif
