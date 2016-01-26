#include <precompiled.h> //always first
#include "dinsFire.h"

#include <link.h>
#include <sound.h>
#include <zc_sys.h>
#include <zdefs.h>
#include <zelda.h>

extern LinkClass Link;

static void dfRocketSparkle(const weapon& wpn)
{
    if((frame&3)!=0)
        return;
    
    int type;
    if(wpn.type==pDINSFIREROCKET)
        type=pDINSFIREROCKETTRAIL;
    else
        type=pDINSFIREROCKETTRAILRETURN;
    
    Lwpns.add(new weapon(
      fix((wpn.getX()-3)+(rand()%7)),
      fix((wpn.getY()-3)+(rand()%7)),
      wpn.getZ(), wPhantom, type, 0, 0, wpn.parentitem, -1));
}

DinsFireAction::DinsFireAction(int itemID, LinkClass& l):
    phase(0),
    timer(0),
    link(l),
    rocket()
{
    magicitem=itemID;
}

DinsFireAction::~DinsFireAction()
{
    rocket.del();
    magicitem=-1;
}

void DinsFireAction::update()
{
    if(timer>0)
    {
        timer--;
        return;
    }
    
    switch(phase)
    {
    case 0:
        rocket.reset(new weapon(LinkX(),LinkY(),LinkZ(),wPhantom,pDINSFIREROCKET,0,up, magicitem, link.getUID()));
        rocket->LOADGFX(itemsbuf[magicitem].wpn);
        rocket->step=4;
        rocket->setSparkleFunc(dfRocketSparkle);
        Lwpns.add(rocket.get());
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1ROCKETUP, pan(link.getX()));
        
        linktile(&link.tile, &link.flip, &link.extend, ls_landhold2, link.getDir(), zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        phase=1;
        timer=64;
        break;
        
    case 1:
        rocket->type=pDINSFIREROCKETRETURN;
        rocket->y=-32;
        rocket->dir=down;
        rocket->LOADGFX(itemsbuf[magicitem].wpn2);
        rocket->step=4;
        
        
        if(get_bit(quest_rules,qr_MORESOUNDS))
            sfx(WAV_ZN1ROCKETDOWN, pan(link.getX()));
        
        linktile(&link.tile, &link.flip, &link.extend, ls_landhold2, link.getDir(), zinit.linkanimationstyle);
        
        if(get_bit(quest_rules,qr_EXPANDEDLTM))
            link.tile+=item_tile_mod();
        
        phase=3;
        break;
        
    case 3:
        if(rocket->getY()<link.getY())
            break;
        
        phase=4;
        rocket.del();
        break;
        
    case 4:
        {
            linktile(&link.tile, &link.flip, &link.extend, ls_cast, link.getDir(), zinit.linkanimationstyle);
            
            if(get_bit(quest_rules,qr_EXPANDEDLTM))
                link.tile+=item_tile_mod();
            
            if(get_bit(quest_rules,qr_MORESOUNDS))
                sfx(itemsbuf[magicitem].usesound, pan(link.getX()));
                
            int flamemax=itemsbuf[magicitem].misc1;
            
            for(int flamecounter=((-1)*(flamemax/2))+1; flamecounter<=((flamemax/2)+1); flamecounter++)
            {
                weapon* flame=new weapon((fix)LinkX(),(fix)LinkY(),(fix)LinkZ(),wFire,3,itemsbuf[magicitem].power*DAMAGE_MULTIPLIER,
                                     (tmpscr->flags7&fSIDEVIEW) ? (flamecounter<flamemax ? left : right) : 0, magicitem, link.getUID());
                flame->step=(itemsbuf[magicitem].misc2/100.0);
                flame->angular=true;
                flame->angle=(flamecounter*PI/(flamemax/2.0));
                Lwpns.add(flame);
            }
            
            phase=5;
            timer=98;
            break;
        }
        
    case 5:
        // Just waiting for the timer...
        phase=6;
        break;
    }
}

bool DinsFireAction::isFinished() const
{
    return phase==6;
}

void DinsFireAction::abort()
{
    rocket.del();
}
