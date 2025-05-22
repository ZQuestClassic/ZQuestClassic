#include <cstring>

#include "base/qrs.h"
#include "base/dmap.h"
#include "base/util.h"
#include "base/zdefs.h"
#include "zc/maps.h"
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
#include "zscriptversion.h"

extern int32_t directItemA;
extern int32_t directItemB;
extern int32_t directItemY;
extern int32_t directItemX;

//DIALOG *sso_properties_dlg;

int current_subscr_pos = sspUP;
static bool opening_subscr_show_bottom_8px;

void put_active_subscr(int32_t y, int32_t pos)
{
	// Active subscreens do not yet get to use those 8 extra pixels. Until then,
	// draw 8 black pixels at the top (as the active subscreen will move an additional
	// 8 pixels and so must obscure the playing field somehow).
	if (opening_subscr_show_bottom_8px)
	{
		rectfill(framebuf, 0, y, framebuf->w, y+8-1, BLACK);
		y += 8;
	}
    show_custom_subscreen(framebuf, new_subscreen_active, 0, y, game->should_show_time(), pos);
}

void draw_subscrs(BITMAP* dest, int x, int y, bool showtime, int pos)
{
	int distance = opening_subscr_show_bottom_8px ? 176 : 168;
	if(get_qr(qr_OLD_SUBSCR))
	{
		put_passive_subscr(dest,x,y+distance,showtime,pos);
		put_active_subscr(y,pos);
	}
	else
	{
		put_active_subscr(y,pos);
		put_passive_subscr(dest,x,y+distance,showtime,pos);
	}
}

