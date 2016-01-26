#include <precompiled.h>
#include "potion.h"
#include "sequence.h"
#include <link.h>
#include <zelda.h>


PotionSequence::PotionSequence(int id, LinkClass& l, const SFX& s1,
  const SFX& s2):
    itemID(id),
    link(l),
    fillSound(s1),
    endSound(s2),
    timer(1)
{
    healthToRefill=itemsbuf[id].misc1; // Values entered in the item editor
    magicToRefill=itemsbuf[id].misc2;
    
    // Percent or blocks?
    if(healthToRefill>0 && (itemsbuf[id].flags&ITEM_FLAG1)!=0)
        healthToRefill*=game->get_maxlife()/100.0;
    else
        healthToRefill*=HP_PER_HEART;
    
    if(magicToRefill>0 && (itemsbuf[id].flags&ITEM_FLAG2)!=0)
        magicToRefill*=game->get_maxlife()/100.0;
    else
        magicToRefill*=MAGICPERBLOCK;
    
    // Don't fill beyond full
    int maxFill=game->get_maxlife()-game->get_life();
    if(healthToRefill>maxFill)
        healthToRefill=maxFill;
    
    maxFill=game->get_maxmagic()-game->get_magic();
    if(magicToRefill>maxFill)
        magicToRefill=maxFill;
}

void PotionSequence::activate()
{
    fillSound.startLooping();
    
    // Clear jinxes
    if(!get_bit(quest_rules,qr_NONBUBBLEMEDICINE))
    {
        link.setSwordClk(0);
        if(get_bit(quest_rules,qr_ITEMBUBBLE))
            link.setItemClk(0);
    }
}

void PotionSequence::update()
{
    timer--;
    if(timer==0)
    {
        if(healthToRefill>0)
        {
            game->change_life(HP_PER_HEART/2);
            healthToRefill-=HP_PER_HEART/2;
            if(healthToRefill<0)
                healthToRefill=0;
        }
        
        if(magicToRefill>0)
        {
            game->change_magic(MAGICPERBLOCK/4);
            magicToRefill-=MAGICPERBLOCK/4;
            if(magicToRefill<0)
                magicToRefill=0;
        }
        
        if(healthToRefill==0 && magicToRefill==0)
        {
            finish();
            fillSound.stopLooping();
            endSound.play();
        }
        else
            timer=get_bit(quest_rules,qr_FASTFILL) ? 6 : 22;
    }
}
