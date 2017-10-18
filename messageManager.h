#ifndef _ZC_MESSAGEMANAGER_H_
#define _ZC_MESSAGEMANAGER_H_

#include <boost/function.hpp>

class Message;
struct MsgStr;
struct BITMAP;

class MessageManager
{
public:
    MessageManager();
    ~MessageManager();
    
    /// Displays a message.
    /** If a message is already being displayed and a callback is set,
     *  the callback function will be called before switching to
     *  the new message.
     */
    void showMessage(int id);
    
    /// Displays a DMap intro.
    /** If a message is already being displayed and a callback is set,
     *  the callback function will be called before switching to the intro.
     */
    void showDMapIntro(int dmap);
    
    /// Clears any messages being displayed.
    void clear(bool runCallback);
    void update();
    void drawToScreen(BITMAP* target, int yOffset); // const?
    bool messageIsActive() const;
    
    /// Clear the current message and switch to a new one.
    void switchTo(int msg);
    
    /// Set a function to be called when the current message is finished.
    /** Call this after setting the message to display.
     */
    inline void setMessageEndCallback(boost::function<void()> func)
    {
        // Should it be called immediately if the message is blank?
        onMessageEnd=func;
    }
    
private:
    enum msgState {
        mst_inactive, mst_active, mst_waitingToAdvance, mst_awaitingInput
    };
    
    MsgStr* currMsgStr;
    Message* activeMessage;
    msgState state;
    int timer;
    boost::function<void()> onMessageEnd;
};

#endif