void dosubscr()
{
	PALETTE temppal;
	
	if (no_subscreen())
		return;
	
	if(usebombpal)
	{
		memcpy(temppal, RAMpal, PAL_SIZE*sizeof(RGB));
		memcpy(RAMpal, tempbombpal, PAL_SIZE*sizeof(RGB));
		refreshpal=true;
	}
	
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

	int h = is_extended_height_mode() ? 240 : 176;
	BITMAP* subscr_scrolling_bitmap = create_bitmap(256, h);

	// Copy the complete frame.
	blit(framebuf,subscr_scrolling_bitmap,0,playing_field_offset,0,0,256,h);
	
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

	opening_subscr_show_bottom_8px = show_bottom_8px;

	int distance = opening_subscr_show_bottom_8px ? 176 : 168;
	int offy = is_extended_height_mode() ? 0 : passive_subscreen_height;

	FFCore.initZScriptSubscreenScript();
	subscrpg_clear_animation();
	subscreen_open = true;
	current_subscr_pos = sspSCROLLING;
	for(int32_t y = -distance; y <= 0; y += 3*Hero.subscr_speed)
	{
		if(replay_version_check(19))
		{
			load_control_state();
			script_drawing_commands.Clear();
		}
		active_sub_yoff = y-playing_field_offset;
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0))
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
		do_dcounters();
		Hero.refill();
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0) && FFCore.waitdraw(ScriptType::EngineSubscreen,0))
		{
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
			FFCore.waitdraw(ScriptType::EngineSubscreen,0) = false;
		}
		//fill in the screen with black to prevent the hall of mirrors effect
		clear_to_color(framebuf, BLACK);

		// With COOLSCROLL on, the subscreen crawls down over the playing field.
		// Otherwise the playing field scrolls down past the bottom of the screen.
		if(COOLSCROLL)
		{
			blit(subscr_scrolling_bitmap,framebuf,0,0,0,offy,256,h);
		}
		else
		{
			blit(subscr_scrolling_bitmap,framebuf,0,0,0,y+distance+passive_subscreen_height,256,-y+(opening_subscr_show_bottom_8px?8:0));
		}
		
		draw_subscrs(framebuf,0,y,showtime,sspSCROLLING);
		if(replay_version_check(19))
			do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
		
		advanceframe(false);
		
		if(Quit)
			return;
	}
	active_sub_yoff = -playing_field_offset;
	current_subscr_pos = sspDOWN;
	bool done=false;

	// Consume whatever input was registered during opening animation.
	if (replay_version_check(18))
		load_control_state();

	bool legacy_btn_press_peek = false;
	if (replay_is_active())
	{
		std::string qst = replay_get_meta_str("qst");
		legacy_btn_press_peek |= qst == "demosp253.qst";
		legacy_btn_press_peek |= qst == "first_quest_layered.qst";
		legacy_btn_press_peek |= qst == "hell_awaits.qst";
	}

	do
	{
		if (replay_version_check(0, 11))
			load_control_state();
		if(replay_version_check(19))
			script_drawing_commands.Clear();
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0))
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
		auto pgnum = new_subscreen_active->curpage;
		auto& pg = new_subscreen_active->cur_page();
		bool can_btn = !subscr_pg_animating;
		if(can_btn)
		{
			byte btn_press = getIntBtnInput(0xFF, true, false, false, false, legacy_btn_press_peek);
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
					must_equip = false, can_unequip = noverify;
				auto eqwpn = widg->getItemVal();
				if(widg->getType() == widgITEMSLOT)
				{
					if((widg->flags & SUBSCR_CURITM_NO_INTER_WO_ITEM)
						&& widg->getDisplayItem() < 0)
						can_interact = false;
					if(widg->flags & SUBSCR_CURITM_NONEQP)
						can_equip = false;
					if(widg->flags & SUBSCR_CURITM_NO_UNEQUIP)
						can_unequip = false;
					must_equip = !b_only && (widg->flags & SUBSCR_CURITM_NO_INTER_WO_EQUIP);
				}
				if(must_equip && (!can_equip || eqwpn < 0))
					can_interact = false;
				
				if(can_interact)
				{
					auto bpress = btn_press;
					if(must_equip)
					{
						bpress &= ((!can_unequip || Bwpn!=eqwpn) ? INT_BTN_B : 0)
							| ((use_a && (!can_unequip || Awpn!=eqwpn)) ? INT_BTN_A : 0)
							| ((use_x && (!can_unequip || Xwpn!=eqwpn)) ? INT_BTN_X : 0)
							| ((use_y && (!can_unequip || Ywpn!=eqwpn)) ? INT_BTN_Y : 0);
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
								if(can_unequip && !b_only && eqwpn == Bwpn)
								{
									Bwpn = -1;
									game->forced_bwpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->bwpn = 255;
									directItemB = -1;
								}
								else
								{
									if(use_a && eqwpn == Awpn)
									{
										Awpn = Bwpn;
										game->awpn = game->bwpn;
										directItemA = directItemB;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Bwpn;
										game->xwpn = game->bwpn;
										directItemX = directItemB;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Bwpn;
										game->ywpn = game->bwpn;
										directItemY = directItemB;
									}
									
									Bwpn = eqwpn;
									game->forced_bwpn = -1; //clear forced if the item is selected using the actual subscreen
									if(!b_only) sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->bwpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									directItemB = NEG_OR_MASK(eqwpn,0xFF);
								}
							}
							else if(use_a && (btn_press&INT_BTN_A))
							{
								if(can_unequip && eqwpn == Awpn)
								{
									Awpn = -1;
									game->forced_awpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->awpn = 255;
									directItemA = -1;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Awpn;
										game->bwpn = game->awpn;
										directItemB = directItemA;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Awpn;
										game->xwpn = game->awpn;
										directItemX = directItemA;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Awpn;
										game->ywpn = game->awpn;
										directItemY = directItemA;
									}
									
									Awpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->awpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_awpn = -1; //clear forced if the item is selected using the actual subscreen
									directItemA = NEG_OR_MASK(eqwpn,0xFF);
								}
							}
							else if(use_x && (btn_press&INT_BTN_EX1))
							{
								if(can_unequip && eqwpn == Xwpn)
								{
									Xwpn = -1;
									game->forced_xwpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->xwpn = 255;
									directItemX = -1;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Xwpn;
										game->bwpn = game->xwpn;
										directItemB = directItemX;
									}
									else if(use_a && eqwpn == Awpn)
									{
										Awpn = Xwpn;
										game->awpn = game->xwpn;
										directItemA = directItemX;
									}
									else if(use_y && eqwpn == Ywpn)
									{
										Ywpn = Xwpn;
										game->ywpn = game->xwpn;
										directItemY = directItemX;
									}
									
									Xwpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->xwpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_xwpn = -1; //clear forced if the item is selected using the actual subscreen
									directItemX = NEG_OR_MASK(eqwpn,0xFF);
								}
							}
							else if(use_y && (btn_press&INT_BTN_EX2))
							{
								if(can_unequip && eqwpn == Ywpn)
								{
									Ywpn = -1;
									game->forced_ywpn = -1;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									
									game->ywpn = 255;
									directItemY = -1;
								}
								else
								{
									if(eqwpn == Bwpn)
									{
										Bwpn = Ywpn;
										game->bwpn = game->ywpn;
										directItemB = directItemY;
									}
									else if(use_a && eqwpn == Awpn)
									{
										Awpn = Ywpn;
										game->awpn = game->ywpn;
										directItemA = directItemY;
									}
									else if(use_x && eqwpn == Xwpn)
									{
										Xwpn = Ywpn;
										game->xwpn = game->ywpn;
										directItemX = directItemY;
									}
									
									Ywpn = eqwpn;
									sfx(QMisc.miscsfx[sfxSUBSCR_ITEM_ASSIGN]);
									game->ywpn = ((pg.cursor_pos)<<8) | new_subscreen_active->curpage;
									game->forced_ywpn = -1; //clear forced if the item is selected using the actual subscreen
									directItemY = NEG_OR_MASK(eqwpn,0xFF);
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
		
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0) && FFCore.waitdraw(ScriptType::EngineSubscreen,0))
		{
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
			FFCore.waitdraw(ScriptType::EngineSubscreen,0) = false;
		}

		clear_to_color(framebuf, BLACK);

		if(compat && COOLSCROLL) //copy the playing field back onto the screen
			blit(subscr_scrolling_bitmap,framebuf,0,0,0,offy,256,h);
		//else nothing to do; the playing field has scrolled off the screen
		
		//draw the passive and active subscreen
		draw_subscrs(framebuf,0,0,showtime,sspDOWN);
		if(replay_version_check(19))
			do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
		
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
	current_subscr_pos = sspSCROLLING;
	for(int32_t y = 0; y >= -distance; y -= 3*Hero.subscr_speed)
	{
		if(replay_version_check(19))
		{
			load_control_state();
			script_drawing_commands.Clear();
		}
		active_sub_yoff = y-playing_field_offset;
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0))
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
		do_dcounters();
		Hero.refill();
		if(new_subscreen_active->script && FFCore.doscript(ScriptType::EngineSubscreen,0) && FFCore.waitdraw(ScriptType::EngineSubscreen,0))
		{
			ZScriptVersion::RunScript(ScriptType::EngineSubscreen, new_subscreen_active->script, 0);
			FFCore.waitdraw(ScriptType::EngineSubscreen,0) = false;
		}
		//fill in the screen with black to prevent the hall of mirrors effect
		clear_to_color(framebuf, BLACK);
		
		if(COOLSCROLL)
		{
			blit(subscr_scrolling_bitmap,framebuf,0,0,0,offy,256,h);
		}
		else
		{
			blit(subscr_scrolling_bitmap,framebuf,0,0,0,y+distance+passive_subscreen_height,256,-y+(opening_subscr_show_bottom_8px?8:0));
		}
		
		draw_subscrs(framebuf,0,y,showtime,sspSCROLLING);
		if(replay_version_check(19))
			do_script_draws(framebuf, origin_scr, 0, playing_field_offset);
		advanceframe(false);
		
		if(Quit)
			return;
	}
	active_sub_yoff = -224;
	subscreen_open = false;
	current_subscr_pos = sspUP;
	updateShowBottomPixels(); // might have changed.
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
				code += origin_scr->door[i]&1; //Mark directions only for sides that have the door state set
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
void markBmap()
{
	markBmap(-1, hero_screen);
}

