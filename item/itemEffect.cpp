#include "itemEffect.h"
#include <debug.h>
#include <items.h>
#include <sfxManager.h>
#include <zelda.h>

extern SFXManager sfxMgr;


ItemEffect::ItemEffect(itemEffectType t):
    type(t),
    list(0)
{
}

ItemEffect::~ItemEffect()
{
    if(list)
        list->remove(this, false);
}

// -----------------------------------------------------------------------------

ItemEffectList::ItemEffectList():
    numEffects(0)
{
}

void ItemEffectList::add(ItemEffect* newEffect)
{
    // Remove any effects of the same type before adding the new one.
    // Will we ever not want to do this?
    remove(newEffect->getType(), true);
    
    effects[numEffects]=newEffect;
    numEffects++;
    newEffect->setList(this);
    newEffect->activate();
}

void ItemEffectList::update()
{
    for(int i=0; i<numEffects; i++)
        effects[i]->update();
}

void ItemEffectList::remove(ItemEffect* effect, bool del)
{
    bool found=false;
    for(int i=0; i<numEffects; i++)
    {
        if(found)
            effects[i]=effects[i+1];
        else if(effects[i]==effect)
        {
            effects[i]->setList(0); // So it won't call remove() recursively
            if(del)
                delete effects[i];
            found=true;
            numEffects--;
            effects[i]=effects[i+1];
        }
    }
    
    ZCASSERT(found);
}

void ItemEffectList::remove(itemEffectType type, bool del)
{
    bool found=false;
    for(int i=0; i<numEffects; i++)
    {
        if(found)
            effects[i]=effects[i+1];
        else if(effects[i]->getType()==type)
        {
            effects[i]->setList(0);
            if(del)
                delete effects[i];
            found=true;
            numEffects--;
            effects[i]=effects[i+1];
        }
    }
}

void ItemEffectList::clear()
{
    for(int i=0; i<numEffects; i++)
    {
        effects[i]->setList(0);
        delete effects[i];
    }
    numEffects=0;
}

// -----------------------------------------------------------------------------

// This should all go in a proper factory class, but it can wait
#include "clock.h"
#include "nayrusLove.h"
#include <link.h>
extern LinkClass Link;

ItemEffect* getItemEffect(int itemID)
{
    const itemdata& data=itemsbuf[itemID];
    switch(data.family)
    {
    case itype_clock:
        return new ClockEffect(Link, data.misc1);
        
    case itype_nayruslove:
        if(get_bit(quest_rules,qr_MORESOUNDS))
        {
            return new NayrusLoveEffect(Link, data.misc1,
              sfxMgr.getSFX(data.usesound), sfxMgr.getSFX(data.usesound+1));
        }
        else
            return new NayrusLoveEffect(Link, data.misc1, SFX(0), SFX(0));
    }
    
    return 0;
}
