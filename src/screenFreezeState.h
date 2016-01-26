#ifndef _ZC_SCREENFREEZESTATE_H_
#define _ZC_SCREENFREEZESTATE_H_

class MessageManager;

// Not sure if this warrants its own class.
// It might be merged into something else later.
class ScreenFreezeState
{
public:
    ScreenFreezeState(const MessageManager& messageMgr);
    void update();
    
    bool scriptDrawingFrozen() const;
    bool ffcsFrozen() const;
    bool globalScriptFrozen() const;
    bool entitiesFrozen() const;
    bool itemRefillsFrozen() const;
    
    // This one sucks. Even if messages freeze everything, guys and fires
    // still update. Gotta do something better here.
    bool guysAndFiresFrozen() const;
    
private:
    const MessageManager& messageMgr;
    bool messageFreeze, freezeFFCCombo, freezeNonFFCCombo;
};

#endif
