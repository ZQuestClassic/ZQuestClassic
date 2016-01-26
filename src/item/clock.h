#ifndef _ZC_ITEM_WATCH_H_
#define _ZC_ITEM_WATCH_H_

#include "itemEffect.h"
class LinkClass;

class ClockEffect: public ItemEffect
{
public:
    ClockEffect(LinkClass& link, int time);
    ~ClockEffect();
    void activate();
    void update();
    
private:
    LinkClass& link;
    int timer;
};

#endif
