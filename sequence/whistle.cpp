#include <precompiled.h>
#include "whistle.h"
#include "sequence.h"
#include <link.h>
#include <weapons.h>
#include <zc_sys.h>
#include <zelda.h>
extern int whistleitem;

WhistleSequence::WhistleSequence(int id, LinkClass& l, const SFX& s):
    itemID(id),
    link(l),
    sound(s)
{
}

void WhistleSequence::activate()
{
    sound.play(link.getX());
}

void WhistleSequence::update()
{
    if(sound.isPlaying())
        return;
    
    finish();
    fix x=link.getX(), y=link.getY(), z=link.getZ();
    int dir=link.getDir();
    if(dir==up || dir==right)
        link.modWhistleCounter(1);
    else
        link.modWhistleCounter(-1);
    
    Lwpns.add(new weapon(x,y,z,wWhistle,0,0,dir,itemID,link.getUID()));
    
    bool foundEntrance=findentrance(x,y,mfWHISTLE,false);
        
    //if(((didstuff&did_whistle) && itemsbuf[itemid].flags&ITEM_FLAG1) || currscr>=128)
    if(foundEntrance || currscr>=128)
        return;
        
    //if(itemsbuf[itemID].flags&ITEM_FLAG1) didstuff |= did_whistle;
    
    if((tmpscr->flags&fWHISTLE) || (tmpscr->flags7 & fWHISTLEWATER)
            || (tmpscr->flags7&fWHISTLEPAL))
        whistleclk=0; // signal to start drying lake or doing other stuff
    else
    {
        int where = itemsbuf[itemID].misc1;
        
        if(where>right) where=dir^1;
        
        if(((DMaps[currdmap].flags&dmfWHIRLWIND && TriforceCount()) || DMaps[currdmap].flags&dmfWHIRLWINDRET) &&
                itemsbuf[itemID].misc2 >= 0 && itemsbuf[itemID].misc2 <= 8 && !foundEntrance)
            Lwpns.add(new weapon((fix)(where==left?240:where==right?0:x),(fix)(where==down?0:where==up?160:y),
                                 (fix)0,wWind,0,0,where,itemID,link.getUID()));
                                 
        whistleitem=itemID;
    }
}
