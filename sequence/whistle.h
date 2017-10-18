#ifndef _ZC_SEQUENCE_WHISTLE_H_
#define _ZC_SEQUENCE_WHISTLE_H_

#include "sequence.h"
#include <sfxClass.h>
class LinkClass;

class WhistleSequence: public Sequence
{
public:
    WhistleSequence(int itemID, LinkClass& link, const SFX& sound);
    void activate();
    void update();
    
private:
    int itemID;
    LinkClass& link;
    SFX sound;
};

#endif
