#include "zc/message_string.h"
#include "subscr.h"
#include "core/msg_scroll.h"
#include "core/zdefs.h"
#include "core/qrs.h"
#include "core/fonts.h"
#include "core/misctypes.h"
#include "sprite_data.h"
#include "zc/zelda.h"
#include "zc/hero.h"
#include "zc/ffscript.h"
#include "zc/guys.h"
#include "items.h"

using namespace msgstr;

namespace // file local
{

enum
{
	MNU_CURSOR_TILE, MNU_CURSOR_CSET,
	MNU_CURSOR_WID, MNU_CURSOR_HEI, MNU_CURSOR_FLIP,
	
	MNU_CHOSEN, MNU_TIMER, MNU_CAN_CONFIRM,
	
	MNU_DATA_MAX
};
struct menu_choice
{
	int x, y;
	int pos;
	int upos, dpos, lpos, rpos;
	menu_choice() : x(0), y(0), pos(0), upos(0), dpos(0), lpos(0), rpos(0)
	{}
	menu_choice(int x, int y, int pos, int upos,
		int dpos, int lpos, int rpos)
		: x(x), y(y), pos(pos), upos(upos), dpos(dpos), lpos(lpos), rpos(rpos)
	{}
};
int msg_menu_data[MNU_DATA_MAX];
BITMAP *txt_bmp_buf = nullptr, *bg_bmp_buf = nullptr, *portrait_bmp_buf = nullptr, *menu_bmp_buf = nullptr;
BITMAP *txt_display_buf = nullptr, *bg_display_buf = nullptr, *portrait_display_buf = nullptr;
bool do_run_menu = false,
	just_confirmed_menu = false,
	do_end_str = false,
	wait_advance = false,
	wait_advance_unprocessed_iter = false;
std::map<int, menu_choice> menu_options;
word xpos = 0, ypos = 0, cursor_x = 0, cursor_y = 0, width = 0, height = 0;
word enqueued_str = 0;
word speed = 0;
int used_height = -1, used_height_buf = -1;
byte shadow_type = 0, shadow_color = 0, text_color = 0;
FONT *msg_font;
optional<MsgStr::iterator> cur_iterator;
message_portrait portrait;
int16_t margins[4] = {0};

MsgScrollState scroll;
int txt_bmp_height = 0;
const int txt_bmp_width = 512+16;

void setmsg(int str)
{
	active_str = str;
	auto& msgstring = get_str();
	cur_iterator = msgstring.create_iterator();
	msg_font = get_zc_font(msgstring.font);
}
void draw_bg(MsgStr const& msg)
{
	if (msg.tile == 0) return;
	if (msg.stringflags & STRINGFLAG_FULLTILE)
	{
		draw_block_flip(bg_bmp_buf, 0, 0, msg.tile, msg.cset,
			(int32_t)ceil(msg.w/16.0), (int32_t)ceil(msg.h/16.0), 0, true, false);
	}
	else
	{
		int32_t add = (get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 2 : 0);
		frame2x2(bg_bmp_buf, 0, 0, msg.tile, msg.cset,
			(msg.w>>3) + add, (msg.h>>3) + add, 0, true, 0);
	}
}
void draw_prt()
{
	clear_bitmap(portrait_bmp_buf);
	if(portrait.tile > 0 && portrait.th > 0 && portrait.tw > 0)
	{
		draw_block_flip(portrait_bmp_buf, 0, 0, portrait.tile, portrait.cset,
			portrait.tw, portrait.th, 0, true, false);
	}
}

void ensure_txt_bmp_height(int new_height)
{
	new_height += margins[down]; // account for bottom margin as extra
	if (txt_bmp_height >= new_height)
		return;
	
	// resize, preserving content
	// to prevent constant resizing, over-allocate a bit of size
	new_height += 64;
	
	BITMAP *tmp_txt = txt_bmp_buf;
	BITMAP *tmp_menu = menu_bmp_buf;
	
	txt_bmp_buf = create_bitmap_ex(8, txt_bmp_width, new_height);
	menu_bmp_buf = create_bitmap_ex(8, txt_bmp_width, new_height);
	clear_bitmap(txt_bmp_buf);
	clear_bitmap(menu_bmp_buf);
	
	blit(tmp_txt, txt_bmp_buf, 0, 0, 0, 0, txt_bmp_width, txt_bmp_height);
	blit(tmp_menu, menu_bmp_buf, 0, 0, 0, 0, txt_bmp_width, txt_bmp_height);
	
	destroy_bitmap(tmp_txt);
	destroy_bitmap(tmp_menu);
	
	txt_bmp_height = new_height;
}

void update_max_scroll(int pos)
{
	if (scroll.update_max_scroll(pos))
		ensure_txt_bmp_height(scroll.max_visible_pos + margins[up] + margins[down]);
}

void ensure_scrolled_to(int pos, int h)
{
	if (scroll.ensure_scrolled_to(pos, h))
		ensure_txt_bmp_height(scroll.max_visible_pos + margins[up] + margins[down]);
}

void blit_bg(BITMAP* dest, int sx, int sy, int dx, int dy, int w, int h)
{
	if (bg_display_buf->clip) return;
	if (get_str().stringflags & STRINGFLAG_TRANS_BG)
	{
		BITMAP* subbmp = create_bitmap_ex(8,w,h);
		if (subbmp)
		{
			color_map = trans_table2;
			clear_bitmap(subbmp);
			masked_blit(bg_display_buf, subbmp, sx, sy, 0, 0, w, h);
			draw_trans_sprite(dest, subbmp, dx, dy);
			destroy_bitmap(subbmp);
			color_map = trans_table;
		}
	}
	else
	{
		masked_blit(bg_display_buf, dest, sx, sy, dx, dy, w, h);
	}
}
void blit_fg(BITMAP* dest, int sx, int sy, int dx, int dy, int w, int h)
{
	if (txt_display_buf->clip) return;
	if (get_str().stringflags & STRINGFLAG_TRANS_FG)
	{
		BITMAP* subbmp = create_bitmap_ex(8,w,h);
		if (subbmp)
		{
			color_map = trans_table2;
			clear_bitmap(subbmp);
			masked_blit(txt_display_buf, subbmp, sx, sy, 0, 0, w, h);
			draw_trans_sprite(dest, subbmp, dx, dy);
			destroy_bitmap(subbmp);
			color_map = trans_table;
		}
	}
	else
	{
		masked_blit(txt_display_buf, dest, sx, sy, dx, dy, w, h);
	}
}
void blit_prt(BITMAP* dest, int sx, int sy, int dx, int dy, int w, int h)
{
	if (portrait_display_buf->clip) return;
	masked_blit(portrait_display_buf, dest, sx, sy, dx, dy, w, h);
}

void update()
{
	if (!active_str) return;
	
	set_clip_state(bg_display_buf, 0);
	blit(bg_bmp_buf, bg_display_buf, 0, 0, xpos, ypos, width+16, height+16);
	set_clip_state(txt_display_buf, 0);
	if(get_qr(qr_OLD_STRING_EDITOR_MARGINS)!=0)
	{
		blit(txt_bmp_buf, txt_display_buf, 0, 0, xpos, ypos, width+16, height+16);
		masked_blit(menu_bmp_buf, txt_display_buf, 0, 0, xpos, ypos, width+16, height+16);
	}
	else
	{
		int ty = scroll.cur_scroll();
		blit(txt_bmp_buf, txt_display_buf, margins[left], ty+margins[up], xpos+margins[left], ypos+margins[up], width-margins[left]-margins[right], height-margins[up]-margins[down]);
		masked_blit(menu_bmp_buf, txt_display_buf, margins[left], ty+margins[up], xpos+margins[left], ypos+margins[up], width-margins[left]-margins[right], height-margins[up]-margins[down]);
	}
	set_clip_state(portrait_display_buf, 0);
	blit(portrait_bmp_buf, portrait_display_buf, 0, 0, portrait.x, portrait.y, portrait.tw*16, portrait.th*16);
}

bool runMenuCursor()
{
	clear_bitmap(menu_bmp_buf);
	if(!menu_options.size())
	{
		msg_menu_data[MNU_CHOSEN] = 0;
		return true; //end menu
	}
	int32_t pos = msg_menu_data[MNU_CHOSEN];
	//If the cursor is at an invalid pos, find the first pos >= 0...
	if(menu_options.find(pos) == menu_options.end())
	{
		pos = 0;
		while(menu_options.find(pos) == menu_options.end())
			++pos;
	}
	menu_choice* ch = &menu_options[pos];
	
	bool pressed = true;
	if(getInput(btnUp, INPUT_PRESS)) pos = ch->upos;
	else if(getInput(btnDown, INPUT_PRESS)) pos = ch->dpos;
	else if(getInput(btnLeft, INPUT_PRESS)) pos = ch->lpos;
	else if(getInput(btnRight, INPUT_PRESS)) pos = ch->rpos;
	else pressed = false;
	
	if(pressed)
		msg_menu_data[MNU_TIMER] = 1;
	
	bool hold_input = !((msg_menu_data[MNU_TIMER]++) % 20);
	bool held = false;
	if(hold_input)
	{
		held = true;
		if(getInput(btnUp)) pos = ch->upos;
		else if(getInput(btnDown)) pos = ch->dpos;
		else if(getInput(btnLeft)) pos = ch->lpos;
		else if(getInput(btnRight)) pos = ch->rpos;
		else held = false;
	}
	//If the cursor is at an invalid pos, find the first pos >= 0...
	if(menu_options.find(pos) == menu_options.end())
	{
		pos = 0;
		while(menu_options.find(pos) == menu_options.end())
			++pos;
	}
	if((pressed || held) && pos != msg_menu_data[MNU_CHOSEN])
		sfx(get_str().menu_move_sfx);
	
	if(!msg_menu_data[MNU_CAN_CONFIRM]) //Prevent instantly accepting when holding A
	{
		getInput(btnA, INPUT_PRESS); //Eat
		if(!getInput(btnA)) msg_menu_data[MNU_CAN_CONFIRM] = 1;
	}
	
	bool ret = (pressed || held) ? false : getInput(btnA, INPUT_PRESS);
	//Eat inputs
	getInput(btnUp, INPUT_PRESS);
	getInput(btnDown, INPUT_PRESS);
	getInput(btnLeft, INPUT_PRESS);
	getInput(btnRight, INPUT_PRESS);
	getInput(btnA, INPUT_PRESS);
	
	ch = &menu_options[pos];
	ensure_scrolled_to(ch->y - margins[up], msg_menu_data[MNU_CURSOR_HEI]);
	
	overtileblock16(menu_bmp_buf, msg_menu_data[MNU_CURSOR_TILE],
		ch->x, ch->y, (int32_t)ceil(msg_menu_data[MNU_CURSOR_WID]/16.0),
		(int32_t)ceil(msg_menu_data[MNU_CURSOR_HEI]/16.0),
		msg_menu_data[MNU_CURSOR_CSET], msg_menu_data[MNU_CURSOR_FLIP]);
	
	msg_menu_data[MNU_CHOSEN] = pos;
	
	if(ret)
	{
		 // imprint the final cursor on the txt_bmp_buf, to stay visible for scrolling and such
		overtileblock16(txt_bmp_buf, msg_menu_data[MNU_CURSOR_TILE],
			ch->x, ch->y, (int32_t)ceil(msg_menu_data[MNU_CURSOR_WID]/16.0),
			(int32_t)ceil(msg_menu_data[MNU_CURSOR_HEI]/16.0),
			msg_menu_data[MNU_CURSOR_CSET], msg_menu_data[MNU_CURSOR_FLIP]);
		
		menu_options.clear();
		just_confirmed_menu = true;
		sfx(get_str().menu_close_sfx);
	}
	
	return ret;
	//false if pos changed this frame; no confirming while moving the cursor!
}

bool bottom_margin_clip()
{
	return msg_layout::bottom_margin_clip(cursor_y, height, margins[down], scroll.can_scroll,
		get_qr(qr_OLD_STRING_EDITOR_MARGINS), get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT));
}