void put_passive_subscr(BITMAP *dest,int32_t x,int32_t y,bool showtime,int32_t pos2)
{
	++subscr_item_clk;
	animate_subscr_buttonitems();
	BITMAP *subscr = create_sub_bitmap(dest,x,y,256,passive_subscreen_height);

	if(no_subscreen())
	{
		if (playing_field_offset)
			rectfill(subscr, 0, 0, subscr->w, playing_field_offset, 0);
		destroy_bitmap(subscr);
		return;
	}

	// Since regions were added, all passive subscreens get a black background underneath (unless
	// using an extended height viewport). Previously, custom but blank subscreens could be used to
	// allow scripts to draw to the entire screen (ex: yuurand), but post-z3 such use cases should
	// enable the "no subscreen" screen flag.
	if (QHeader.is_z3 && playing_field_offset)
		rectfill(subscr, 0, 0, subscr->w, playing_field_offset - 1, 0);

	int32_t prev_currscr;
	if (currscr_for_passive_subscr != -1)
	{
		prev_currscr = cur_screen;
		cur_screen = currscr_for_passive_subscr;
	}

	show_custom_subscreen(subscr, new_subscreen_passive, 0, 0, showtime, pos2);
	destroy_bitmap(subscr);
	if(new_subscreen_overlay)
	{
		subscr = create_sub_bitmap(dest,x,0,dest->w,dest->h);
		show_custom_subscreen(subscr, new_subscreen_overlay, 0, 0, showtime, pos2);
		destroy_bitmap(subscr);
	}

	if (currscr_for_passive_subscr != -1)
		cur_screen = prev_currscr;
}
