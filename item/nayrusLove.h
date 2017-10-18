#ifndef _ZC_ITEM_NAYRUSLOVE_H_
#define _ZC_ITEM_NAYRUSLOVE_H_

#include "itemAction.h"
#include "itemEffect.h"
#include <entityPtr.h>
#include <sfxClass.h>
class dNayrusLoveShield;
class LinkClass;
class weapon;

class NayrusLoveAction: public ItemAction
{
public:
    NayrusLoveAction(int itemID, LinkClass& link);
    ~NayrusLoveAction();
    void update();
    bool isFinished() const;
    void abort();
    
private:
    int phase;
    int timer;
    LinkClass& link;
    EntityPtr<weapon> rocket1;
    EntityPtr<weapon> rocket2;
};

class NayrusLoveEffect: public ItemEffect
{
public:
    NayrusLoveEffect(LinkClass& link, int time, const SFX& normal,
      const SFX& fading);
    ~NayrusLoveEffect();
    void activate();
    void update();
    
private:
    LinkClass& link;
    int timer;
    EntityPtr<dNayrusLoveShield> shieldFront, shieldBack;
    SFX normalSFX, fadingSFX;
    
    void createShield();
};

#endif
