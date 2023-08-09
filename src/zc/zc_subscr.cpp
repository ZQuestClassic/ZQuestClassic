//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include <string.h>

#include "base/qrs.h"
#include "base/dmap.h"
#include "zc/zelda.h"
#include "subscr.h"
#include "zc/zc_subscr.h"
#include "zc/hero.h"
#include "gamedata.h"
#include "zc/guys.h"
#include "zc/ffscript.h"
#include "zc/replay.h"
#include "base/mapscr.h"
#include "base/misctypes.h"

extern HeroClass   Hero;
extern int32_t directItem;
extern int32_t directItemA;
extern int32_t directItemB;
extern int32_t directItemY;
extern int32_t directItemX;

//DIALOG *sso_properties_dlg;

void put_active_subscr(int32_t y, int32_t pos)
{
    //Don't call Sitems.animate() - that gets called somewhere else, somehow. -L
    animate_selectors();
    show_custom_subscreen(framebuf, new_subscreen_active, 0, 6-y, game->should_show_time(), pos);
}

void dosubscr()
{
    PALETTE temppal;
    
    if(tmpscr->flags3&fNOSUBSCR)
        return;
    
    if(usebombpal)
    {
        memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
        memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
        refreshpal=true;
    }
    
    int32_t miny;
    bool showtime = game->should_show_time();
    load_Sitems();
    
    pause_sfx(WAV_BRANG);
    
    if(current_item_id(itype_brang)>=0)
        pause_sfx(itemsbuf[current_item_id(itype_brang)].usesound);
        
    if(current_item_id(itype_hookshot)>=0)
        pause_sfx(itemsbuf[current_item_id(itype_hookshot)].usesound);
        
    adjust_sfx(QMisc.miscsfx[sfxLOWHEART],128,false);
    adjust_sfx(QMisc.miscsfx[sfxREFILL],128,false);
    adjust_sfx(QMisc.miscsfx[sfxDRAIN],128,false);
    
    set_clip_rect(scrollbuf, 0, 0, scrollbuf->w, scrollbuf->h);
    set_clip_rect(framebuf, 0, 0, framebuf->w, framebuf->h);
    
    //make a copy of the blank playing field on the right side of scrollbuf
    blit(scrollbuf,scrollbuf,0,playing_field_offset,256,0,256,176);
    //make a copy of the complete playing field on the bottom of scrollbuf
    blit(framebuf,scrollbuf,0,playing_field_offset,0,176,256,176);
    miny = 6;
    
	bool use_a = get_qr(qr_SELECTAWPN), use_x = get_qr(qr_SET_XBUTTON_ITEMS),
	     use_y = get_qr(qr_SET_YBUTTON_ITEMS);
	bool b_only = !(use_a||use_x||use_y||get_qr(qr_SUBSCR_PRESS_TO_EQUIP));
	//Set the selector to the correct position before bringing up the subscreen -DD
	if(!new_subscreen_active) return;
	bool compat = get_qr(qr_OLD_SUBSCR);
	if(compat)
	{
		new_subscreen_active->curpage = 0;
		auto& pg = new_subscreen_active->pages[0];
		if(Bwpn)
			pg.cursor_pos = zc_max(game->bwpn,0);
		else if(use_a && Awpn)
			pg.cursor_pos = zc_max(game->awpn,0);
		else if(use_x && Xwpn)
			pg.cursor_pos = zc_max(game->xwpn,0);
		else if(use_y && Ywpn)
			pg.cursor_pos = zc_max(game->ywpn,0);
		else pg.cursor_pos = 0;
	}
        
    for(int32_t y=176-2; y>=6; y-=3*Hero.subscr_speed)
    {
        do_dcounters();
        Hero.refill();
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
        put_passive_subscr(framebuf,0,176-2-y,showtime,sspSCROLLING);
        //put the active subscreen above the passive subscreen
        put_active_subscr(y,sspSCROLLING);
        
        advanceframe(false);
        
        if(Quit)
            return;
    }
    
    bool done=false;

    // Consume whatever input was registered during opening animation.
    if (replay_version_check(18))
        load_control_state();

    do
    {
		auto& pg = new_subscreen_active->cur_page();
		if (replay_version_check(0, 11))
			load_control_state();
		int32_t pos = pg.cursor_pos;
		
		if(rUp())         pg.move_cursor(SEL_UP);
		else if(rDown())  pg.move_cursor(SEL_DOWN);
		else if(rLeft())  pg.move_cursor(SEL_LEFT);
		else if(rRight()) pg.move_cursor(SEL_RIGHT);
		else if(compat)
		{
			if(rLbtn())
			{
				if (!get_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP))
				{
					pg.cursor_pos = pg.move_legacy(SEL_LEFT, pos, -1, -1, -1, false, true);
				}
			}
			else if(rRbtn() )
			{
				if (!get_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP)) 
				{
					pg.cursor_pos = pg.move_legacy(SEL_RIGHT, pos, -1, -1, -1, false, true);
				}
			}
			else if(rEx3btn() )
			{
				if ( use_a && get_qr(qr_USE_EX1_EX2_INVENTORYSWAP) )
				{
					selectNextAWpn(SEL_LEFT);
				}
			}
			else if(rEx4btn() )
			{
				if ( use_a && get_qr(qr_USE_EX1_EX2_INVENTORYSWAP) )
				{
					selectNextAWpn(SEL_RIGHT);
				}
			}
		}
		//Assign items to buttons
		bool can_equip = true;
		
		SubscrWidget* widg = pg.get_sel_widg();
		
		//!TODO SUBSCR If widg needs to respond to other buttons, check them here...
		if(widg && !(widg->flags & SUBSCR_CURITM_NONEQP))
		{
			auto eqwpn = widg->getItemVal();
			if(eqwpn > -1)
			{
				if(rBbtn() || b_only)
				{
					int32_t t = eqwpn;
					if(use_a && t == Awpn)
					{
						Awpn = Bwpn;
						game->awpn = game->bwpn;
						game->awpnpg = game->bwpnpg;
						directItemA = directItemB;
					}
					else if(use_x && t == Xwpn)
					{
						Xwpn = Bwpn;
						game->xwpn = game->bwpn;
						game->xwpnpg = game->bwpnpg;
						directItemX = directItemB;
					}
					else if(use_y && t == Ywpn)
					{
						Ywpn = Bwpn;
						game->ywpn = game->bwpn;
						game->ywpnpg = game->bwpnpg;
						directItemY = directItemB;
					}
					
					Bwpn = t;
					game->forced_bwpn = -1; //clear forced if the item is selected using the actual subscreen
					if(!b_only) sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
					
					game->bwpn = pg.cursor_pos;
					game->bwpnpg = new_subscreen_active->curpage;
					directItemB = directItem;
				}
				else if(use_a && rAbtn())
				{
					int32_t t = eqwpn;
					if(t == Bwpn)
					{
						Bwpn = Awpn;
						game->bwpn = game->awpn;
						game->bwpnpg = game->awpnpg;
						directItemB = directItemA;
					}
					else if(use_x && t == Xwpn)
					{
						Xwpn = Awpn;
						game->xwpn = game->awpn;
						game->xwpnpg = game->awpnpg;
						directItemX = directItemA;
					}
					else if(use_y && t == Ywpn)
					{
						Ywpn = Awpn;
						game->ywpn = game->awpn;
						game->ywpnpg = game->awpnpg;
						directItemY = directItemA;
					}
					
					Awpn = t;
					sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
					game->awpn = pg.cursor_pos;
					game->awpnpg = new_subscreen_active->curpage;
					game->forced_awpn = -1; //clear forced if the item is selected using the actual subscreen
					directItemA = directItem;
				}
				else if(use_x && rEx1btn())
				{
					int32_t t = eqwpn;
					if(t == Bwpn)
					{
						Bwpn = Xwpn;
						game->bwpn = game->xwpn;
						game->bwpnpg = game->xwpnpg;
						directItemB = directItemX;
					}
					else if(use_a && t == Awpn)
					{
						Awpn = Xwpn;
						game->awpn = game->xwpn;
						game->awpnpg = game->xwpnpg;
						directItemA = directItemX;
					}
					else if(use_y && t == Ywpn)
					{
						Ywpn = Xwpn;
						game->ywpn = game->xwpn;
						game->ywpnpg = game->xwpnpg;
						directItemY = directItemX;
					}
					
					Xwpn = t;
					sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
					game->xwpn = pg.cursor_pos;
					game->xwpnpg = new_subscreen_active->curpage;
					game->forced_xwpn = -1; //clear forced if the item is selected using the actual subscreen
					directItemX = directItem;
				}
				else if(use_y && rEx2btn())
				{
					int32_t t = eqwpn;
					if(t == Bwpn)
					{
						Bwpn = Ywpn;
						game->bwpn = game->ywpn;
						game->bwpnpg = game->ywpnpg;
						directItemB = directItemY;
					}
					else if(use_a && t == Awpn)
					{
						Awpn = Ywpn;
						game->awpn = game->ywpn;
						game->awpnpg = game->ywpnpg;
						directItemA = directItemY;
					}
					else if(use_x && t == Xwpn)
					{
						Xwpn = Ywpn;
						game->xwpn = game->ywpn;
						game->xwpnpg = game->ywpnpg;
						directItemX = directItemY;
					}
					
					Ywpn = t;
					sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
					game->ywpn = pg.cursor_pos;
					game->ywpnpg = new_subscreen_active->curpage;
					game->forced_ywpn = -1; //clear forced if the item is selected using the actual subscreen
					directItemY = directItem;
				}
			}
		}
        if(pos!=pg.cursor_pos)
            sfx(QMisc.miscsfx[sfxSUBSCR_CURSOR_MOVE]);
            
        do_dcounters();
        Hero.refill();
        
        //put_passive_subscr(framebuf,0,174-miny,showtime,true);
        //blit(scrollbuf,framebuf,0,6,0,6-miny,256,168);
        //put_active_subscr(miny,true);
        
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
        put_passive_subscr(framebuf,0,176-2-miny,showtime,sspDOWN);
        //put the active subscreen above the passive subscreen
        put_active_subscr(miny,sspDOWN);
        
        
        advanceframe(false);
		if (replay_version_check(11))
			load_control_state();
        
        if(NESquit && Up() && cAbtn() && cBbtn())
        {
            down_control_states[btnUp] = true;
            Quit=qQUIT;
        }
        
        if(Quit)
            return;
            
        if(rSbtn())
            done=true;
    }
    while(!done);
    for(int32_t y=6; y<=174; y+=3*Hero.subscr_speed)
    {
        do_dcounters();
        Hero.refill();
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
        put_passive_subscr(framebuf,0,176-2-y,showtime,sspSCROLLING);
        //put the active subscreen above the passive subscreen
        put_active_subscr(y,sspSCROLLING);
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

void markBmap(int32_t dir, int32_t sc)
{
    /*
      if((DMaps[get_currdmap()].type&dmfTYPE)==dmOVERW)
        return;
    */
    if(sc>=128 || sc<0)
    {
        return;
    }
    
    byte drow = DMaps[get_currdmap()].grid[sc>>4];
    byte mask = 1 << (7-((sc&15)-DMaps[get_currdmap()].xoff));
    int32_t di = (get_currdmap() << 7) + (sc & 0x7F); //+ ((sc&0xF)-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff));
    int32_t code = 0;
    
    
    switch((DMaps[get_currdmap()].type&dmfTYPE))
    {
    case dmDNGN:
		if(get_qr(qr_DUNGEONS_USE_CLASSIC_CHARTING))
		{
			// check dmap
			if((drow&mask)==0) //Only squares marked in dmap editor can be charted
				return;
				
			// calculate code
			for(int32_t i=3; i>=0; i--)
			{
				code <<= 1;
				code += tmpscr->door[i]&1; //Mark directions only for sides that have the door state set
			}
			
			// mark the map
			game->bmaps[di] = code|128;
		}
		else goto bmaps_default;
        break;
        
    case dmOVERW:
		if(get_qr(qr_NO_OVERWORLD_MAP_CHARTING))
			break;
        
    default:
	bmaps_default:
        game->bmaps[di] |= 128;
        
        if(dir>=0)
            game->bmaps[di] |= (1<<dir);
            
        break;
    }
}

void markBmap(int32_t dir)
{
    markBmap(dir, get_currscr());
}

