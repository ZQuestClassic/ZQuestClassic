#include "msgstr_preview.h"
#include "zq/zquest.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "jwin.h"
#include "subscr.h"
#include "tiles.h"
#include <utility>

extern char namebuf[9];
extern byte quest_rules[QUESTRULES_NEW_SIZE];

void init_msgstr(MsgStr *str);
std::string parse_msg_str(std::string const& s);
void strip_trailing_spaces(std::string& str);
word grab_next_argument(std::string const& s2, uint32_t* i);
int32_t msg_code_operands(byte cc);

bool bottom_margin_clip(int32_t cursor_y, int32_t msg_h, int32_t bottom_margin)
{
	return !get_bit(quest_rules, qr_OLD_STRING_EDITOR_MARGINS)
		&& cursor_y >= (msg_h + (get_bit(quest_rules,qr_STRING_FRAME_OLD_WIDTH_HEIGHT)?16:0) - bottom_margin);
}
#define BOTTOM_MARGIN_CLIP() bottom_margin_clip(cursor_y, h, msg_margins[down])

void put_msg_str(char const* s, int32_t x, int32_t y, MsgStr const* str, int32_t index = -1)
{
	int32_t w = str->w; //8-256
	int32_t h = str->h; //8-168
	int32_t nextstring = str->nextstring;
	int16_t msg_margins[4];
	
	int16_t old_margins[4] = {8,0,8,-8};
	int16_t const* copy_from = get_bit(quest_rules,qr_OLD_STRING_EDITOR_MARGINS) ? old_margins : str->margins;
	for(auto q = 0; q < 4; ++q)
		msg_margins[q] = copy_from[q];
	
	int32_t cursor_x = msg_margins[left];
	int32_t cursor_y = msg_margins[up];
	
	uint32_t i=0;
	int32_t msgcolour=misc.colors.msgtext;
	int32_t shdtype=str->shadow_type;
	int32_t shdcolor=str->shadow_color;
	int32_t msgtile = str->tile;
	int32_t msgcset = str->cset;
	
	int workfont_id = str->font;
	FONT *workfont = get_zc_font(workfont_id);
	int32_t ssc_tile_hei = text_height(workfont);
	
	std::string s2 = parse_msg_str(s);
	strip_trailing_spaces(s2);
	
	BITMAP *buf = create_bitmap_ex(8,256,168);
	if(!buf) return; //sanity, I guess?
	clear_bitmap(buf);
	
	bool done = false;
	
	if(msgtile)
	{
		if(str->stringflags & STRINGFLAG_FULLTILE)
		{
			draw_block_flip(buf,0,0,msgtile,msgcset,
				(int32_t)ceil(w/16.0),(int32_t)ceil(h/16.0),0,false,false);
		}
		else
		{
			int32_t add = (get_bit(quest_rules,qr_STRING_FRAME_OLD_WIDTH_HEIGHT)!=0 ? 2 : 0);
			frame2x2(buf,&misc,0,0,msgtile,msgcset,(w/8)+add,(h/8)+add,0,0,0);
		}
	}
		
	bool space=true;
	int32_t tlength=0;
	
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
		i=0;
		while(i < s2.size() && !done)
		{
			std::string s3; // Stores a complete word
			int32_t j;
			int32_t s3length = 1;
			int32_t hjump=0;
			
			if(str->stringflags & STRINGFLAG_WRAP)
			{
				if(space)
				{
					// Control codes and spaces are like one-character words
					if((s2[i]) == ' ' || (s2[i]) < 32 || (s2[i]) > 126)
					{
						s3 = s2[i];
						hjump = str->hspace;
						i++;
					}
					else space = false;
				}
				
				if(!space)
				{
					// Complete words finish at spaces or control codes.
					for(j=i; (s2[j]) != ' ' && (s2[j]) >= 32 && (s2[j]) <= 126 && (s2[j]); j++)
					{
						s3 += s2[j];
						hjump += str->hspace;
						
						if(s3[j-i]-1 == MSGC_NEWLINE)
						{
							j++;
							break;
						}
					}
					
					space = true;
					i=j;
				}
			}
			else
			{
				space=false;
				s3 = s2[i];
				
				if(s3[0] >= 32 && s3[0] <= 126) hjump = str->hspace;
				
				i++;
			}
			
			tlength = text_length(workfont, s3.c_str());
			
			if(cursor_x+tlength+hjump > (w-msg_margins[right]) 
			   && ((cursor_x > (w-msg_margins[right]) || !(str->stringflags & STRINGFLAG_WRAP))
					? 1 : (s3 != " ")))
			{
				int32_t thei = ssc_tile_hei;
				ssc_tile_hei = text_height(workfont);
				cursor_y += thei + str->vspace;
				if(BOTTOM_MARGIN_CLIP()) break;
				cursor_x=msg_margins[left];
			}
			
			// Evaluate what control code the character is, and skip over the CC's arguments by incrementing i (NOT k).
			// Interpret the control codes which affect text display (currently just MSGC_COLOR). -L
			for(uint32_t k=0; k < s3.size() && !done; k++)
			{
				switch(byte(s3[k]-1))
				{
					case MSGC_NEWLINE:
					{
						if(cursor_x>msg_margins[left] || (cursor_y<=msg_margins[up] && cursor_x<=msg_margins[left])) // If the newline's already at the end of a line, ignore it
						{
							int32_t thei = ssc_tile_hei;
							ssc_tile_hei = text_height(workfont);
							cursor_y += thei + str->vspace;
							if(BOTTOM_MARGIN_CLIP()) done = true;
							cursor_x=msg_margins[left];
						}
						
						//No i++ here - s3 terminates at newlines.
						break;
					}
					
					case MSGC_COLOUR:
					{
						int32_t cset = grab_next_argument(s2, &i);
						msgcolour = CSET(cset)+grab_next_argument(s2, &i);
						break;
					}
					case MSGC_FONT:
					{
						workfont_id = grab_next_argument(s2, &i);
						workfont = get_zc_font(workfont_id);
						int wf_hei = text_height(workfont);
						if(wf_hei > ssc_tile_hei)
							ssc_tile_hei = wf_hei;
						break;
					}
					case MSGC_RUN_FRZ_GENSCR:
					{
						int scr_id = grab_next_argument(s2, &i);
						bool force_redraw = grab_next_argument(s2, &i)!=0;
						break;
					}
					case MSGC_SHDCOLOR:
					{
						int32_t cset = grab_next_argument(s2, &i);
						shdcolor = CSET(cset)+grab_next_argument(s2, &i);
						break;
					}
					case MSGC_SHDTYPE:
					{
						shdtype = grab_next_argument(s2, &i);
						break;
					}
					
					case MSGC_NAME:
					{
						char *namestr = namebuf;
						char wrapstr[9] = {0};
						for(int32_t q = 0; namestr[q]; ++q)
						{
							if(str->stringflags & STRINGFLAG_WRAP)
							{
								strcpy(wrapstr, namestr+q);
							}
							else
							{
								wrapstr[0] = namestr[q];
							}
							
							tlength = text_length(workfont, wrapstr);
							
							if(int32_t(cursor_x+tlength+(str->hspace*strlen(namestr))) > int32_t(w-msg_margins[right]))
							{
								int32_t thei = ssc_tile_hei;
								ssc_tile_hei = text_height(workfont);
								cursor_y += thei + str->vspace;
								if(BOTTOM_MARGIN_CLIP()) break;
								cursor_x=msg_margins[left];
							}
							
							char cbuf[2] = {0};
							
							sprintf(cbuf,"%c",namestr[q]);
							
							textout_styled_aligned_ex(buf,workfont,cbuf,cursor_x,cursor_y,shdtype,sstaLEFT,msgcolour,shdcolor,-1);
							
							cursor_x += workfont->vtable->char_length(workfont, namestr[q]);
							cursor_x += str->hspace;
						}
						break;
					}
					
					case MSGC_DRAWTILE:
					{
						int32_t tl = grab_next_argument(s2, &i);
						int32_t cs = grab_next_argument(s2, &i);
						int32_t t_wid = grab_next_argument(s2, &i);
						int32_t t_hei = grab_next_argument(s2, &i);
						int32_t fl = grab_next_argument(s2, &i);
						
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
						_menu_tl = grab_next_argument(s2, &i);
						_menu_cs = grab_next_argument(s2, &i);
						_menu_t_wid = grab_next_argument(s2, &i);
						_menu_t_hei = grab_next_argument(s2, &i);
						_menu_fl = grab_next_argument(s2, &i);
						break;
					}
					case MSGC_MENUCHOICE:
					{
						(void)grab_next_argument(s2, &i);
						(void)grab_next_argument(s2, &i);
						(void)grab_next_argument(s2, &i);
						(void)grab_next_argument(s2, &i);
						(void)grab_next_argument(s2, &i);
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
					
					default:
						if(s3[k] >= 32 && s3[k] <= 126)
						{
							//textprintf_ex(buf,workfont,cursor_x,cursor_y,msgcolour,-1,"%c",s3[k]);
							char cbuf[2] = {0};
							
							sprintf(cbuf,"%c",s3[k]);
							
							textout_styled_aligned_ex(buf,workfont,cbuf,cursor_x,cursor_y,shdtype,sstaLEFT,msgcolour,shdcolor,-1);
							
							cursor_x += workfont->vtable->char_length(workfont, s3[k]);
							cursor_x += str->hspace;
						}
						else
						{
							for(int32_t numops=msg_code_operands(s3[k]-1); numops>0; numops--)
							{
								if(++i < s2.size() && (byte)s2[i]==255)
									i+=2;
							}
						}
						
						break;
				}
			}
			if(BOTTOM_MARGIN_CLIP()) break;
		}
		
		if(nextstring && !visited[nextstring]
			&& MsgStrings[nextstring].stringflags & STRINGFLAG_CONT)
		{
			str = &MsgStrings[nextstring];
			workfont = get_zc_font(str->font);
			
			s2 = str->s;
			strip_trailing_spaces(s2);
			
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
