#ifndef _ZC_MESSAGE_H_
#define _ZC_MESSAGE_H_

#include "messageStream.h"

#include "fontClass.h"
#include "messageRenderer.h"

struct BITMAP;
struct FONT;
class MessageManager;
struct MsgStr;

class Message
{
public:
    enum updateSpeed { spd_normal, spd_fast, spd_instant };
    
    Message(const MsgStr* msgstr, MessageManager& mm);
    Message(int dmap, MessageManager& mm);
    
    void update(updateSpeed speed);
    
    /// Clear the current message and switch to a new one.
    void reset(const MsgStr* nextMsg);
    
    /// Load a message as a continuation of the current one.
    void continueTo(const MsgStr* nextMsg);
    void draw(BITMAP* target, int yOffset);
    
    inline bool atEnd() const
    {
        return stream.atEnd();
    }
    
private:
    MessageManager& manager;
    MessageStream stream;
    MessageRenderer renderer;
    
    int msgX, msgY;
    int textSFX;
    int textSpeed;
    bool wordWrappingEnabled;
    int timer;
    
    void processNext(updateSpeed speed);
    
    /// Starts a new line if the next word won't fit on the current line.
    /** Assumes the stream is at the beginning of a word, so this should
     *  be called after processing whitespace.
     */
    void checkWordWrapping();
    
    /// Executes a control code.
    /** Arguments are read from the stream; it's assumed that the stream
     *  position is at the beginning of the first argument, if there are any.
     */
    void executeControlCode(char code);
    int getNumControlCodeArgs(char code) const;
};

#endif
