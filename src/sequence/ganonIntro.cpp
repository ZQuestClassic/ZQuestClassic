#include <precompiled.h>
#include "ganonIntro.h"
#include <guys.h>
#include <link.h>
#include <sfx.h>
#include <sound.h>
#include <zc_sys.h>
#include <zelda.h>


GanonIntro::GanonIntro(LinkClass& l):
    counter(0),
    link(l)
{
    if(game->lvlitems[dlevel]&liBOSS)
        finish();
}

void GanonIntro::activate()
{
    loaded_guys=true;
    loaditem();
    link.dir=down;
    link.setAction(landhold2);
    link.setHeldItem(getItemID(itemsbuf, itype_triforcepiece, 1));
}

void GanonIntro::update()
{
    /*
    ************************
    * GANON INTRO SEQUENCE *
    ************************
    -25 DOT updates
    -24 LINK in
    0 TRIFORCE overhead - code begins at this point (counter == 0)
    47 GANON in
    58 LIGHT step
    68 LIGHT step
    78 LIGHT step
    255 TRIFORCE out
    256 TRIFORCE in
    270 TRIFORCE out
    271 GANON out, LINK face up
    */
    
    if(counter==47)
    {
        music_stop();
        stop_sfx(WAV_ROAR);
        sfx(WAV_GASP);
        sfx(WAV_GANON);
        int Id=0;
        
        for(int i=0; i<eMAXGUYS; i++)
        {
            if(guysbuf[i].flags2&eneflag_ganon)
            {
                Id=i;
                break;
            }
        }
        
        if(current_item(itype_ring))
            addenemy(160,96,Id,0);
        else
            addenemy(80,32,Id,0);
    }
    
    else if(counter==48)
    {
        lighting(true,true); // Hmm. -L
        counter += 30;
    }
    
    //NES Z1, the triforce vanishes for one frame in two cases
    //while still showing Link's two-handed overhead sprite.
    else if(counter==255 || counter==270)
        link.setHeldItem(-1);
    
    else if(counter==256)
        link.setHeldItem(getItemID(itemsbuf,itype_triforcepiece,1));
    
    counter++;
    if(counter<271)
        return;
    
    link.setAction(none);
    link.dir=up;
    
    if(!getmapflag() && (tunes[MAXMIDIS-1].data))
        jukebox(MAXMIDIS-1);
    else
        playLevelMusic();
        
    currcset=DMaps[currdmap].color;
    showCurrentDMapIntro();
    cont_sfx(WAV_ROAR);
    finish();
}
