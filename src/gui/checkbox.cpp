#include "gui/checkbox.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "gui/jwin.h"
#include <cassert>
#include <utility>

int32_t new_check_proc(int32_t msg, DIALOG *d, int32_t)
{
	using GUI::Checkbox;
	
	Checkbox::boxPlacement box_align = static_cast<Checkbox::boxPlacement>(d->d1);
	int32_t box_xoff=0, txt_wid=0;
	ASSERT(d);
	
	FONT *oldfont = font;
	unsigned char const* str = (unsigned char const*)d->dp;
	bool has_text = str && str[0];
	if(d->dp2)
	{
		font = (FONT *)d->dp2;
	}
	switch(msg)
	{
		case MSG_DRAW:
		{
			const int box_spacing = 4;
			int32_t tx = 2, ty = 2, txt_x = 2;
			int box_sz = d->h;
			int fh = text_height(font);
			auto txt_y = ty+(d->h-fh)/2;
			BITMAP* tmp = create_bitmap_ex(8, d->w+4, d->h+4);
			clear_bitmap(tmp);
			set_clip_rect(tmp, 2, 2, tmp->w-2, tmp->h-2);
			if(box_align == Checkbox::boxPlacement::RIGHT)
			{
				txt_x = tmp->w - (box_sz + box_spacing); //right-alignment
				if(has_text)
				{
					txt_wid = gui_text_width(font, (const char*)str);
					txt_x -= txt_wid + box_spacing; //handle right-alignment for text
					if(d->flags & D_DISABLED)
					{
						gui_textout_ln(tmp, font, str, txt_x+1, txt_y+1, scheme[jcLIGHT], scheme[jcBOX], 0);
						txt_wid=gui_textout_ln(tmp, font, str, txt_x, txt_y, scheme[jcDISABLED_FG], -1, 0);
						box_xoff=txt_wid+box_spacing;
					}
					else
					{
						txt_wid=gui_textout_ln(tmp, font, str, txt_x, txt_y, scheme[jcBOXFG], scheme[jcBOX], 0);
						box_xoff=txt_wid+box_spacing;
					}
				}
				tx = txt_x; // ensure box is drawn properly for right-alignment
			}
			
			jwin_draw_frame(tmp, tx+box_xoff, ty, box_sz, box_sz, FR_DEEP);
			
			if(!(d->flags & D_DISABLED))
			{
				rectfill(tmp, tx+box_xoff+2, ty+2, tx+box_xoff+box_sz-3, ty+box_sz-3, scheme[jcTEXTBG]);
			}
			
			if(box_align == Checkbox::boxPlacement::LEFT)
			{
				txt_x=tx+box_xoff+box_sz-1+box_spacing;
				
				if(has_text)
				{
					if(d->flags & D_DISABLED)
					{
						gui_textout_ln(tmp, font, str, txt_x+1, txt_y+1, scheme[jcLIGHT], scheme[jcBOX], 0);
						txt_wid=gui_textout_ln(tmp, font, str, txt_x, txt_y, scheme[jcDISABLED_FG], -1, 0);
					}
					else
					{
						txt_wid=gui_textout_ln(tmp, font, str, txt_x, txt_y, scheme[jcBOXFG], scheme[jcBOX], 0);
					}
				}
			}

			if(d->flags & D_SELECTED)
			{
				line(tmp, tx+box_xoff+2, ty+2, tx+box_xoff+box_sz-3, ty+box_sz-3, scheme[jcTEXTFG]);
				line(tmp, tx+box_xoff+2, ty+box_sz-3, tx+box_xoff+box_sz-3, ty+2, scheme[jcTEXTFG]);
			}
			
			set_clip_rect(tmp, 0, 0, tmp->w, tmp->h);
			if(has_text)
			{
				dotted_rect(tmp, txt_x-1, txt_y-1, txt_x+txt_wid, txt_y+fh, (d->flags & D_GOTFOCUS)?scheme[jcDARK]:scheme[jcBOX], scheme[jcBOX]);
			}
			
			masked_blit(tmp, screen, 0, 0, d->x-2, d->y-2, d->w+4, d->h+4);
			break;
		}
	}
	
	int32_t rval = D_O_K;
	if(msg != MSG_DRAW)
		rval = d_jwinbutton_proc(msg, d, 0);
	font = oldfont;
	return rval;
}

namespace GUI
{

Checkbox::Checkbox(): message(-1), checked(false),
	text(), placement(boxPlacement::LEFT), alDialog()
{
	setPreferredHeight(14_px);
}

void Checkbox::setText(std::string newText)
{
	int32_t textWidth = text_length(widgFont, newText.c_str());
	setPreferredWidth(Size::pixels(textWidth)+13_px);
	text = newText;
	if(alDialog)
	{
		alDialog->dp = text.data();
	}
}

void Checkbox::setChecked(bool value)
{
	checked = value;
	if(alDialog)
	{
		if(checked)
			alDialog->flags |= D_SELECTED;
		else
			alDialog->flags &= ~D_SELECTED;
	}
}

bool Checkbox::getChecked()
{
	return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void Checkbox::setOnToggleFunc(std::function<void(bool)> newOnToggleFunc)
{
	onToggleFunc = std::move(newOnToggleFunc);
}

void Checkbox::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void Checkbox::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void Checkbox::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void Checkbox::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<new_check_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		getAccelKey(text),
		getFlags()|(checked ? D_SELECTED : 0),
		static_cast<int32_t>(placement), 0, // d1, d2,
		text.data(), widgFont, nullptr // dp, dp2, dp3
	});
}

void Checkbox::calculateSize()
{
	auto prefh = Size::pixels(std::max(text_height(widgFont)+4, 14));
	if (text.empty())
		setPreferredWidth(prefh);
	else setPreferredWidth(prefh+4+gui_text_width(widgFont, text.c_str())+2);
	setPreferredHeight(prefh);
	Widget::calculateSize();
}

int32_t Checkbox::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geTOGGLE);
	if(onToggleFunc)
		onToggleFunc((alDialog->flags&D_SELECTED) != 0);
	if(message >= 0)
		sendMessage(message, (alDialog->flags&D_SELECTED) != 0);
	return -1;
}

}
