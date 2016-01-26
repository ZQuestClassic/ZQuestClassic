#include <precompiled.h> //always first
#include "nayrusLove.h"
#include <decorations.h>
#include <link.h>
#include <sound.h>
#include <zc_sys.h>
#include <zdefs.h>
#include <zelda.h>

extern sprite_list decorations;

static void nlRocketSparkle(const weapon& wpn)
{
    if((frame&3)!=0)
        return;
    
    int type;
    switch(wpn.type)
    {
    case pNAYRUSLOVEROCKET1:
        type=pNAYRUSLOVEROCKETTRAIL1;
        break;
        
    case pNAYRUSLOVEROCKET2:
        type=pNAYRUSLOVEROCKETTRAIL2;
        break;
        
    case pNAYRUSLOVEROCKETRETURN1:
        type=pNAYRUSLOVEROCKETTRAILRETURN1;
        break;
        
    case pNAYRUSLOVEROCKETRETURN2:
        type=pNAYRUSLOVEROCKETTRAILRETURN2;
        break;
    }
    
    Lwpns.add(new weapon(
      fix((wpn.getX()-3)+(rand()%7)),
      fix((wpn.getY()-3)+(rand()%7)),
      wpn.getZ(), wPhantom, type, 0, 0, wpn.parentitem, -1));
}

NayrusLoveAction::NayrusLoveAction(int itemID, LinkClass& l):
    phase(0),
    timer(0),
    link(l)
{
    magicitem=itemID;
}

NayrusLoveAction::~NayrusLoveAction()
{
    rocket1.del();
    rocket2.del();
    magicitem=-1;
}

void NayrusLoveAction::update()
{
    if(timer>0)
    {
        timer--;
        return;
    }
    
    // See also link.cpp, LinkClass::checkhit().
    switch(phase)
    {
    case 0:
        rocket1.reset(new weapon(link.getX(),link.getY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET1,0,left, magicitem, link.getUID()));
        rocket1->LOADGFX(itemsbuf[magicitem].wpn);
        rocket1->step=4;
        rocket1->setSparkleFunc(nlRocketSparkle);
        Lwpns.add(rocket1.get());
        
        rocket2.reset(new weapon(link.getX(),link.getY(),(fix)0,wPhantom,pNAYRUSLOVEROCKET2,0,right, magicitem, link.getUID()));
        rocket2->LOADGFX(itemsbuf[magicitem].wpn6);
        rocket2->step=4;
        rocket2->setSparkleFunc(nlRocketSparkle);
        Lwpns.add(rocket2.get());
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1ROCKETUP, pan(link.getX()));
        
        linktile(&link.tile, &link.flip, &link.extend, ls_cast, link.getDir(), zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        phase=1;
        timer=64;
        break;
        
    case 1:
        rocket1->dir=right;
        rocket1->type=pNAYRUSLOVEROCKETRETURN1;
        rocket1->LOADGFX(itemsbuf[magicitem].wpn2);
        
        rocket2->dir=left;
        rocket2->type=pNAYRUSLOVEROCKETRETURN2;
        rocket2->LOADGFX(itemsbuf[magicitem].wpn7);
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1ROCKETDOWN, pan(link.getX()));
        
        linktile(&link.tile, &link.flip, &link.extend, ls_cast, link.getDir(), zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        phase=3;
        break;
            
    case 3:
        if(rocket1->getX()<link.getX())
            break;
        
        rocket1.del();
        rocket2.del();
        
        linktile(&link.tile, &link.flip, &link.extend, ls_landhold2, link.getDir(), zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        link.addItemEffect(getItemEffect(magicitem));
        
        phase=4;
        timer=32;
        break;
        
    case 4:
        // Just waiting for the timer...
        phase=5;
        break;
    }
}

bool NayrusLoveAction::isFinished() const
{
    return phase==5;
}

void NayrusLoveAction::abort()
{
    rocket1.del();
    rocket2.del();
}


// -----------------------------------------------------------------------------


NayrusLoveEffect::NayrusLoveEffect(LinkClass& l, int time, const SFX& ns,
  const SFX& fs):
    ItemEffect(ie_nayrusLove),
    link(l),
    timer(time),
    normalSFX(ns),
    fadingSFX(fs)
{
}

NayrusLoveEffect::~NayrusLoveEffect()
{
    link.setNayrusLoveActive(false);
    if(shieldFront)
        shieldFront.del();
    if(shieldBack)
        shieldBack.del();
}

void NayrusLoveEffect::activate()
{
    createShield();
    link.setNayrusLoveActive(true);
    normalSFX.startLooping();
}

void NayrusLoveEffect::update()
{
    timer--;
    if(timer==0)
    {
        delete this;
        return;
    }
    else if(timer==256)
    {
        normalSFX.stopLooping();
        fadingSFX.startLooping();
    }
    
    if(!shieldFront || !shieldBack) // They may have been deleted by ALLOFF()
        createShield();
    shieldFront->setX(link.getX());
    shieldFront->setY(link.getY());
    shieldBack->setX(link.getX());
    shieldBack->setY(link.getY());
}

void NayrusLoveEffect::createShield()
{
    if(!shieldFront)
    {
        shieldFront.reset(new dNayrusLoveShield(link.getX(), link.getY(), timer));
        shieldFront->misc=0; // I may have front and back mixed up...
        decorations.add(shieldFront.get());
    }
    if(!shieldBack)
    {
        shieldBack.reset(new dNayrusLoveShield(link.getX(), link.getY(), timer));
        shieldBack->misc=1;
        decorations.add(shieldBack.get());
    }
}
