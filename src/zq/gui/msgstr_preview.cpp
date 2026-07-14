#include "msgstr_preview.h"
#include "components/scc/scc.h"
#include "zq/zquest.h"
#include "core/qrs.h"
#include "core/msgstr.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "subscr.h"
#include "tiles.h"
#include <utility>

extern char namebuf[9];

void init_msgstr(MsgStr *str);
void strip_trailing_spaces(std::string& str);
int32_t msg_code_operands(byte cc);

namespace GUI
{

int32_t d_newmsg_preview_proc(int32_t msg, DIALOG *d, int32_t c)
{
	switch (msg)
	{
		case MSG_WANTWHEEL:
			return 1;
		case MSG_DRAW:
		case MSG_VSYNC:
		case MSG_WHEEL:
		case MSG_CLICK:
			break;
		default:
			return D_O_K;
	}
	
	GUI::MsgPreview* prv=(GUI::MsgPreview*)d->dp3;
	MsgStr const* s = prv->getData();
	DCHECK(s);
	if (!s) return D_O_K;
	auto ret = D_O_K;
	int max = prv->max_scroll_pos();
	if (prv->is_tmp_scrolling)
	{
		if (prv->segmented_scroll)
			prv->target_scroll_pos = prv->scroll_pos = prv->tmp_scroll * prv->body_height;
		else
			prv->target_scroll_pos = prv->scroll_pos = prv->tmp_scroll;
	}
	prv->scroll_pos = vbound(prv->scroll_pos, 0, max);
	prv->target_scroll_pos = vbound(prv->target_scroll_pos, 0, max);
	switch (msg)
	{
		case MSG_WHEEL:
		{
			if (!c)
				break;
			if (!prv->can_scroll)
				break;
			c = -c;
			if (prv->segmented_scroll)
			{
				if (prv->scroll_pos != prv->target_scroll_pos)
					break; // only segmented scroll from still
				if (prv->target_scroll_pos % prv->body_height)
					prv->target_scroll_pos -= (prv->target_scroll_pos % prv->body_height);
				if (c > 0 && prv->target_scroll_pos < max)
					prv->target_scroll_pos += prv->body_height;
				else if (c < 0 && prv->target_scroll_pos > 0)
					prv->target_scroll_pos -= prv->body_height;
			}
			else
			{
				prv->target_scroll_pos = vbound(prv->target_scroll_pos + 4*c, 0, max);
			}
			break;
		}
		case MSG_VSYNC:
		{
			if (prv->scroll_pos == prv->target_scroll_pos)
				break;
			int spd = s->passive_scroll_speed;
			if (!spd)
				prv->scroll_pos = prv->target_scroll_pos;
			else if (prv->scroll_pos < prv->target_scroll_pos)
			{
				prv->scroll_pos += spd;
				if (prv->scroll_pos > prv->target_scroll_pos)
					prv->scroll_pos = prv->target_scroll_pos;
			}
			else if (prv->scroll_pos > prv->target_scroll_pos)
			{
				prv->scroll_pos -= spd;
				if (prv->scroll_pos < prv->target_scroll_pos)
					prv->scroll_pos = prv->target_scroll_pos;
			}
			ret = D_REDRAWME;
			break;
		}
		case MSG_DRAW:
		{
			prv->update_string();
			max = prv->max_scroll_pos();
			
			const int prev_w = 256*2, prev_h = 168*2;
			rectfill(screen, d->x, d->y, d->x+prev_w, d->y+prev_h, 0);
			jwin_draw_frame(screen, d->x, d->y, prev_w, prev_h, FR_DEEP);
			if (prv->can_scroll && max > 0)
			{
				int cur = prv->scroll_pos;
				int h = prv->body_height;
				if (prv->segmented_scroll && prv->is_tmp_scrolling)
				{
					cur /= prv->body_height;
					max /= prv->body_height;
					h = 1;
				}
				_jwin_draw_scrollable_frame(d, max+h, cur, h, 0);
			}
			BITMAP* buf = create_bitmap_ex(8, 256, 168);
			clear_bitmap(buf);
			auto scrollpos = vbound(prv->scroll_pos, 0, prv->max_scroll_pos());
			masked_blit(prv->bg_buf, buf, 0, 0, 0, 0, 256, 168);
			int mu = prv->msg_margins[up], md = prv->msg_margins[down], ml = prv->msg_margins[left], mr = prv->msg_margins[right];
			masked_blit(prv->fg_buf, buf, ml, mu+scrollpos, ml+s->x, mu+s->y, s->w-ml-mr, s->h-mu-md);
			
			stretch_blit(buf,screen,0,0,256,168,d->x+2,d->y+2,256*2,168*2);
			
			destroy_bitmap(buf);
			break;
		}
		case MSG_CLICK:
		{
			if (prv->can_scroll && max > 0 && (gui_mouse_x() > d->x + d->w - 18) && !prv->is_tmp_scrolling)
			{
				prv->is_tmp_scrolling = true;
				prv->tmp_scroll = prv->scroll_pos;
				int h = prv->body_height;
				if (prv->segmented_scroll)
				{
					prv->tmp_scroll /= prv->body_height;
					max /= prv->body_height;
					h = 1;
				}
				if (_handle_jwin_scrollable_scroll_click(d, max+h, &prv->tmp_scroll, h))
				{
					if (prv->segmented_scroll)
						prv->tmp_scroll *= prv->body_height;
					prv->target_scroll_pos = prv->scroll_pos = prv->tmp_scroll;
				}
				prv->is_tmp_scrolling = false;
			}
			break;
		}
	}
	
	return ret;
}

MsgPreview::MsgPreview(): str_data(nullptr), index(-1),
	scroll_pos(0), body_height(1), max_visible_pos(0), target_scroll_pos(0),
	tmp_scroll(0), is_tmp_scrolling(false)
{
	can_scroll = !get_qr(qr_STRINGS_DONT_SCROLL) && !get_qr(qr_OLD_STRING_EDITOR_MARGINS);
	setPreferredWidth(256_px*2+4_px + (can_scroll ? 18_px : 0_px));
	setPreferredHeight(168_px*2+4_px);
	bg_buf = create_bitmap_ex(8, 256, 168);
	fg_buf = create_bitmap_ex(8, 256, 512);
	clear_bitmap(bg_buf);
	clear_bitmap(fg_buf);
	fg_bmp_height = 512;
	segmented_scroll = can_scroll && get_qr(qr_STRING_SEGMENTED_SCROLL);
	memset(msg_margins, 0, sizeof(msg_margins));
}
MsgPreview::~MsgPreview()
{
	destroy_bitmap(bg_buf);
	destroy_bitmap(fg_buf);
}

void MsgPreview::update_string()
{
	MsgStr const* str = str_data;
	
	max_visible_pos = 0;
	int32_t w = str->w; //8-256
	int32_t h = str->h; //8-168
	int32_t nextstring = str->nextstring;
	
	int16_t old_margins[4] = {8,0,8,-8};
	int16_t const* copy_from = get_qr(qr_OLD_STRING_EDITOR_MARGINS) ? old_margins : str->margins;
	for(auto q = 0; q < 4; ++q)
		msg_margins[q] = copy_from[q];
	
	int32_t cursor_x = msg_margins[left];
	int32_t cursor_y = msg_margins[up];
	
	int32_t msgcolour=QMisc.colors.msgtext;
	int32_t shdtype=str->shadow_type;
	int32_t shdcolor=str->shadow_color;
	int32_t msgtile = str->tile;
	int32_t msgcset = str->cset;
	
	int workfont_id = str->font;
	FONT *workfont = get_zc_font(workfont_id);
	int32_t ssc_tile_hei = text_height(workfont);
	body_height = zc_max(1, h - msg_margins[up] - msg_margins[down]);
	
	auto bottom_margin_clip = [&]()
	{
		if (!get_qr(qr_STRINGS_DONT_SCROLL) && !get_qr(qr_OLD_STRING_EDITOR_MARGINS))
			return false; // don't clip in scrolling mode
		return !get_qr(qr_OLD_STRING_EDITOR_MARGINS)
			&& cursor_y >= (h + (get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)?16:0) - msg_margins[down]);
	};
	auto update_max_scroll = [&](int pos)
	{
		if (segmented_scroll && (pos % body_height))
		{
			// round up to next multiple of body_height
			pos += body_height - (pos % body_height);
		}
		
		if (pos > max_visible_pos)
			max_visible_pos = pos;
		
		int new_height = max_visible_pos + msg_margins[down];
		if (fg_bmp_height >= new_height)
			return;
		
		new_height += 64;
		BITMAP* tmp = fg_buf;
		fg_buf = create_bitmap_ex(8, 256, new_height);
		clear_bitmap(fg_buf);
		blit(tmp, fg_buf, 0, 0, 0, 0, 256, fg_bmp_height);
		destroy_bitmap(tmp);
		fg_bmp_height = new_height;
	};
	
