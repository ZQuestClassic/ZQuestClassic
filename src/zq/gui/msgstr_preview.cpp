#include "msgstr_preview.h"
#include "zq/zquest.h"
#include "base/qrs.h"
#include "base/msgstr.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "subscr.h"
#include "tiles.h"
#include <utility>

extern char namebuf[9];

void init_msgstr(MsgStr *str);
std::string parse_to_legacy_msg_str_encoding(std::string const& s);
void strip_trailing_spaces(std::string& str);
int32_t msg_code_operands(byte cc);

bool bottom_margin_clip(int32_t cursor_y, int32_t msg_h, int32_t bottom_margin)
{
	return !get_qr(qr_OLD_STRING_EDITOR_MARGINS)
		&& cursor_y >= (msg_h + (get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)?16:0) - bottom_margin);
}
#define BOTTOM_MARGIN_CLIP() bottom_margin_clip(cursor_y, h, msg_margins[down])

void put_msg_str(char const* s, int32_t x, int32_t y, MsgStr const* str, int32_t index = -1)
{
	int32_t w = str->w; //8-256
	int32_t h = str->h; //8-168
	int32_t nextstring = str->nextstring;
	int16_t msg_margins[4];
	
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
	
	BITMAP *buf = create_bitmap_ex(8,256,168);
	if(!buf) return; //sanity, I guess?
	clear_bitmap(buf);
	
	if(msgtile)
	{
		if(str->stringflags & STRINGFLAG_FULLTILE)
		{
			draw_block_flip(buf,0,0,msgtile,msgcset,
				(int32_t)ceil(w/16.0),(int32_t)ceil(h/16.0),0,false,false);
		}
		else
		{
			int32_t add = (get_qr(qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 2 : 0);
			frame2x2(buf,0,0,msgtile,msgcset,(w/8)+add,(h/8)+add,0,0,0);
		}
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
			if (BOTTOM_MARGIN_CLIP())
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
							if(BOTTOM_MARGIN_CLIP()) break;
							cursor_x=msg_margins[left];
						}
						
						overtileblock16(buf, tl, cursor_x, cursor_y, (int32_t)ceil(t_wid/16.0), (int32_t)ceil(t_hei/16.0), cs, fl);
						if(t_hei > ssc_tile_hei)
							ssc_tile_hei = t_hei;
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
							if(BOTTOM_MARGIN_CLIP()) break;
							cursor_x=msg_margins[left];
						}
						
						overtileblock16(buf, _menu_tl, cursor_x, cursor_y, (int32_t)ceil(_menu_t_wid/16.0), (int32_t)ceil(_menu_t_hei/16.0), _menu_cs, _menu_fl);
						if(_menu_t_hei > ssc_tile_hei)
							ssc_tile_hei = _menu_t_hei;
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
				if(BOTTOM_MARGIN_CLIP()) break;
				cursor_x=msg_margins[left];
			}

			// Print the character (unless it's just a space).
			if (it.character != " ")
				textout_styled_aligned_ex(buf, workfont, it.character.c_str(), cursor_x, cursor_y, shdtype, sstaLEFT, msgcolour, shdcolor, -1);

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
	
	stretch_blit(buf,screen,0,0,256,168,x,y,256*2,168*2);
	destroy_bitmap(buf);
}

int32_t d_newmsg_preview_proc(int32_t msg,DIALOG *d,int32_t)
{
	if(msg != MSG_DRAW) return D_O_K;
	static MsgStr nulled_str;
	static bool init_nulled_str = false;
	if(!init_nulled_str)
	{
		init_msgstr(&nulled_str);
		init_nulled_str = true;
	}
	char const* s=(char*)d->dp;
	GUI::MsgPreview* prv=(GUI::MsgPreview*)d->dp3;
	MsgStr const* str = prv->getData();
	if(!str) str = &nulled_str;
	
	rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h, 0);
	jwin_draw_frame(screen, d->x, d->y, d->w, d->h, FR_DEEP);
	put_msg_str(s,d->x+2,d->y+2, str, prv->getIndex());
	
	return D_O_K;
}

namespace GUI
{

MsgPreview::MsgPreview(): text(), str_data(NULL), index(-1)
{
	setPreferredWidth(256_px*2+4_px);
	setPreferredHeight(168_px*2+4_px);
}

void MsgPreview::setText(std::string newText)
{
	text=newText;
	if(alDialog)
	{
		alDialog->dp = text.data();
	}
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
		text.data(), nullptr, this // dp, dp2, dp3
	});
}

}
