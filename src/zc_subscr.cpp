//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>

#include "zelda.h"
#include "subscr.h"
#include "zc_subscr.h"
#include "link.h"
#include "gamedata.h"
#include "guys.h"
#include "ffscript.h"

extern LinkClass   Link;
extern int directItem;
extern int directItemA;
extern int directItemB;

//DIALOG *sso_properties_dlg;

void put_active_subscr(miscQdata *misc, int y, int pos)
{
    //Don't call Sitems.animate() - that gets called somewhere else, somehow. -L
    animate_selectors();
    bool showtime = game->get_timevalid() && !game->get_cheat() && get_bit(quest_rules,qr_TIME);
    show_custom_subscreen(framebuf, misc, current_subscreen_active, 0, 6-y, showtime, pos);
}

void dosubscr(miscQdata *misc)
{
    PALETTE temppal;
    
    if(tmpscr->flags3&fNOSUBSCR)
    {
        return;
    }
    
    if(usebombpal)
    {
        memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
        memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
        refreshpal=true;
    }
    
    int miny;
    bool showtime = game->get_timevalid() && !game->get_cheat() && get_bit(quest_rules,qr_TIME);
    load_Sitems(misc);
    
    pause_sfx(WAV_BRANG);
    
    if(current_item_id(itype_brang)>=0)
        pause_sfx(itemsbuf[current_item_id(itype_brang)].usesound);
        
    if(current_item_id(itype_hookshot)>=0)
        pause_sfx(itemsbuf[current_item_id(itype_hookshot)].usesound);
        
    adjust_sfx(WAV_ER,128,false);
    adjust_sfx(WAV_MSG,128,false);
    
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
    set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
    
    //make a copy of the blank playing field on the right side of scrollbuf
    blit(scrollbuf,scrollbuf,0,playing_field_offset,256,0,256,176);
    //make a copy of the complete playing field on the bottom of scrollbuf
    blit(framebuf,scrollbuf,0,playing_field_offset,0,176,256,176);
    miny = 6;
    
    //Set the selector to the correct position before bringing up the subscreen -DD
    if(get_bit(quest_rules,qr_SELECTAWPN))
    {
        if(Bwpn==0 && Awpn!=0)
            Bpos = zc_max(game->awpn,0);
        else
            Bpos = zc_max(game->bwpn,0);
    }
    else
    {
        Bpos = zc_max(game->bwpn,0);
	//set the position for the sword
	//Apos = game->awpn(game->awpn,0); 
        
    }
        
    for(int y=176-2; y>=6; y-=3)
    {
        do_dcounters();
        Link.refill();
        //fill in the screen with black to prevent the hall of mirrors effect
        rectfill(framebuf, 0, 0, 255, 223, 0);
        
        if(COOLSCROLL)
        {
            //copy the playing field back onto the screen
            blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
        }
        else
        {
            //scroll the playing field (copy the copy we made)
            blit(scrollbuf,framebuf,256,0,0,176-2-y+passive_subscreen_height,256,y);
        }
        
        //throw the passive subscreen onto the screen
        put_passive_subscr(framebuf,misc,0,176-2-y,showtime,sspSCROLLING);
        //put the active subscreen above the passive subscreen
        put_active_subscr(misc,y,sspSCROLLING);
        
        advanceframe(false);
        
        if(Quit)
            return;
    }
    
    bool done=false;
    
    do
    {
        load_control_state();
        int pos = Bpos;
        
        if(rUp())         Bpos = selectWpn_new(SEL_UP, pos);
        else if(rDown())  Bpos = selectWpn_new(SEL_DOWN, pos);
        else if(rLeft())  Bpos = selectWpn_new(SEL_LEFT, pos);
        else if(rRight()) Bpos = selectWpn_new(SEL_RIGHT, pos);
        else if(rLbtn())
	{
		if (!get_bit(quest_rules,qr_NO_L_R_BUTTON_INVENTORY_SWAP))
		{
			Bpos = selectWpn_new(SEL_LEFT, pos);
		}
	}
        else if(rRbtn() )
	{
		if (!get_bit(quest_rules,qr_NO_L_R_BUTTON_INVENTORY_SWAP)) 
		{
			Bpos = selectWpn_new(SEL_RIGHT, pos);
		}
	}
        else if(rEx3btn() )
	{
		if ( get_bit(quest_rules,qr_SELECTAWPN) && get_bit(quest_rules,qr_USE_EX1_EX2_INVENTORYSWAP) )
		{
			selectNextAWpn(SEL_LEFT);
		}
	}
	else if(rEx4btn() )
	{
		if ( get_bit(quest_rules,qr_SELECTAWPN) && get_bit(quest_rules,qr_USE_EX1_EX2_INVENTORYSWAP) )
		{
			selectNextAWpn(SEL_RIGHT);
		}
	}
        
        if(get_bit(quest_rules,qr_SELECTAWPN))
        {
            if(rBbtn())
            {
                if(Awpn == Bweapon(Bpos))
                {
                    Awpn = Bwpn;
                    game->awpn = game->bwpn;
                    directItemA = directItemB;
                }
                
                Bwpn = Bweapon(Bpos);
		game->forced_bwpn = -1; //clear forced if the item is selected using the actual subscreen
                sfx(WAV_PLACE);
                
                game->bwpn = Bpos;
                directItemB = directItem;
            }
            else if(rAbtn())
            {
                if(Bwpn == Bweapon(Bpos))
                {
                    Bwpn = Awpn;
                    game->bwpn = game->awpn;
                    directItemB = directItemA;
                }
                
                Awpn = Bweapon(Bpos);
                sfx(WAV_PLACE);
                game->awpn = Bpos;
		game->forced_awpn = -1; //clear forced if the item is selected using the actual subscreen
                directItemA = directItem;
            }
        }
        else
        {
            Bwpn = Bweapon(Bpos);
            game->bwpn = Bpos;
	    game->forced_bwpn = -1; //clear forced if the item is selected using the actual subscreen
            directItemB = directItem;
        }
        
        if(pos!=Bpos)
            sfx(WAV_CHIME);
            
        do_dcounters();
        Link.refill();
        
        //put_passive_subscr(framebuf,misc,0,174-miny,showtime,true);
        //blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
        //put_active_subscr(misc,miny,true);
        
        //fill in the screen with black to prevent the hall of mirrors effect
        rectfill(framebuf, 0, 0, 255, 223, 0);
        
        if(COOLSCROLL)
        {
            //copy the playing field back onto the screen
            blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
        }
        else
        {
            //nothing to do; the playing field has scrolled off the screen
        }
        
        //throw the passive subscreen onto the screen
        put_passive_subscr(framebuf,misc,0,176-2-miny,showtime,sspDOWN);
        //put the active subscreen above the passive subscreen
        put_active_subscr(misc,miny,sspDOWN);
        
        
        advanceframe(false);
        
        if(NESquit && Up() && cAbtn() && cBbtn())
        {
            Udown=true;
            Quit=qQUIT;
        }
        
        if(Quit)
            return;
            
        if(rSbtn())
            done=true;
    }
    while(!done);
    
    for(int y=6; y<=174; y+=3)
    {
        do_dcounters();
        Link.refill();
        //fill in the screen with black to prevent the hall of mirrors effect
        rectfill(framebuf, 0, 0, 255, 223, 0);
        
        if(COOLSCROLL)
        {
            //copy the playing field back onto the screen
            blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
        }
        else
        {
            //scroll the playing field (copy the copy we made)
            blit(scrollbuf,framebuf,256,0,0,176-2-y+passive_subscreen_height,256,y);
        }
        
        //throw the passive subscreen onto the screen
        put_passive_subscr(framebuf,misc,0,176-2-y,showtime,sspSCROLLING);
        //put the active subscreen above the passive subscreen
        put_active_subscr(misc,y,sspSCROLLING);
        advanceframe(false);
        
        if(Quit)
            return;
    }
    
    //  Sitems.clear();
    if(usebombpal)
    {
        memcpy(RAMpal, temppal, PAL_SIZE*sizeof(RGB));
    }
    
    resume_sfx(WAV_BRANG);
}

void markBmap(int dir, int sc)
{
    /*
      if((DMaps[get_currdmap()].type&dmfTYPE)==dmOVERW)
        return;
    */
    if(sc>=128)
    {
        return;
    }
    
    byte drow = DMaps[get_currdmap()].grid[sc>>4];
    byte mask = 1 << (7-((sc&15)-DMaps[get_currdmap()].xoff));
    int di = ((get_currdmap()-1)<<6) + ((sc>>4)<<3) + ((sc&15)-DMaps[get_currdmap()].xoff);
    int code = 0;
    
    
    switch((DMaps[get_currdmap()].type&dmfTYPE))
    {
    case dmDNGN:
    
        // check dmap
        if((drow&mask)==0)
            return;
            
        // calculate code
        for(int i=3; i>=0; i--)
        {
            code <<= 1;
            code += tmpscr->door[i]&1;
        }
        
        // mark the map
        game->bmaps[di] = code|128;
        break;
        
    case dmOVERW:
        break;
        
    default:
        game->bmaps[di] |= 128;
        
        if(dir>=0)
            game->bmaps[di] |= (1<<dir);
            
        break;
    }
}

void markBmap(int dir)
{
    markBmap(dir, get_currscr());
}

/*** end of subscr.cc ***/

