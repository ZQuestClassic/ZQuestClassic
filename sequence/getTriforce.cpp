#include <precompiled.h>
#include "getTriforce.h"
#include <link.h>
#include <sound.h>
#include <zelda.h>

extern int draw_screen_clip_rect_x1;
extern int draw_screen_clip_rect_x2;

GetTriforce::GetTriforce(int tid, LinkClass& l):
    link(l),
    triforceID(tid),
    counter(0),
    counter2(0),
    x2(0)
{
    for(int i=0; i<256; i++)
        flash_pal[i] = get_bit(quest_rules,qr_FADE) ? _RGB(63,63,0) : _RGB(63,63,63);
}
void GetTriforce::activate()
{
    //get rid off all sprites but Link
    guys.clear();
    items.clear();
    Ewpns.clear();
    Lwpns.clear();
    Sitems.clear();
    chainlinks.clear();
    
    //decorations.clear();
    if(!COOLSCROLL)
        show_subscreen_items=false;
    
    sfx(itemsbuf[triforceID].playsound);
    music_stop();
    
    if(itemsbuf[triforceID].misc1)
        jukebox(itemsbuf[triforceID].misc1+ZC_MIDI_COUNT-1);
    else
        try_zcmusic((char*)"zelda.nsf",5, ZC_MIDI_TRIFORCE);
        
    if(itemsbuf[triforceID].flags & ITEM_GAMEDATA)
        game->lvlitems[dlevel]|=liTRIFORCE;
}

void GetTriforce::update()
{
    if(counter==40)
    {
        actiontype oldaction = (actiontype)link.getAction();
        ALLOFF(true, false);
        link.setAction(oldaction); // have to reset this
    }
    
    if(counter>=40 && counter<88)
    {
        if(get_bit(quest_rules,qr_FADE))
        {
            if((counter&3)==0)
            {
                fade_interpolate(RAMpal,flash_pal,RAMpal,42,0,CSET(6)-1);
                refreshpal=true;
            }
            
            if((counter&3)==2)
            {
                loadpalset(0,0);
                loadpalset(1,1);
                loadpalset(5,5);
                
                if(currscr<128) loadlvlpal(DMaps[currdmap].color);
                else loadlvlpal(0xB); // TODO: Cave/Item Cellar distinction?
            }
        }
        else
        {
            if((counter&7)==0)
            {
                for(int cs2=2; cs2<5; cs2++)
                {
                    for(int i=1; i<16; i++)
                    {
                        RAMpal[CSET(cs2)+i]=flash_pal[CSET(cs2)+i];
                    }
                }
                
                refreshpal=true;
            }
            
            if((counter&7)==4)
            {
                if(currscr<128) loadlvlpal(DMaps[currdmap].color);
                else loadlvlpal(0xB);
                
                loadpalset(5,5);
            }
        }
    }
    
    if(itemsbuf[triforceID].flags & ITEM_GAMEDATA)
    {
        if(counter==88)
        {
            refill_what=REFILL_ALL;
            refill_why=triforceID;
            link.StartRefill(REFILL_ALL);
            link.refill();
        }
        
        if(counter==89)
        {
            if(link.refill())
                counter--;
        }
    }
    
    if(itemsbuf[triforceID].flags & ITEM_FLAG1) // Warp out flag
    {
        if(counter>=208 && counter<288)
        {
            x2++;
            counter2++;
            switch(counter2)
            {
            case 5:
                counter2=0;
                
            case 0:
            case 2:
            case 3:
                x2++;
                break;
            }
        }
        
        do_dcounters();
        
        if(counter<288)
        {
            int curtain_x=x2&0xF8;
            draw_screen_clip_rect_x1=curtain_x;
            draw_screen_clip_rect_x2=255-curtain_x;
        }
    }
    
    counter++;
    if(counter<408)
        return;
    if(midi_pos > 0 || (zcmusic && zcmusic->position<800))   // 800 may not be just right, but it works
        return;
    
    finish();
    draw_screen_clip_rect_x1=0;
    draw_screen_clip_rect_x2=255;
    show_subscreen_items=true;
    
    if(itemsbuf[triforceID].flags & ITEM_FLAG1 && currscr < 128)
    {
        link.setScrollDir(link.dir);
        link.dowarp(1,0); //side warp
    }
    else
        playLevelMusic();
}
