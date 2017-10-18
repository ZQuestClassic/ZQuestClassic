#include <precompiled.h>
#include "getBigTriforce.h"
#include <link.h>
#include <sound.h>
#include <zelda.h>


GetBigTriforce::GetBigTriforce(int tid, LinkClass& l):
    link(l),
    triforceID(tid),
    counter(0)
{
}

void GetBigTriforce::activate()
{
    sfx(itemsbuf[triforceID].playsound);
    guys.clear();
    
    if(itemsbuf[triforceID].flags & ITEM_GAMEDATA)
        game->lvlitems[dlevel]|=liTRIFORCE;
}

void GetBigTriforce::update()
{
    /*
      *************************
      * BIG TRIFORCE SEQUENCE *
      *************************
      0 BIGTRI out, WHITE flash in
      4 WHITE flash out, PILE cset white
      8 WHITE in
      ...
      188 WHITE out
      191 PILE cset red
      200 top SHUTTER opens
      209 bottom SHUTTER opens
      */
    
    if(counter==4)
    {
        for(int i=1; i<16; i++)
            RAMpal[CSET(9)+i]=_RGB(63,63,63);
    }
    
    if((counter&7)==0)
    {
        for(int cs=2; cs<5; cs++)
        {
            for(int i=1; i<16; i++)
                RAMpal[CSET(cs)+i]=_RGB(63,63,63);
        }
        
        refreshpal=true;
    }
    
    if((counter&7)==4)
    {
        if(currscr<128)
            loadlvlpal(DMaps[currdmap].color);
        else
            loadlvlpal(0xB);
    }
    
    if(counter==191)
        loadpalset(9,pSprite(spPILE));
    
    //advanceframe(true);
    counter++;
    if(counter<24*8)
        return;
    
    finish();
    playLevelMusic();
    
    if(itemsbuf[triforceID].flags & ITEM_FLAG1 && currscr < 128)
        link.dowarp(1,0); //side warp
}