void wait_to_advance(bool next_string = false)
{
	if (do_end_str)
	{
		linked_clk = 1;
	}
	else
	{
		wait_advance = !next_string;
		if (just_confirmed_menu)
			linked_clk = 1;
		else
			linked_clk = 1 + zinit.msg_advance_delay;
	}
	// handle max_visible_pos for segmented mode properly
	scroll.segment_crossed(scroll.target_scroll_pos, cursor_y - margins[up] + used_height_buf);
}

bool next_line()
{
	used_height = used_height_buf;
	int32_t thei = zc_max(used_height, text_height(msg_font));
	used_height_buf = text_height(msg_font);
	auto ty = cursor_y - margins[up];
	cursor_y += thei + get_str().vspace;
	if (bottom_margin_clip())
		return false;
	cursor_x = margins[left];
	if (auto seg_row = scroll.segment_crossed(ty, cursor_y - margins[up] + used_height_buf))
	{
		cursor_y = *seg_row + margins[up];
		if (menu_options.empty()) // don't pause if menu options are being processed
			wait_to_advance();
	}
	return true;
}

bool parsemsgcode(const StringCommand& command)
{
	auto args = command.args;
	int last_arg = 0;
	
	if (get_qr(qr_OLD_LONG_SCC_ARGS))
	{
		int idx = -1;
		switch (command.code)
		{
			case MSGC_GOTOIFSCREEND:
				idx = 1;
				break;
			case MSGC_SETSCREEND:
				idx = 3;
				break;
			case MSGC_SETCURRENTSCREEND:
				idx = 1;
				break;
			case MSGC_GOTOIFCREEND:
				idx = 3;
				break;
		}
		if (idx > -1)
			args[idx] = zslongToFix(args[idx].getTrunc());
	}

	switch (command.code)
	{
		case MSGC_NEWLINE:
		{
			next_line();
			if (wait_advance)
				return true; // not wait_advance_unprocessed_iter, as the newline would be double-applied
			ensure_scrolled_to(cursor_y - margins[up], used_height_buf);
			return true;
		}	
		
		case MSGC_COLOUR:
		{
			int32_t cset = args[0];
			text_color = CSET(cset)+(args[1]);
			return true;
		}
		
		case MSGC_SHDCOLOR:
		{
			int32_t cset = args[0];
			shadow_color = CSET(cset)+args[1];
			return true;
		}
		case MSGC_SHDTYPE:
		{
			shadow_type = args[0];
			return true;
		}
		
		case MSGC_SPEED:
		{
			speed = args[0];
			return true;
		}
		
		case MSGC_CTRUP:
		{
			int32_t a1 = args[0];
			int32_t a2 = args[1];
			game->change_counter(a2, a1);
			return true;
		}
		
		case MSGC_CTRDN:
		{
			int32_t a1 = args[0];
			int32_t a2 = args[1];
			game->change_counter(-a2, a1);
			return true;
		}
		
		case MSGC_CTRSET:
		{
			int32_t a1 = args[0];
			int32_t a2 = args[1];
			game->set_counter(vbound(a2, 0, game->get_maxcounter(a1)), a1);
			return true;
		}
		
		case MSGC_CTRUPPC:
		case MSGC_CTRDNPC:
		case MSGC_CTRSETPC:
		{
			int32_t counter = args[0];
			int32_t amount = args[1];
			amount = int32_t(vbound(amount*0.01, 0.0, 1.0)*game->get_maxcounter(counter));
			
			if(command.code==MSGC_CTRDNPC)
				amount*=-1;
				
			if(command.code==MSGC_CTRSETPC)
				game->set_counter(amount, counter);
			else
				game->change_counter(amount, counter);
				
			return true;
		}
		
		case MSGC_GIVEITEM:
		{
			int32_t itemID = args[0];
			
			if (valid_item_id(itemID))
			{
				getitem(itemID, true);
				if (!FFCore.doscript(ScriptType::Item, itemID))
				{
					FFCore.reset_script_engine_data(ScriptType::Item, itemID);
					FFCore.doscript(ScriptType::Item, itemID) = (itemsbuf.get(itemID).flags&item_passive_script) > 0;
				}
			}
			return true;
		}

		case MSGC_COLLECTITEM:
		{
			int32_t itemID = args[0];

			if (!items.has_space())
			{
				Z_error("SCC: No room to spawn item\n");
				return true;
			}

			additem(0, (get_qr(qr_NOITEMOFFSET) ? 1: 0), itemID, ipHOLDUP);
			auto* s = (item*)items.spr(items.Count() - 1);
			s->screen_spawned = scr->screen;
			s->set_forcegrab(true);
			// The item will be collected by Hero::checkItems when the message box closes. However,
			// if qr_MSGFREEZE is off, this command will instantly close the message box. It's
			// expected that new quests have that QR on.
			return true;
		}
		
		case MSGC_WARP:
		{
			int32_t dmap =  args[0];
			int32_t scrn =  args[1];
			int32_t dx =  args[2];
			int32_t dy =  args[3];
			int32_t wfx =  args[4];
			int32_t sfx =  args[5];
			if(dx >= 65023) dx = -1;
			if(dy >= 65023) dy = -1;
			FFCore.warp_player(wtIWARP, dmap, scrn, dx, dy, wfx, sfx, 0, 0);
			do_end_str = true;
			return true;
		}
		
		case MSGC_SETSCREEND:
		{
			int32_t dmap =     args[0];
			int32_t screen =     args[1];
			int32_t reg =     args[2];
			int32_t val =     args[3].getZLong();
			FFCore.set_screen_d(screen + dmap*128, reg, val);
			return true;
		}
		case MSGC_SETCURRENTSCREEND:
		{
			int32_t dmap =    cur_dmap;
			int32_t screen =  cur_screen;
			int32_t reg =     args[0];
			int32_t val =     args[1].getZLong();
			FFCore.set_screen_d(screen + dmap*128, reg, val);
			return true;
		}
		case MSGC_TAKEITEM:
		{
			int32_t itemID = args[0];
			if ( FFCore.doscript(ScriptType::Item, itemID) )
			{
				FFCore.doscript(ScriptType::Item, itemID) = 4; //Val of 4 means 'clear stack and quit'
			}
			takeitem(itemID);
			if ( game->forced_bwpn == itemID ) 
			{
				game->forced_bwpn = -1;
			} //not else if! -Z
			if ( game->forced_awpn == itemID ) 
			{
				game->forced_awpn = -1;
			}
			if ( game->forced_xwpn == itemID ) 
			{
				game->forced_xwpn = -1;
			} //not else if! -Z
			if ( game->forced_ywpn == itemID ) 
			{
				game->forced_ywpn = -1;
			}
			verifyBothWeapons();
			return true;
		}
			
		case MSGC_SFX:
		{
			sfx(args[0]);
			return true;
		}
		
		case MSGC_MIDI:
		{
			int32_t music = args[0];
			
			if(music==0)
				music_stop();
			else
				jukebox(music+MIDIOFFSET_ZSCRIPT);
				
			return true;
		}
		
		case MSGC_MUSIC:
		{
			int32_t music = args[0];
			
			if (music == -1)
				playLevelMusic();
			else if(unsigned(music - 1) < quest_music.size())
				quest_music[music - 1].play();
			else
				music_stop();
				
			return true;
		}
		
		case MSGC_MUSIC_REFRESH:
		{
			int32_t refresh_type = args[0];
			
			if(refresh_type >= MUSIC_UPDATE_SCREEN && refresh_type <= MUSIC_UPDATE_REGION)
				FFCore.music_update_cond = refresh_type;
				
			return true;
		}
		
		case MSGC_FONT:
		{
			int fontid = args[0];
			int oh = text_height(msg_font);
			msg_font = get_zc_font(fontid);
			int nh = text_height(msg_font);
			int mh = std::max(oh,nh);
			if(mh > used_height_buf)
				used_height_buf = mh;
			return true;
		}
		case MSGC_RUN_FRZ_GENSCR:
		{
			just_confirmed_menu = false;
			word scr_id = args[0];
			bool force_redraw = args[1]!=0;
			if(force_redraw)
			{
				update();
				draw_screen();
			}
			// Optional args (beyond script num and force_redraw) are passed as the
			// script's InitD. Any unspecified trailing InitD slots default to 0.
			if(command.num_args > 2)
			{
				script_config scrconfig;
				scrconfig.script = scr_id;
				for(int q = 0; q < 8 && q + 2 < command.num_args; ++q)
					scrconfig.run_args[q] = args[q + 2].getZLong();
				FFCore.runGenericFrozenEngine(scrconfig);
			}
			else FFCore.runGenericFrozenEngine(scr_id);
			return true;
		}
		case MSGC_DRAWTILE:
		{
			int32_t tl = args[0];
			int32_t cs = args[1];
			int32_t t_wid = args[2];
			int32_t t_hei = args[3];
			int32_t fl = args[4];
			auto const& msgstring = get_str();
			
			if (msg_layout::wrap_needed(cursor_x, msgstring.hspace, t_wid, width, margins[right]))
			{
				if (!next_line())
					return true;
				if (wait_advance)
				{
					wait_advance_unprocessed_iter = true;
					return true;
				}
			}
			just_confirmed_menu = false;
			
			used_height_buf = zc_max(used_height_buf, t_hei);
			ensure_scrolled_to(cursor_y - margins[up], used_height_buf);
			overtileblock16(txt_bmp_buf, tl, cursor_x, cursor_y, (int32_t)ceil(t_wid/16.0), (int32_t)ceil(t_hei/16.0), cs, fl);
			cursor_x += msgstring.hspace + t_wid;
			return true;
		}
		
		case MSGC_GOTOIFRAND:
		{
			int32_t odds = args[0];

			last_arg = 1;
			if(!odds || !(zc_oldrand()%odds))
				goto switched;

			return true;
		}
		
		case MSGC_GOTOIFSCREEND:
		{
			int32_t arg = args[0];
			int32_t d = zc_min(7,arg);
			int32_t s = ((get_currdmap())<<7) + scr->screen-(DMaps[get_currdmap()].type==dmOVERW ? 0 : DMaps[get_currdmap()].xoff);
			arg = args[1].getZLong();

			if(game->screen_d.get(s).get(d) >= arg)
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}

		case MSGC_GOTOIFGLOBALSTATE:
		{
			int32_t index = args[0];
			bool value = args[1];

			if (unsigned(index) >= NUM_GSWITCHES)
			{
				Z_error("SCC: Global state %d is invalid\n", index);
				return true;
			}

			bool state = game->gswitch_timers.get(index) != 0;

			if (state == value)
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}

		case MSGC_SETGLOBALSTATE:
		{
			int32_t index = args[0];
			int32_t value = args[1];

			if (unsigned(index) >= NUM_GSWITCHES)
			{
				Z_error("SCC: Global state %d is invalid\n", index);
				return true;
			}

			if (bool(game->gswitch_timers.get(index)) != bool(value))
				toggle_gswitches(index, false);
			game->gswitch_timers[index] = value;

			return true;
		}

		case MSGC_GOTOIFSCREENSTATE:
		{
			int32_t flag = args[0];
			bool value = args[1];

			if (unsigned(flag) >= mMAXIND)
			{
				Z_error("SCC: Screen state %d is invalid\n", flag);
				return true;
			}

			bool state = getmapflag(scr, 1<<flag);
			if (state == value)
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}

		case MSGC_GOTOIFANYSCREENSTATE:
		{
			int32_t map = args[0];
			int32_t screen = args[1];
			int32_t flag = args[2];
			bool value = args[3];

			if (unsigned(flag)>=mMAXIND)
			{
				Z_error("SCC: Screen state %d is invalid\n", flag);
				return true;
			}

			if (map < 1 || map > map_count)
			{
				Z_error("SCC: Map %d is invalid\n", map);
				return true;
			}

			if (unsigned(screen)>=0x80)
			{
				Z_error("SCC: Screen %d is invalid\n", screen);
				return true;
			}
			if (!get_qr(qr_BROKEN_SCC_MAP_ARGS))
				--map; // 0-index

			int mi = mapind(map, screen);
			bool state = (game->maps.get(mi) & (1<<flag)) != 0;
			if (state == value)
			{
				last_arg = 4;
				goto switched;
			}

			return true;
		}

		case MSGC_GOTOIFSCREENEXSTATE:
		{
			int32_t flag = args[0];
			bool value = args[1];

			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Screen ExState %d is invalid\n", flag);
				return true;
			}

			bool state = getxmapflag(scr->screen, 1<<flag);
			if (state == value)
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}

		case MSGC_GOTOIFANYSCREENEXSTATE:
		{
			int32_t map = args[0];
			int32_t screen = args[1];
			int32_t flag = args[2];
			bool value = args[3];

			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Screen ExState %d is invalid\n", flag);
				return true;
			}

			if (map < 1 || map > map_count)
			{
				Z_error("SCC: Map %d is invalid\n", map);
				return true;
			}

			if (unsigned(screen)>=0x80)
			{
				Z_error("SCC: Screen %d is invalid\n", screen);
				return true;
			}
			if (!get_qr(qr_BROKEN_SCC_MAP_ARGS))
				--map; // 0-index

			bool state = getxmapflag(mapind(map, screen), 1<<flag);
			if (state == value)
			{
				last_arg = 4;
				goto switched;
			}

			return true;
		}

		case MSGC_GOTOIFLEVELSTATE:
		{
			int32_t level = args[0];
			int32_t flag = args[1];
			bool value = args[2];

			if (level == -1)
				level = dlevel;

			if (unsigned(level) >= game->lvlswitches.size())
			{
				Z_error("SCC: Level %d is invalid\n", level);
				return true;
			}

			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Level state %d is invalid\n", flag);
				return true;
			}

			bool state = game->lvlswitches.get(level) & (1<<flag);
			if (state == value)
			{
				last_arg = 3;
				goto switched;
			}

			return true;
		}

		case MSGC_SETLEVELSTATE:
		{
			int32_t level = args[0];
			int32_t flag = args[1];
			bool value = args[2];

			if (level == -1)
				level = dlevel;

			if (unsigned(level) >= game->lvlswitches.size())
			{
				Z_error("SCC: Level %d is invalid\n", level);
				return true;
			}

			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Level state %d is invalid\n", flag);
				return true;
			}

			if (level == dlevel)
			{
				if (bool(game->lvlswitches.get(level) & (1<<flag)) != value)
					toggle_switches(1<<flag, false);
			}
			else
			{
				SETFLAG(game->lvlswitches[level], 1<<flag, value);
			}

			return true;
		}

		case MSGC_GOTOIFLEVELITEM:
		{
			int32_t level = args[0];
			int32_t flag = args[1];
			bool value = args[2];

			if (level == -1)
				level = dlevel;

			if (unsigned(level) >= game->lvlitems.size())
			{
				Z_error("SCC: Level %d is invalid\n", level);
				return true;
			}

			if (unsigned(flag) >= 16)
			{
				Z_error("SCC: Level item %d is invalid\n", flag);
				return true;
			}

			bool state = game->lvlitems.get(level) & (1<<flag);
			if (state == value)
			{
				last_arg = 3;
				goto switched;
			}

			return true;
		}

		case MSGC_SETLEVELITEM:
		{
			int32_t level = args[0];
			int32_t flag = args[1];
			bool value = args[2];

			if (level == -1)
				level = dlevel;

			if (unsigned(level) >= game->lvlitems.size())
			{
				Z_error("SCC: Level %d is invalid\n", level);
				return true;
			}

			if (unsigned(flag) >= 16)
			{
				Z_error("SCC: Level item %d is invalid\n", flag);
				return true;
			}

			SETFLAG(game->lvlitems[level], 1<<flag, value);

			return true;
		}

		case MSGC_GOTOIFSECRETS:
		{
			if (get_screen_state(scr->screen).triggered_secrets)
			{
				last_arg = 0;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFCREEND:
		{
			int32_t dmap =     (args[0]<<7); //dmap and screen may be transposed here.
			int32_t screen =     args[1];
			int32_t reg =     args[2];
			int32_t val =     args[3].getZLong();
			if ( FFCore.get_screen_d(screen + dmap, reg) >= val )
			{
				last_arg = 4;
				goto switched;
			}
			return true;
		}
		
		case MSGC_GOTOIFITEM:
		{
			int32_t it = args[0];
			
			if(valid_item_id(it) && game->get_item(it))
			{
				last_arg = 1;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFCTR:
		{
			if(game->get_counter(args[0]) >= args[1])
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFCTRPC:
		{
			int32_t counter = args[0];
			int32_t amount = (int32_t)((args[1]/100)*game->get_maxcounter(counter));
			
			if(game->get_counter(counter)>=amount)
			{
				last_arg = 2;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFTRICOUNT:
		{
			if(TriforceCount() >= args[0])
			{
				last_arg = 1;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFTRI:
		{
			int32_t lev = args[0];
			
			if(unsigned(lev)<MAXLEVELS && game->lvlitems.get(lev)&(1 << li_mcguffin))
			{
				last_arg = 1;
				goto switched;
			}

			return true;
		}
		
		case MSGC_GOTOIFBOTTLE:
		{
			int quant = args[0];
			int type = args[1];
			if (game->hasBottle(type, quant))
			{
				last_arg = 2;
				goto switched;
			}
			return true;
		}
		case MSGC_CHANGEBOTTLE:
		{
			int quant = args[0];
			int old_type = args[1];
			int new_type = args[2];
			game->fillBottle(new_type, old_type, quant);
			return true;
		}
		
		case MSGC_CHANGEPORTRAIT:
		{
			portrait = {
				args[0], byte(args[1]),
				byte(args[2]), byte(args[3]),
				byte(args[4]), byte(args[5]),
			};
			draw_prt();
			return true;
		}
		
		case MSGC_SETUPMENU:
		{
			msg_menu_data[MNU_CURSOR_TILE] = args[0];
			msg_menu_data[MNU_CURSOR_CSET] = args[1];
			msg_menu_data[MNU_CURSOR_WID] = args[2];
			msg_menu_data[MNU_CURSOR_HEI] = args[3];
			msg_menu_data[MNU_CURSOR_FLIP] = args[4];
			return true;
		}
		
		case MSGC_MENUCHOICE:
		{
			just_confirmed_menu = false;
			int32_t pos = args[0];
			int32_t upos = args[1];
			int32_t dpos = args[2];
			int32_t lpos = args[3];
			int32_t rpos = args[4];
			auto const& msgstring = get_str();
			if (msg_layout::wrap_needed(cursor_x, msgstring.hspace, msg_menu_data[MNU_CURSOR_WID], width, margins[right]))
			{
				if (!next_line())
					return true;
				if (wait_advance)
				{
					wait_advance_unprocessed_iter = true;
					return true;
				}
			}
			
			used_height_buf = zc_max(used_height_buf, msg_menu_data[MNU_CURSOR_HEI]);
			ensure_scrolled_to(cursor_y - margins[up], used_height_buf);
			menu_options[pos] = menu_choice(cursor_x, cursor_y, pos,
				upos, dpos, lpos, rpos);
			
			cursor_x += msgstring.hspace + msg_menu_data[MNU_CURSOR_WID];
			return true;
		}
		
		case MSGC_RUNMENU:
		{
			just_confirmed_menu = false;
			msg_menu_data[MNU_CHOSEN] = 0;
			msg_menu_data[MNU_CAN_CONFIRM] = 0;
			if(menu_options.size() < 1)
				return true;
			do_run_menu = true;
			return true;
		}
		
		case MSGC_GOTOMENUCHOICE:
		{
			int32_t choice = args[0];
			if(msg_menu_data[MNU_CHOSEN] == choice)
			{
				last_arg = 1;
				goto switched;
			}

			return true;
		}

		case MSGC_KILLHERO:
		{
			bool bypass_revive = args[0];
			Hero.kill(bypass_revive);
			return true;
		}
		case MSGC_ENDSTRING:
		{
			do_end_str = true;
			return true;
		}
		case MSGC_WAIT_ADVANCE:
		{
			wait_to_advance();
			return true;
		}
		case MSGC_DELAY:
		case MSGC_FORCE_DELAY:
		{
			just_confirmed_menu = false;
			int frames = args[0];
			if (frames <= 0)
			{
				Z_error("SCC: frames %d is invalid (must be >0)\n", frames);
				return true;
			}

			cur_iterator->set_post_segment_delay(frames, command.code == MSGC_FORCE_DELAY);
			return true;
		}
		case MSGC_TRIGSECRETS:
		{
			bool perm = args[0];
			trigger_secrets_for_screen(TriggerSource::SCC, scr, false);
			if(perm)
				setmapflag(scr, mSECRET);
			return true;
		}
		case MSGC_TRIG_CMB_COPYCAT:
		{
			int copy_id = args[0];
			if(copy_id == byte(copy_id))
				trig_copycat(copy_id);
			return true;
		}
		case MSGC_SETSCREENSTATE:
		{
			int32_t flag = args[0];
			if(unsigned(flag)>=mMAXIND)
			{
				Z_error("SCC 133: State %d is invalid\n", flag);
				return true;
			}
			bool state = args[1];
			if(state)
				setmapflag(scr, 1<<flag);
			else
				unsetmapflag(scr, 1<<flag, true);
			return true;
		}
		case MSGC_SETSCREENSTATER:
		{
			int32_t map = args[0];
			int32_t scrid = args[1];
			if(map < 1 || map > map_count)
			{
				Z_error("SCC 134: Map %d is invalid\n", map);
				return true;
			}
			if(unsigned(scrid)>=0x80)
			{
				Z_error("SCC 134: Screen %d is invalid\n", scrid);
				return true;
			}
			
			int32_t flag = args[2];
			if(unsigned(flag)>=mMAXIND)
			{
				Z_error("SCC 134: State %d is invalid\n", flag);
				return true;
			}
			if (!get_qr(qr_BROKEN_SCC_MAP_ARGS))
				--map; // 0-index
			bool state = args[3];
			auto mi = mapind(map, scrid);
			if(state)
				setmapflag_mi(scr, mi, 1<<flag);
			else
				unsetmapflag_mi(scr, mi, 1<<flag, true);
			return true;
		}

		case MSGC_SETANYSCREENEXSTATE:
		{
			int32_t map = args[0];
			int32_t screen = args[1];
			int32_t flag = args[2];

			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Screen ExState %d is invalid\n", flag);
				return true;
			}

			if (map < 1 || map > map_count)
			{
				Z_error("SCC: Map %d is invalid\n", map);
				return true;
			}

			if (unsigned(screen)>=0x80)
			{
				Z_error("SCC: Screen %d is invalid\n", screen);
				return true;
			}
			if (!get_qr(qr_BROKEN_SCC_MAP_ARGS))
				--map; // 0-index

			bool state = args[3];
			auto mi = mapind(map, screen);
			if(state)
				setxmapflag(mi, 1<<flag);
			else
				unsetxmapflag(mi, 1<<flag);
			return true;
		}

		case MSGC_SETSCREENEXSTATE:
		{
			int32_t flag = args[0];
			if (unsigned(flag) >= 32)
			{
				Z_error("SCC: Screen ExState %d is invalid\n", flag);
				return true;
			}

			bool state = args[1];
			if(state)
				setxmapflag(scr->screen, 1<<flag);
			else
				unsetxmapflag(scr->screen, 1<<flag);
			return true;
		}

switched:
		int32_t lev = args[last_arg];
		if(lev && get_qr(qr_SCC_GOTO_RESPECTS_CONTFLAG)
			&& (MsgStrings[lev].stringflags & STRINGFLAG_CONT))
		{
			setmsg(lev);
		}
		else
		{
			do_new(scr, lev);
			used_height_buf = text_height(msg_font);
		}
		just_confirmed_menu = false;
		putprices(false);
		return true;
	}

	return false;
}

std::string parsemsgcode2(const StringCommand& command)
{
	if (command.code == MSGC_NAME)
	{
		return game->get_name();
	}
	else if (command.code == MSGC_COUNTER)
	{
		return fmt::format("{}", game->get_counter(command.args[0]));
	}
	else if (command.code == MSGC_MAXCOUNTER)
	{
		return fmt::format("{}", game->get_maxcounter(command.args[0]));
	}
	else
	{
		parsemsgcode(command);
	}

	return "";
}

bool putmsgchar(bool play_sfx)
{
	DCHECK(cur_iterator->state == MsgStr::iterator::CHARACTER);
	DCHECK(!cur_iterator->character.empty());

	if (bottom_margin_clip())
		return false;
	
	auto const& msgstring = get_str();

	// If the current word would overflow the margins, increment cursor to the next line.
	const char* rem_word = cur_iterator->remaining_word();
	int tlength = text_length(msg_font, rem_word) + ((int32_t)strlen(rem_word)*msgstring.hspace);
	if (msg_layout::char_wrap_needed(cursor_x, tlength, width, margins[right],
		msgstring.stringflags & STRINGFLAG_WRAP, strcmp(rem_word," ")==0))
	{
		if (!next_line())
			return false;
		if (wait_advance)
		{
			wait_advance_unprocessed_iter = true;
			return false;
		}
	}
	ensure_scrolled_to(cursor_y - margins[up], used_height_buf);
	just_confirmed_menu = false;

	if (play_sfx)
		sfx(msgstring.sfx);

	// Print the character (unless it's just a space).
	if (cur_iterator->character != " ")
		textout_styled_aligned_ex(txt_bmp_buf, msg_font, cur_iterator->character.c_str(), cursor_x, cursor_y, shadow_type, ALIGN_LEFT, text_color, shadow_color, -1);

	// Move the cursor.
	cursor_x += msg_font->vtable->text_length(msg_font, cur_iterator->character.c_str());
	if (cur_iterator->character != " ")
		cursor_x += msgstring.hspace;

	return true;
}

enum msg_tick_result {msg_tick_exit, msg_tick_break, msg_tick_continue};

void msg_tick_end(bool disappear = false);
msg_tick_result msg_tick(bool play_sfx, bool burst_mode)
{
	if (cur_iterator->done())
		return msg_tick_exit;

	if (!do_run_menu)
	{
		while (cur_iterator->state == MsgStr::iterator::COMMAND && !do_run_menu)
		{
			bool one_frame_command_delay = !replay_version_check(41);
			std::string text = parsemsgcode2(cur_iterator->command);
			if (wait_advance || do_end_str)
				break;
			if (do_run_menu)
			{
				cur_iterator->set_buffer("");
				break;
			}

			if (text.empty())
			{
				cur_iterator->set_buffer("");
				// Advance the iterator to run many commands in one frame.
				if (!one_frame_command_delay)
					cur_iterator->next();
			}
			else
			{
				cur_iterator->set_buffer(text);
				cur_iterator->next();
				if (one_frame_command_delay)
					cur_iterator->set_post_segment_delay(1, false);
			}
		}

		if (cur_iterator->state == MsgStr::iterator::CHARACTER)
		{
			if (!putmsgchar(play_sfx)) return msg_tick_break;
		}
	}

	bool check_early = !burst_mode;

	if (check_early)
	{
		if (do_end_str)
		{
			msg_tick_end();
			return msg_tick_exit;
		}
	}
	
	if (do_run_menu)
	{
		if (!runMenuCursor())
			return msg_tick_break;
		scroll.target_scroll_pos = scroll.max_scroll();
		do_run_menu = false;
		if (!burst_mode && replay_version_check(0, 62))
		{
			cur_iterator->next();
			return msg_tick(play_sfx, burst_mode);
		}
	}

	if (!(wait_advance && wait_advance_unprocessed_iter))
		cur_iterator->next();

	if (!check_early)
	{
		if (do_end_str)
		{
			msg_tick_end();
			return msg_tick_exit;
		}
	}

	if (cur_iterator->done())
	{
		auto nextstr = get_str().nextstring;
		if (nextstr && MsgStrings[nextstr].stringflags & STRINGFLAG_CONT)
			setmsg(nextstr);
	}
	
	if (wait_advance)
		return msg_tick_break;

	return msg_tick_continue;
}

void msg_tick_end(bool disappear)
{
	int nextstr = 0;
	if (disappear)
		goto disappear;
	
	// Done printing the string
	if (do_end_str || (!do_run_menu && (cur_iterator->done() || bottom_margin_clip()) && (wait_advance || !linked_clk)))
	{
		if (do_end_str)
			just_confirmed_menu = false;
		
		if(!do_end_str && !wait_advance)
		{
			while (cur_iterator->state == MsgStr::iterator::COMMAND)
			{
				parsemsgcode2(cur_iterator->command);
				if (do_end_str)
					break;
				cur_iterator->next();
			}
		}
			
		// Go to next string, or make it disappear by going to string 0.
		nextstr = get_str().nextstring;
		if (nextstr || get_qr(qr_MSGDISAPPEAR) || enqueued_str)
			wait_to_advance(true);
		
		if (!nextstr && !wait_advance)
		{
			if(!get_qr(qr_MSGDISAPPEAR))
			{
disappear:
				active = false;
				Hero.finishedmsg();
			}
			
			if(repaircharge)
			{
				game->change_drupy(-(cur_screen >= 128 ? special_warp_return_scr : scr)->catchall);
				repaircharge = 0;
			}
			
			if(adjustmagic)
			{
				if(get_qr(qr_OLD_HALF_MAGIC))
				{
					if(game->get_magicdrainrate())
						game->set_magicdrainrate(1);
				}
				else if(game->get_magicdrainrate() > 1)
				{
					game->set_magicdrainrate(game->get_magicdrainrate()/2);
				}
				adjustmagic = false;
				sfx(WAV_SCALE);
				setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
			}
			
			if(learnslash)
			{
				game->set_canslash(1);
				learnslash = false;
				sfx(WAV_SCALE);
				setmapflag(scr, (cur_screen < 128 && get_qr(qr_ITEMPICKUPSETSBELOW)) ? mITEM : mSPECIALITEM);
			}
		}
	}
}

void msg_consume_spaces()
{
	while (cur_iterator->character == " ")
	{
		if (!putmsgchar(false)) break;

		// Advance the iterator.
		cur_iterator->next();

		// The "Continue From Previous" feature
		if (cur_iterator->state == MsgStr::iterator::DONE)
		{
			auto nextstr = get_str().nextstring;
			if(nextstr && MsgStrings[nextstr].stringflags & STRINGFLAG_CONT)
				setmsg(nextstr);
		}
	}
}

void tick_message()
{
	if (wait_advance && linked_clk < 1)
		linked_clk = 1;
	scroll.tick();
	if (linked_clk > 0)
	{
		if (linked_clk == 1)
		{
			if (scroll.can_scroll && !do_end_str && scroll.scroll_pos == scroll.target_scroll_pos)
			{
				bool up = getInput(btnUp, scroll.segmented ? INPUT_PRESS : 0), down = getInput(btnDown, scroll.segmented ? INPUT_PRESS : 0);
				scroll.scroll_input(up, down);
			}
			bool advance = do_end_str;
			if (!advance && just_confirmed_menu)
			{
				scroll.target_scroll_pos = scroll.max_scroll();
				advance = true;
			}
			if (!advance && scroll.at_max())
				advance = getInput(btnA) || getInput(btnB);
				
			if (advance)
			{
				if (!(do_end_str || just_confirmed_menu))
					sfx(get_str().adv_sfx);
				do_end_str = just_confirmed_menu = false;
				linked_clk = 0;
				if(wait_advance)
				{
					wait_advance = wait_advance_unprocessed_iter = false;
				}
				else
				{
					if (!scr) scr = hero_scr;
					active_str = get_str().nextstring;
					used_height_buf = text_height(msg_font);
					if (!active_str && enqueued_str)
					{
						active_str = enqueued_str;
						enqueued_str = 0;
					}
					if (!active_str)
					{
						if (scr->room!=rGRUMBLE)
							blockpath=false;
							
						dismiss();
						msg_tick_end(true);
						return;
					}
					
					do_new(scr, active_str);
					putprices(false);
				}
			}
		}
		else
		{
			--linked_clk;
		}
	}
	if (wait_advance) return; //Waiting for buttonpress
	
	if (!do_run_menu && (!active_str || !cur_iterator || cur_iterator->done() || bottom_margin_clip()))
	{
		active = false;
		cur_iterator.reset();
		return;
	}

	if (!cur_iterator)
		return;

	on_screen = true; // Now the message is onscreen (see do_new()).

	if (cur_iterator->state == MsgStr::iterator::NOT_STARTED)
	{
		cur_iterator->next();
		msg_consume_spaces();
	}

	// If the player is holding down the B button, or if speed is 0, process as many characters
	// as possible. This skips the character-by-character animation that usually renders a string
	// slowly over many frames.
	if ((getInput(btnB) && get_qr(qr_ALLOWMSGBYPASS)) || speed == 0)
	{
		while (!cur_iterator->done())
		{
			if (speed && !(getInput(btnB) && get_qr(qr_ALLOWMSGBYPASS)))
				goto breakout; // break out if message speed was changed to non-zero
			if (cur_iterator->get_post_segment_delay() && cur_iterator->get_post_segment_delay_forced())
				goto breakout; // or if a ForceDelay command was hit.

			auto tick = msg_tick(cur_iterator->character != " ", true);
			if (tick == msg_tick_break)
				break;
			if (tick == msg_tick_exit)
				return;
		}

		if (!do_run_menu)
			clk = 72;
	}
	else
	{
breakout:
		word tempspeed = speed;
		bool go_fast = get_qr(qr_ALLOWFASTMSG) && getInput(btnA);
		if (do_run_menu)
			tempspeed = 0;
		if (cur_iterator->get_post_segment_delay() && cur_iterator->peek(0).empty())
		{
			tempspeed = 0;
			cur_iterator->set_post_segment_delay_fast(go_fast);
		}
		if ((clk++ % (tempspeed+1) < tempspeed) && !go_fast)
			return;
	}

	// Process the next msg tick.
	// This will either print a single character, or process a single string command (with one
	// exception).
	if (!cur_iterator->done() && !bottom_margin_clip() && !wait_advance)
	{
		// This may run an additional tick in the case of a string menu finishing: the first
		// tick wraps up the menu, and then a second tick processes the next character or command.
		auto tick = msg_tick(true, false);
		if (tick == msg_tick_break)
		{
			msg_tick_end();
			return;
		}
		if (tick == msg_tick_exit)
			return;

		// If the next two characters are spaces, consume all upcoming spaces now.
		if (!cur_iterator->done() && cur_iterator->peek(0) == " " && cur_iterator->peek(1) == " ")
			msg_consume_spaces();
	}

	msg_tick_end();
}

void draw_icon(message_icon const& icon, int type, bool drawPassiveSubscreenSeparate)
{
	if (icon.sprite < 0 || !sprite_data_buf.get(icon.sprite).tile)
		return;
	int x = icon.x, y = icon.y;
	auto& msgstring = get_str();
	switch (icon.anchor) // x anchor
	{
		case message_anchor::screen:
			break;
		case message_anchor::screen_y_offset:
			break;
		case message_anchor::l_up:
		case message_anchor::left:
		case message_anchor::l_down:
			x += msgstring.x;
			break;
		case message_anchor::up:
		case message_anchor::center:
		case message_anchor::down:
			x += msgstring.x + msgstring.w / 2;
			break;
		case message_anchor::r_up:
		case message_anchor::right:
		case message_anchor::r_down:
			x += msgstring.x + msgstring.w;
			break;
	}
	switch (icon.anchor) // y anchor
	{
		case message_anchor::screen:
			break;
		case message_anchor::screen_y_offset:
			y += msgstring.y;
			break;
		case message_anchor::l_up:
		case message_anchor::up:
		case message_anchor::r_up:
			y += msgstring.y;
			break;
		case message_anchor::left:
		case message_anchor::center:
		case message_anchor::right:
			y += msgstring.y + msgstring.h / 2;
			break;
		case message_anchor::l_down:
		case message_anchor::down:
		case message_anchor::r_down:
			y += msgstring.y + msgstring.h;
			break;
	}
	
	y += playing_field_offset;
	
	if (drawPassiveSubscreenSeparate)
	{
		// Need to draw to two bitmaps. 'peek = true' for first call to not increment clks.
		draw_lens_hint_sprite(framebuf, x + viewport.x, y + viewport.y, wPhantom, type, up, -1, icon.sprite);
		draw_lens_hint_sprite(framebuf_no_passive_subscreen, x + viewport.x, y + viewport.y, wPhantom, type, up, -1, icon.sprite);
	}
	else
	{
		draw_lens_hint_sprite(framebuf, x + viewport.x, y + viewport.y, wPhantom, type, up, -1, icon.sprite);
	}
}
void dummy_animate_icon(message_icon const& icon, int type)
{
	if (icon.sprite < 0 || !sprite_data_buf.get(icon.sprite).tile)
		return;
	draw_lens_hint_sprite(nullptr, 0, 0, wPhantom, type, up, -1, icon.sprite);
}
void reset_hint_icons()
{
	if (replay_version_check(62))
		reset_hint_sprite(wPhantom, pMESSAGEMORE);
	reset_hint_sprite(wPhantom, pMESSAGESCROLLUP);
	reset_hint_sprite(wPhantom, pMESSAGESCROLLDOWN);
}
}

namespace msgstr
{
	
mapscr* scr;
word active_str = 0;
bool on_screen = false, active = false;

byte layer = 6;
bool nofreeze = false;

word clk = 0, linked_clk = 0;

bool allocate_bmps()
{
	txt_bmp_height = 512+16;
	txt_bmp_buf = create_bitmap_ex(8, txt_bmp_width, txt_bmp_height);
	bg_bmp_buf = create_bitmap_ex(8, 512+16, 512+16);
	portrait_bmp_buf = create_bitmap_ex(8, 256, 256);
	menu_bmp_buf = create_bitmap_ex(8, txt_bmp_width, txt_bmp_height);
	
	txt_display_buf = create_bitmap_ex(8,256, 224);
	bg_display_buf = create_bitmap_ex(8,256, 224);
	portrait_display_buf = create_bitmap_ex(8, 256, 256);
	
	return txt_bmp_buf && bg_bmp_buf && portrait_bmp_buf && menu_bmp_buf
		&& txt_display_buf && bg_display_buf && portrait_display_buf;
}
void cleanup_bmps()
{
	destroy_bitmap(txt_bmp_buf);
	destroy_bitmap(bg_bmp_buf);
	destroy_bitmap(portrait_bmp_buf);
	destroy_bitmap(menu_bmp_buf);
	set_clip_state(txt_display_buf, 1);
	destroy_bitmap(txt_display_buf);
	set_clip_state(bg_display_buf, 1);
	destroy_bitmap(bg_display_buf);
	set_clip_state(portrait_display_buf, 1);
	destroy_bitmap(portrait_display_buf);
}

void clear_display_bmps()
{
	clear_bitmap(txt_display_buf);
	set_clip_state(txt_display_buf, 1);
	clear_bitmap(bg_display_buf);
	set_clip_state(bg_display_buf, 1);
	clear_bitmap(portrait_display_buf);
	set_clip_state(portrait_display_buf, 1);
}
void clear_bmps()
{
	clear_bitmap(txt_bmp_buf);
	clear_bitmap(bg_bmp_buf);
	clear_bitmap(portrait_bmp_buf);
	clear_bitmap(menu_bmp_buf);
	clear_display_bmps();
}

void blit_str(BITMAP* dest, int sx, int sy, int dx, int dy, int w, int h)
{
	blit_bg(dest, sx, sy, dx, dy, w, h);
	blit_prt(dest, sx, sy, dx, dy, w, h);
	blit_fg(dest, sx, sy, dx, dy, w, h);
}

static const int16_t old_margins[4] = {8, 0, 8, -8};
void do_new(mapscr* new_scr, int str)
{
	if(on_screen || active)
		dismiss();
		
	linked_clk = 0;
	active = true;
	// Don't set on_screen - not onscreen just yet
	setmsg(str);
	scr = new_scr;
	text_color = QMisc.colors.msgtext;
	speed = zinit.msg_speed;
	auto const& msgstring = get_str();
	layer = msgstring.drawlayer;
	nofreeze = msgstring.stringflags & STRINGFLAG_NOFREEZE;
	scroll.can_scroll = !get_qr(qr_STRINGS_DONT_SCROLL) && !get_qr(qr_OLD_STRING_EDITOR_MARGINS);
	scroll.segmented = scroll.can_scroll && get_qr(qr_STRING_SEGMENTED_SCROLL);
	scroll.active_scrolling = false;
	used_height_buf = text_height(msg_font);
	scroll.scroll_pos = scroll.target_scroll_pos = scroll.max_visible_pos = 0;
	scroll.passive_speed = msgstring.passive_scroll_speed;
	scroll.active_speed = msgstring.active_scroll_speed;
	reset_hint_icons();
	
	clear_bmps();
	
	clk = 0;
	width = msgstring.w;
	height = msgstring.h;
	xpos = msgstring.x;
	ypos = msgstring.y;
	portrait = msgstring.portrait;
	shadow_type = msgstring.shadow_type;
	shadow_color = msgstring.shadow_color;
	
	draw_bg(msgstring);
	draw_prt();
	
	int16_t const* copy_from = get_qr(qr_OLD_STRING_EDITOR_MARGINS) ? old_margins : msgstring.margins;
	for(auto q = 0; q < 4; ++q)
		margins[q] = copy_from[q];
	cursor_x = margins[left];
	cursor_y = margins[up];
	scroll.set_body_height(height, margins[up], margins[down]);
}

// Called to make a message disappear
void dismiss()
{
	linked_clk = 0;
	active_str = clk = 0;
	cur_iterator.reset();
	xpos = ypos = 0;
	//width = height = 0;
	cursor_x = cursor_y = 0;
	portrait.clear();
	on_screen = active = false;
	nofreeze = false;
	msg_font = get_zc_font(font_zfont);
	clear_display_bmps();
	do_end_str = false;
	wait_advance = wait_advance_unprocessed_iter = false;
	just_confirmed_menu = false;
	do_run_menu = false;
	menu_options.clear();
	memset(msg_menu_data, 0, sizeof(msg_menu_data));
	scr = nullptr;
	layer = 6;
	shadow_type = 0;
	shadow_color = 0;
	enqueued_str = 0;
	speed = zinit.msg_speed;
	for(auto q = 0; q < 4; ++q)
		margins[q] = old_margins[q];
	scroll.clear();
	reset_hint_icons();
}
void init_string_vars()
{
	dismiss();
	if (txt_bmp_height != 512+16)
	{
		txt_bmp_height = 512 + 16;
		destroy_bitmap(txt_bmp_buf);
		txt_bmp_buf = create_bitmap_ex(8, txt_bmp_width, txt_bmp_height);
		destroy_bitmap(menu_bmp_buf);
		menu_bmp_buf = create_bitmap_ex(8, txt_bmp_width, txt_bmp_height);
	}
}

void draw_icons(bool drawPassiveSubscreenSeparate)
{
	if (!active_str)
		return;
	if (linked_clk != 1 || do_end_str)
		return;
	auto& msgstring = get_str();
	
	bool draw_more = false, draw_scr_up = false, draw_scr_down = false;
	
	if (!scroll.can_scroll || scroll.at_max())
		draw_more = true;
	if (scroll.can_scroll && scroll.active_speed)
	{
		draw_scr_up = scroll.cur_scroll();
		draw_scr_down = scroll.cur_scroll() < scroll.max_scroll();
	}
	
	if (draw_more)
		draw_icon(msgstring.icon_more, pMESSAGEMORE, drawPassiveSubscreenSeparate);
	else
		dummy_animate_icon(msgstring.icon_more, pMESSAGEMORE);
	
	if (draw_scr_up)
		draw_icon(msgstring.icon_scroll_up, pMESSAGESCROLLUP, drawPassiveSubscreenSeparate);
	else
		dummy_animate_icon(msgstring.icon_scroll_up, pMESSAGESCROLLUP);
	
	if (draw_scr_down)
		draw_icon(msgstring.icon_scroll_down, pMESSAGESCROLLDOWN, drawPassiveSubscreenSeparate);
	else
		dummy_animate_icon(msgstring.icon_scroll_down, pMESSAGESCROLLDOWN);
}

MsgStr const& get_str()
{
	return MsgStrings[active_str];
}

void queue_string(int str)
{
	enqueued_str = str;
}

void run()
{
	tick_message();
	update();
}

void run_intro()
{
	if (!get_qr(qr_OLD_DMAP_INTRO_STRINGS))
		return;

	if(!stricmp("                                                                        ",
		DMaps[cur_dmap].intro))
	{
		introclk=intropos=72;
		return;
	}
	
	if(getInput(btnB) && get_qr(qr_ALLOWMSGBYPASS))
	{
		//finish writing out the string
		for(; intropos<72; ++intropos)
		{
			textprintf_ex(txt_display_buf,get_zc_font(font_zfont),((intropos%24)<<3)+32,((intropos/24)<<3)+40,QMisc.colors.msgtext,-1,
						  "%c",DMaps[cur_dmap].intro[intropos]);
		}
	}
	
	if(intropos>=72)
	{
		if(dmapmsgclk >= 51)
			dmapmsgclk=50;
			
		return;
	}
	
	if(((introclk++) % 6 < 5) && (!getInput(btnA) || !get_qr(qr_ALLOWFASTMSG)))
		return;
		
	dmapmsgclk=51;
	
	if(intropos == 0)
	{
		while(DMaps[cur_dmap].intro[intropos]==' ')
			++intropos;
	}
	
	sfx(WAV_MSG); //actual message display
	
	
	//using the clip value to indicate the bitmap is "dirty"
	//rather than add yet another global variable
	set_clip_state(txt_display_buf, 0);
	textprintf_ex(txt_display_buf,get_zc_font(font_zfont),((intropos%24)<<3)+32,((intropos/24)<<3)+40,QMisc.colors.msgtext,-1,
				  "%c",DMaps[cur_dmap].intro[intropos]);
				  
	++intropos;
	
	if(DMaps[cur_dmap].intro[intropos]==' ' && DMaps[cur_dmap].intro[intropos+1]==' ')
		while(DMaps[cur_dmap].intro[intropos]==' ')
			++intropos;
}

}

