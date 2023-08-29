//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include <cstring>

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

extern HeroClass Hero;
extern FFScript FFCore;

//DIALOG *sso_properties_dlg;

void put_active_subscr(int32_t y, int32_t pos)
{
    show_custom_subscreen(framebuf, new_subscreen_active, 0, 6-y, game->should_show_time(), pos);
}

void draw_subscrs(BITMAP* dest, int x, int y, bool showtime, int pos)
{
	if(get_qr(qr_OLD_SUBSCR))
	{
		put_passive_subscr(dest,x,176-2-y,showtime,pos);
		put_active_subscr(y,pos);
	}
	else
	{
		put_active_subscr(y,pos);
		put_passive_subscr(dest,x,176-2-y,showtime,pos);
	}
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
	if(replay_version_check(0,19))
		refresh_subscr_items();
	else flushItemCache();
	
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
	bool noverify = get_qr(qr_NO_BUTTON_VERIFY);
	if(compat)
	{
		new_subscreen_active->curpage = 0;
		auto& pg = new_subscreen_active->pages[0];
		if((game->bwpn&0xFF) == 0)
			pg.cursor_pos = game->bwpn>>8;
		else if((game->awpn&0xFF) == 0)
			pg.cursor_pos = game->awpn>>8;
		else if((game->xwpn&0xFF) == 0)
			pg.cursor_pos = game->xwpn>>8;
		else if((game->ywpn&0xFF) == 0)
			pg.cursor_pos = game->ywpn>>8;
		else pg.cursor_pos = 0;
	}
	
	subscrpg_clear_animation();
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
		
		draw_subscrs(framebuf,0,y,showtime,sspSCROLLING);
		
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
		auto pgnum = new_subscreen_active->curpage;
		auto& pg = new_subscreen_active->cur_page();
		if (replay_version_check(0, 11))
			load_control_state();
		bool can_btn = !subscr_pg_animating;
		if(can_btn)
		{
			byte btn_press = getIntBtnInput(0xFF, true, false, false, false, true);
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
						pg.cursor_pos = pg.movepos_legacy(SEL_LEFT, (pos<<8)|pg.getIndex(), 255, 255, 255, false, true)>>8;
					}
				}
				else if(rRbtn() )
				{
					if (!get_qr(qr_NO_L_R_BUTTON_INVENTORY_SWAP)) 
					{
						pg.cursor_pos = pg.movepos_legacy(SEL_RIGHT, (pos<<8)|pg.getIndex(), 255, 255, 255, false, true)>>8;
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
			
			if(pos!=pg.cursor_pos)
				sfx(QMisc.miscsfx[sfxSUBSCR_CURSOR_MOVE]);
			
			SubscrWidget* widg = pg.get_sel_widg();
			
			if(widg)
			{
				bool can_interact = true, can_equip = true,
					must_equip = false;
				auto eqwpn = widg->getItemVal();
				if(widg->getType() == widgITEMSLOT)
				{
					if((widg->flags & SUBSCR_CURITM_NO_INTER_WO_ITEM)
						&& widg->getDisplayItem() < 0)
						can_interact = false;
					if(widg->flags & SUBSCR_CURITM_NONEQP)
						can_equip = false;
					must_equip = !b_only && (widg->flags & SUBSCR_CURITM_NO_INTER_WO_EQUIP);
				}
				if(must_equip && (!can_equip || eqwpn < 0))
					can_interact = false;
				
				if(can_interact)
				{
					auto bpress = btn_press;
					if(must_equip)
					{
						bpress &= (Bwpn!=eqwpn ? INT_BTN_B : 0)
							| ((use_a && Awpn!=eqwpn) ? INT_BTN_A : 0)
							| ((use_x && Xwpn!=eqwpn) ? INT_BTN_X : 0)
							| ((use_y && Ywpn!=eqwpn) ? INT_BTN_Y : 0);
					}
					if(widg->generic_script && (bpress&widg->gen_script_btns))
					{
						FFCore.runGenericFrozenEngine(widg->generic_script, widg->generic_initd);
						eqwpn = widg->getItemVal(); //update incase script changed
						if(must_equip) //update values depending on eqwpn
						{
							bpress = btn_press;
							bpress &= (Bwpn!=eqwpn ? INT_BTN_B : 0)
								| ((use_a && Awpn!=eqwpn) ? INT_BTN_A : 0)
								| ((use_x && Xwpn!=eqwpn) ? INT_BTN_X : 0)
								| ((use_y && Ywpn!=eqwpn) ? INT_BTN_Y : 0);
						}
					}
					
					if(can_equip)
					{
						if(eqwpn > -1)
						{
							if(b_only || (btn_press&INT_BTN_B))
							{
								if(noverify && !b_only && eqwpn == Bwpn)
								{
									Bwpn = -1;
									game->forced_bwpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->bwpn = 255;
								}
								else
								{
									if(use_a && eqwpn == Awpn)
									{
										Awpn = Bwpn;
										game->awpn = game->bwpn;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Bwpn;
										game->xwpn = game->bwpn;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Bwpn;
										game->ywpn = game->bwpn;
									}
									
									Bwpn = eqwpn;
									game->forced_bwpn = -1; //clear forced if the item is selected using the actual subscreen
									if(!b_only) sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->bwpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
								}
							}
							else if(use_a && (btn_press&INT_BTN_A))
							{
								if(noverify && eqwpn == Awpn)
								{
									Awpn = -1;
									game->forced_awpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->awpn = 255;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Awpn;
										game->bwpn = game->awpn;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Awpn;
										game->xwpn = game->awpn;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Awpn;
										game->ywpn = game->awpn;
									}
									
									Awpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->awpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_awpn = -1; //clear forced if the item is selected using the actual subscreen
								}
							}
							else if(use_x && (btn_press&INT_BTN_EX1))
							{
								if(noverify && eqwpn == Xwpn)
								{
									Xwpn = -1;
									game->forced_xwpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->xwpn = 255;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Xwpn;
										game->bwpn = game->xwpn;
									}
									else if(use_a && eqwpn == Awpn)
									{
										Awpn = Xwpn;
										game->awpn = game->xwpn;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Xwpn;
										game->ywpn = game->xwpn;
									}
									
									Xwpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->xwpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_xwpn = -1; //clear forced if the item is selected using the actual subscreen
								}
							}
							else if(use_y && (btn_press&INT_BTN_EX2))
							{
								if(noverify && eqwpn == Ywpn)
								{
									Ywpn = -1;
									game->forced_ywpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->ywpn = 255;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Ywpn;
										game->bwpn = game->ywpn;
									}
									else if(use_a && eqwpn == Awpn)
									{
										Awpn = Ywpn;
										game->awpn = game->ywpn;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Ywpn;
										game->xwpn = game->ywpn;
									}
									
									Ywpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->ywpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_ywpn = -1; //clear forced if the item is selected using the actual subscreen
								}
							}
						}
					}
					if(!must_equip || eqwpn > -1)
						widg->check_btns(bpress,*new_subscreen_active);
				}
			}
			if(new_subscreen_active->curpage == pgnum)
				new_subscreen_active->check_btns(btn_press);
		}
		
		do_dcounters();
		Hero.refill();
		
		rectfill(framebuf, 0, 0, 255, 223, 0);
		
		if(compat && COOLSCROLL) //copy the playing field back onto the screen
			blit(scrollbuf,framebuf,0,176,0,passive_subscreen_height,256,176);
		//else nothing to do; the playing field has scrolled off the screen
		
		//draw the passive and active subscreen
		draw_subscrs(framebuf,0,miny,showtime,sspDOWN);
		
		advanceframe(false);
		if (replay_version_check(11))
			load_control_state();
		
		if(can_btn && NESquit && Up() && cAbtn() && cBbtn())
		{
			down_control_states[btnUp] = true;
			Quit=qQUIT;
		}
		
		if(Quit)
			return;
			
		if(can_btn && rSbtn())
			done=true;
	}
	while(!done);
	subscrpg_clear_animation();
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
		
		draw_subscrs(framebuf,0,y,showtime,sspSCROLLING);
		advanceframe(false);
		
		if(Quit)
			return;
	}
	
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

void put_passive_subscr(BITMAP *dest,int32_t x,int32_t y,bool showtime,int32_t pos2)
{
	++subscr_item_clk;
	animate_subscr_buttonitems();
	BITMAP *subscr = create_sub_bitmap(dest,x,y,256,passive_subscreen_height);
	
	if(no_subscreen())
	{
		clear_to_color(subscr,0);
		destroy_bitmap(subscr);
		return;
	}
	
	show_custom_subscreen(subscr, new_subscreen_passive, 0, 0, showtime, pos2);
	destroy_bitmap(subscr);
	if(new_subscreen_overlay)
	{
		subscr = create_sub_bitmap(dest,x,0,256,224);
		show_custom_subscreen(subscr, new_subscreen_overlay, 0, 0, showtime, pos2);
		destroy_bitmap(subscr);
	}
}

