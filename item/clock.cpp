#include <precompiled.h>
#include "clock.h"
#include <link.h>

extern bool cheat_superman, watch;

ClockEffect::ClockEffect(LinkClass& l, int time):
    ItemEffect(ie_clock),
    link(l),
    timer((time>0) ? time : -1)
{
}

ClockEffect::~ClockEffect()
{
    if(!cheat_superman)
        link.setInvincible(false);
    watch=false;
}

void ClockEffect::activate()
{
    watch=true;
    link.setInvincible(true);
}

void ClockEffect::update()
{
    if(timer<0)
        return;
    
    timer--;
    if(timer==0)
        delete this;
}