	clear_bitmap(bg_buf);
	clear_bitmap(fg_buf);
	
	if(msgtile)
	{
		if(str->stringflags & STRINGFLAG_FULLTILE)
		{
			draw_block_flip(bg_buf,str->x,str->y,msgtile,msgcset,
				(int32_t)ceil(w/16.0),(int32_t)ceil(h/16.0),0,false,false);
		}
		else
		{
			int32_t add = (get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 2 : 0);
			frame2x2(bg_buf,str->x,str->y,msgtile,msgcset,(w/8)+add,(h/8)+add,0,0,0);
		}
	}
	auto const& portrait = str->portrait;
	if (portrait.tile > 0 && portrait.tw > 0 && portrait.th > 0)
	{
		draw_block_flip(bg_buf, portrait.x, portrait.y, portrait.tile, portrait.cset,
			portrait.tw, portrait.th, 0, true, false);
	}
		
	
	int32_t _menu_tl = 0;
	int32_t _menu_cs = 0;
	int32_t _menu_t_wid = 0;
	int32_t _menu_t_hei = 0;
	int32_t _menu_fl = 0;
	std::map<int32_t, bool> visited;
	if(index > -1)
		visited[index] = true;
	
	for(;;)
	{
		auto it = str->create_iterator();
		while (!it.next())
		{
			if (bottom_margin_clip())
				break;

			int hjump = 0;
			int tlength = 0;

			const char* rem_word = "";
			if (it.state == MsgStr::iterator::CHARACTER)
			{
				rem_word = it.remaining_word();
				tlength = text_length(workfont, rem_word);
				hjump = strlen(rem_word) * str->hspace;
			}
			else
			{
				auto& args = it.command.args;
				switch (it.command.code)
				{
					case MSGC_NEWLINE:
					{
						if(cursor_x>msg_margins[left] || (cursor_y<=msg_margins[up] && cursor_x<=msg_margins[left])) // If the newline's already at the end of a line, ignore it
						{
							int32_t thei = ssc_tile_hei;
							ssc_tile_hei = text_height(workfont);
							cursor_y += thei + str->vspace;
							cursor_x=msg_margins[left];
						}
						update_max_scroll(cursor_y - msg_margins[up] + ssc_tile_hei);
						break;
					}
					
					case MSGC_COLOUR:
					{
						int32_t cset = args[0];
						msgcolour = CSET(cset)+args[1];
						break;
					}
					case MSGC_FONT:
					{
						workfont_id = args[0];
						workfont = get_zc_font(workfont_id);
						int wf_hei = text_height(workfont);
						if(wf_hei > ssc_tile_hei)
							ssc_tile_hei = wf_hei;
						break;
					}
					case MSGC_SHDCOLOR:
					{
						int32_t cset = args[0];
						shdcolor = CSET(cset)+args[1];
						break;
					}
					case MSGC_SHDTYPE:
					{
						shdtype = args[0];
						break;
					}

					case MSGC_NAME:
					{
						it.set_buffer(namebuf);
						break;
					}

					case MSGC_COUNTER:
					{
						it.set_buffer("99");
						break;
					}

					case MSGC_MAXCOUNTER:
					{
						it.set_buffer("100");
						break;
					}

					case MSGC_DRAWTILE:
					{
						int32_t tl = args[0];
						int32_t cs = args[1];
						int32_t t_wid = args[2];
						int32_t t_hei = args[3];
						int32_t fl = args[4];
						
						if(cursor_x+str->hspace + t_wid > w-msg_margins[right])
						{
							int32_t thei = ssc_tile_hei;
							ssc_tile_hei = text_height(workfont);
							cursor_y += thei + str->vspace;
							if(bottom_margin_clip()) break;
							cursor_x=msg_margins[left];
						}
						
						if(t_hei > ssc_tile_hei)
							ssc_tile_hei = t_hei;
						update_max_scroll(cursor_y - msg_margins[up] + ssc_tile_hei);
						overtileblock16(fg_buf, tl, cursor_x, cursor_y, (int32_t)ceil(t_wid/16.0), (int32_t)ceil(t_hei/16.0), cs, fl);
						cursor_x += str->hspace + t_wid;
						break;
					}
					
					case MSGC_SETUPMENU:
					{
						_menu_tl = args[0];
						_menu_cs = args[1];
						_menu_t_wid = args[2];
						_menu_t_hei = args[3];
						_menu_fl = args[4];
						break;
					}
					case MSGC_MENUCHOICE:
					{
						if(cursor_x+str->hspace + _menu_t_wid > w-msg_margins[right])
						{
							int32_t thei = ssc_tile_hei;
							ssc_tile_hei = text_height(workfont);
							cursor_y += thei + str->vspace;
							if(bottom_margin_clip()) break;
							cursor_x=msg_margins[left];
						}
						
						if(_menu_t_hei > ssc_tile_hei)
							ssc_tile_hei = _menu_t_hei;
						update_max_scroll(cursor_y - msg_margins[up] + ssc_tile_hei);
						overtileblock16(fg_buf, _menu_tl, cursor_x, cursor_y, (int32_t)ceil(_menu_t_wid/16.0), (int32_t)ceil(_menu_t_hei/16.0), _menu_cs, _menu_fl);
						cursor_x += str->hspace + _menu_t_wid;
						break;
					}
				}

				// Legacy decision. Could be fixed behind a compat QR someday.
				hjump = str->hspace;
				if (!hjump)
					continue;
			}

			if(cursor_x+tlength+hjump > (w-msg_margins[right]) 
			   && ((cursor_x > (w-msg_margins[right]) || !(str->stringflags & STRINGFLAG_WRAP))
					? 1 : strcmp(rem_word," ")!=0))
			{
				int32_t thei = ssc_tile_hei;
				ssc_tile_hei = text_height(workfont);
				cursor_y += thei + str->vspace;
				if(bottom_margin_clip()) break;
				cursor_x=msg_margins[left];
			}
			update_max_scroll(cursor_y - msg_margins[up] + ssc_tile_hei);

			// Print the character (unless it's just a space).
			if (it.character != " ")
				textout_styled_aligned_ex(fg_buf, workfont, it.character.c_str(), cursor_x, cursor_y, shdtype, ALIGN_LEFT, msgcolour, shdcolor, -1);

			cursor_x += workfont->vtable->text_length(workfont, it.character.c_str());
			if (it.character != " ")
				cursor_x += str->hspace;
		}

		if(nextstring && !visited[nextstring]
			&& MsgStrings[nextstring].stringflags & STRINGFLAG_CONT)
		{
			str = &MsgStrings[nextstring];
			it = str->create_iterator();
			workfont = get_zc_font(str->font);
			
			visited[nextstring] = true;
			nextstring = str->nextstring;
		}
		else
		{
			break;
		}
	}
	scroll_pos = vbound(scroll_pos, 0, max_scroll_pos());
	target_scroll_pos = vbound(target_scroll_pos, 0, max_scroll_pos());
}

int MsgPreview::max_scroll_pos() const
{
	if (!can_scroll)
		return 0;
	if (max_visible_pos < body_height)
		return 0;
	return zc_max(0, max_visible_pos - body_height);
}
void MsgPreview::setData(MsgStr const* data)
{
	str_data = data;
}

void MsgPreview::setIndex(int32_t ind)
{
	index = ind;
}

void MsgPreview::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void MsgPreview::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void MsgPreview::arrange(int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
	Widget::arrange(cx, cy, cw, ch);
}

void MsgPreview::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<d_newmsg_preview_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		0, 0, // d1, d2
		nullptr, nullptr, this // dp, dp2, dp3
	});
}

}
