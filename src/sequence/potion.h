#ifndef _ZC_SEQUENCE_POTION_H_
#define _ZC_SEQUENCE_POTION_H_

#include "sequence.h"
#include <sfxClass.h>
class LinkClass;

class PotionSequence: public Sequence
{
public:
    PotionSequence(int itemID, LinkClass& link, const SFX& fillSound,
      const SFX& endSound);
    void activate();
    void update();
    
private:
    int itemID;
    LinkClass& link;
    SFX fillSound, endSound;
    int timer;
    int healthToRefill, magicToRefill;
};

#endif
