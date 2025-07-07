#include "gui/text_field.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zdefs.h"
#include "base/zsys.h"
#include <gui/builder.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <utility>

bool edit_ins_mode = true;

void put_legacy_edit_str(char* s, int32_t x, int32_t y, int32_t w, int32_t h, int32_t fg, int32_t bg, int32_t pos)
{
	int32_t i = 0;
	char buf[2] = { 0, 0 };
	FONT* fnt = get_zc_font(font_zfont);

	// text_mode(bg);
	for (int32_t dy = 0; dy < h; dy++)
		for (int32_t dx = 0; dx < w; dx++)
		{
			if (edit_ins_mode)
			{
				buf[0] = *(s + i);
				textout_ex(screen, fnt, buf, x + (dx << 3), y + (dy << 3), fg, bg);
			}
			else
			{
				//     text_mode(i==pos?vc(15):bg);
				buf[0] = *(s + i);
				textout_ex(screen, fnt, buf, x + (dx << 3), y + (dy << 3), fg, bg);
			}

			++i;
		}

	if (edit_ins_mode && pos > -1)
	{
		//   text_mode(-1);
		buf[0] = '_';
		textout_ex(screen, fnt, buf, x + ((pos % w) << 3), y + ((pos / w) << 3), vc(15), -1);
	}
}

// Old proc used by the dmap editor for fixed size strings
int32_t d_legacy_edit_proc(int32_t msg, DIALOG* d, int32_t c)
{
	char* s = (char*)(d->dp);

	int32_t cw = (d->w - 4) / 8;
	int32_t ch = (d->h - 4) / 8;
	int32_t len = d->d1;
	int32_t x = d->x + 3;
	int32_t y = d->y + 3;

	switch (msg)
	{
	case MSG_WANTFOCUS:
		return D_WANTFOCUS;

	case MSG_CLICK:
		d->d2 = ((gui_mouse_x() - x) >> 3) + ((gui_mouse_y() - y) >> 3) * cw;
		bound(d->d2, 0, len - 1);
		//put_legacy_edit_str(s, x, y, cw, ch, jwin_pal[jcTEXTBG], jwin_pal[jcTEXTFG], d->d2);
		d->flags |= D_DIRTY;

		while (gui_mouse_b())
		{
			/* do nothing */
			rest(1);
		}

		break;

	case MSG_DRAW:
		if (!(d->flags & D_GOTFOCUS))
		{
			d->d2 = -1;
		}

		rectfill(screen, x - 3, y - 3, x - 3 + d->w - 1, y - 3 + d->h - 1, scheme[jcTEXTBG]);
		jwin_draw_frame(screen, x - 3, y - 3, d->w, d->h, FR_DEEP);
		put_legacy_edit_str(s, x, y, cw, ch, scheme[jcTEXTFG], scheme[jcTEXTBG], d->d2);
		break;

	case MSG_CHAR:
		bool used = false;
		int32_t k = c >> 8;

		switch (k)
		{
		case KEY_INSERT:
			edit_ins_mode = !edit_ins_mode;
			used = true;
			break;

		case KEY_HOME:
			d->d2 -= d->d2 % cw;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_END:
			d->d2 -= d->d2 % cw;
			d->d2 += cw - 1;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_UP:
			if (d->d2 >= cw) d->d2 -= cw;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_DOWN:
			if (d->d2 < cw * (ch - 1)) d->d2 += cw;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_LEFT:
			if (d->d2 > 0) --d->d2;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_RIGHT:
			if (d->d2 < len - 1) ++d->d2;
			d->flags |= D_DIRTY;
			used = true;
			break;

		case KEY_BACKSPACE:
			if (d->d2 > 0)
				--d->d2;

		case KEY_DEL:
			strcpy(s + d->d2, s + d->d2 + 1);
			s[len - 1] = ' ';
			s[len] = 0;
			used = true;
			d->flags |= D_DIRTY;
			GUI_EVENT(d, geCHANGE_VALUE);
			break;

		default:
			if (isprint(c & 255))
			{
				if (edit_ins_mode)
				{
					for (int32_t i = len - 1; i > d->d2; i--)
						s[i] = s[i - 1];
				}

				s[d->d2] = c & 255;

				if (d->d2 < len - 1)
					++d->d2;

				d->flags |= D_DIRTY;
				GUI_EVENT(d, geCHANGE_VALUE);
				used = true;
			}
		}

		return used ? D_USED_CHAR : D_O_K;
	}

	return D_O_K;
}

namespace GUI
{

TextField::TextField(): buffer(nullptr), startVal(0), fixedPlaces(4),
	lbound(0), ubound(-1), tfType(type::TEXT), swap_type_start(0), maxLength(0),
	forced_length(false), last_applied_vis(true), last_applied_dis(false), onEnterMsg(-1),
	onValueChangedMsg(-1), valSet(false)
{
	setPreferredWidth(1_em);
	setPreferredHeight(24_px);
	fgColor = vc(12);
	bgColor = vc(1);
}

void TextField::setType(type newType)
{
	tfType = newType;
	refresh_cb_swap();
}

void TextField::setText(std::string_view newText)
{
	check_len(newText.size());
	if(newText.size())
	{
		newText.copy(buffer.get(), maxLength);
		buffer[std::min(maxLength, newText.size())] = '\0';
	}
	else buffer[0] = '\0';
	
	Size nsz = Size::pixels(isSwapType(true) ? 16 : 0) + Size::pixels(gui_text_width(widgFont, &buffer[0]));
	if(getWidth() < nsz)
		setPreferredWidth(nsz);
	
	valSet = true;
	pendDraw();
}

void TextField::setVal(int32_t val)
{
	if(ubound > lbound)
		val = vbound(val, lbound, ubound);
	char buf[32] = {0};
	switch(tfType)
	{
		case type::TEXT:
		case type::INT_DECIMAL:
			sprintf(buf, "%d", val);
			break;
		case type::INT_HEX:
			sprintf(buf, "%X", val);
			break;
		case type::SWAP_BYTE:
			startVal = vbound(val, 0, 255);
			sprintf(buf, "%d", startVal);
			break;
		case type::SWAP_SSHORT:
			startVal = vbound(val, -32768, 32767);
			sprintf(buf, "%d", startVal);
			break;
		case type::SWAP_ZSINT:
		case type::SWAP_ZSINT2:
			startVal = val;
			if(!(startVal%10000))
				sprintf(buf, "%d", startVal/10000);
			else
			{
				strcpy(buf, zslongToFix(startVal).str().c_str());
				for(auto q = strlen(buf)-1; buf[q] == '0'; --q)
					buf[q] = 0;
			}
			break;
		case type::NOSWAP_ZSINT:
		{
			startVal = val;
			if(alDialog)
			{
				alDialog->fg = val;
				alDialog.message(MSG_START,0);
			}
			break;
		}
		case type::SWAP_ZSINT_NO_DEC:
		{
			startVal = val * 10000;
			sprintf(buf,"%d",val);
			break;
		}
		case type::FIXED_DECIMAL:
		{
			int32_t scale = int32_t(pow(10, fixedPlaces));
			char templ[32] = {0};
			sprintf(templ, "%%s%%d.%%0%dd", fixedPlaces);
			sprintf(buf, templ, val < 0 ? "-" : "", abs(val)/scale, abs(val)%scale);
			for(size_t q = strlen(buf)-1; q > 0; --q)
			{
				if(buf[q] == '0')
				{
					buf[q] = 0;
					continue;
				}
				else if(buf[q] == '.')
					buf[q] = 0;
				break;
			}
			break;
		}
		
	}
	
	std::string v(buf);
	//
	size_t s = 1;
	switch(tfType)
	{
		case type::TEXT:
		case type::INT_DECIMAL:
		case type::INT_HEX:
		case type::FIXED_DECIMAL:
			if(v.size()>0)
			{
				s = v.size();
			}
			break;
		case type::SWAP_BYTE:
			s = 3;
			break;
		case type::SWAP_SSHORT:
			s = 6;
			break;
		case type::SWAP_ZSINT:
		case type::SWAP_ZSINT2:
		case type::NOSWAP_ZSINT:
			s = 12;
			break;
		case type::SWAP_ZSINT_NO_DEC:
			s = 7;
			break;
	}
	check_len(s);
	//
	strcpy(buffer.get(), v.c_str());
	
	Size nsz = Size::pixels(isSwapType(true) ? 16 : 0) + Size::pixels(gui_text_width(widgFont, &buffer[0]));
	if(getWidth() < nsz)
		setPreferredWidth(nsz);
	
	if(isSwapType() && alDialog)
	{
		alDialog->fg = startVal;
		if(swapBtnDialog)
			swapBtnDialog->d1 &= 0xF;
	}
	
	valSet = true;
	pendDraw();
}

std::string_view TextField::getText()
{
	if(maxLength > 0)
	{
		size_t max = strlen(buffer.get());
		if(max > maxLength+1)
			max = maxLength+1;
		return std::string_view(buffer.get(), max);
	}
	else
		return std::string_view("", 1);
}
void TextField::setLowBound(int32_t low)
{
	lbound = low;
	if(ubound > lbound)
		startVal = vbound(startVal, lbound, ubound);
	check_len(1);
}
void TextField::setHighBound(int32_t high)
{
	ubound = high;
	if(ubound > lbound)
		startVal = vbound(startVal, lbound, ubound);
	check_len(1);
}
int32_t TextField::getVal()
{
	int32_t value=0;
	switch(tfType)
	{
		case type::TEXT:
		case type::INT_DECIMAL:
			try { value = std::stoi(buffer.get(), nullptr, 10); }
			catch(std::exception) { value = 0; }
			break;

		case type::INT_HEX:
			try { value = std::stoi(buffer.get(), nullptr, 16); }
			catch(std::exception) { value = 0; }
			break;
			
		case type::SWAP_BYTE:
		case type::SWAP_SSHORT:
		case type::SWAP_ZSINT:
		case type::SWAP_ZSINT2:
		case type::NOSWAP_ZSINT:
			if(alDialog)
				value = alDialog->fg;
			else value = startVal;
			break;
		case type::SWAP_ZSINT_NO_DEC:
			if(alDialog)
				value = alDialog->fg / 10000;
			else value = startVal / 10000;
			break;
			
		case type::FIXED_DECIMAL:
		{
			int32_t scale = int32_t(pow(10, fixedPlaces));
			char buf[32] = {0};
			char* intptr = buf;
			char* decptr = NULL;
			strcpy(buf, buffer.get());
			bool negative = buf[0] == '-';
			if(negative) ++intptr;
			buf[31] = 0;
			for(size_t q = 0;q<31;++q)
			{
				if(buf[q] == '.')
				{
					buf[q] = 0;
					decptr = buf+q+1;
					for(int32_t j = 0; j < fixedPlaces; ++j)
						if(decptr[j] == 0)
							decptr[j] = '0';
					decptr[fixedPlaces] = 0;
					break;
				}
				if(buf[q] == 0)
					break;
			}
			value = atoi(intptr) * scale;
			if(decptr) value += atoi(decptr);
			if(negative)
				value = -value;
			break;
		}
	}
	
	if(ubound > lbound)
		return vbound(value, lbound, ubound);
	return value;
}

int32_t TextField::getSwapType()
{
	if(swapBtnDialog)
	{
		int32_t d = swapBtnDialog->d1;
		return (d&0xF);
	}
	return swap_type_start;
}
void TextField::setSwapType(int32_t newtype)
{
	switch(tfType)
	{
		case type::SWAP_BYTE:
		case type::SWAP_SSHORT:
		case type::SWAP_ZSINT_NO_DEC:
			if(unsigned(newtype) > 1) newtype = 0;
			break;
		case type::SWAP_ZSINT:
			if(unsigned(newtype) > 3) newtype = 0;
			break;
		case type::SWAP_ZSINT2:
		case type::NOSWAP_ZSINT:
			if(unsigned(newtype) > 4) newtype = 0;
			break;
	}
	swap_type_start = newtype;
	if(swapBtnDialog)
	{
		int32_t d = swapBtnDialog->d1;
		if((d&0xF) != ((d&0xF0)>>4))
		{
			swapBtnDialog->d1 = (d&0xF0) | swap_type_start;
		}
		else
		{
			swapBtnDialog->d1 = ((d & 0xF)<<4) | swap_type_start;
		}
		refresh_cb_swap();
		pendDraw();
	}
	else if(alDialog && tfType == type::NOSWAP_ZSINT)
	{
		alDialog.message(MSG_START,0);
	}
}

void TextField::setMaxLength(size_t newMax)
{
	if(newMax < 1)
	{
		forced_length = false;
		check_len(1);
		return;
	}
	forced_length = true;
	
	_updateBuf(newMax);
}

void TextField::check_len(size_t min)
{
	if(forced_length)
		return;
	if(min < 1) min = 1;
	size_t s = std::max(min, maxLength);
	if(ubound > lbound)
	{
		s = std::max(count_digits(lbound), count_digits(ubound));
	}
	_updateBuf(s);
}

void TextField::_updateBuf(size_t sz)
{
	bool z = !sz;
	if(sz < 1) sz = 1;
	if(sz == maxLength)
		return;
	size_t msz = zc_max(sz, 32);
	if(!maxLength || sz >= 32)
	{
		std::unique_ptr<char[]> newBuffer = std::make_unique<char[]>(msz+1);
		if(maxLength > 0)
		{
			strncpy(newBuffer.get(), buffer.get(), std::min(maxLength, sz));
		}
		buffer = std::move(newBuffer);
	}
	buffer[sz] = 0;
	if(z) buffer[0] = 0;
	maxLength = sz;
	
	setPreferredWidth(Size::pixels(isSwapType(true) ? 16 : 0)+Size::em(std::min((sz+1)*0.75, 20.0)));
	if(alDialog)
	{
		alDialog->dp = buffer.get();
	}
}

void TextField::setOnValChanged(std::function<void(type,std::string_view,int32_t)> newOnValChanged)
{
	onValChanged = std::move(newOnValChanged);
}

void TextField::setFixedPlaces(size_t places)
{
	places = vbound(places,1,4);
	if(valSet)
	{
		int32_t val = getVal();
		fixedPlaces = places;
		setVal(val);
	}
	else fixedPlaces = places;
}

void TextField::applyReadableFont()
{
}
void TextField::updateReadOnly(bool ro)
{
	applyReadableFont();
}
void TextField::realize(DialogRunner& runner)
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	
	Widget::realize(runner);
	assert(maxLength > 0);

	using ProcType = int32_t(*)(int32_t, DIALOG*, int32_t);
	ProcType proc;
	if(isSwapType())
	{
		bool hascb = false;
		bool hasbutton = true;
		switch(tfType)
		{
			case type::SWAP_BYTE:
				proc = newGUIProc<jwin_numedit_swap_byte_proc>;
				if(unsigned(swap_type_start) > 1)
					swap_type_start = 0;
				break;

			case type::SWAP_SSHORT:
				proc = newGUIProc<jwin_numedit_swap_sshort_proc>;
				if(unsigned(swap_type_start) > 1)
					swap_type_start = 0;
				break;

			case type::SWAP_ZSINT:
				proc = newGUIProc<jwin_numedit_swap_zsint_proc>;
				if(unsigned(swap_type_start) > 3)
					swap_type_start = 0;
				break;
			
			case type::SWAP_ZSINT_NO_DEC:
				proc = newGUIProc<jwin_numedit_swap_zsint_nodec_proc>;
				if(unsigned(swap_type_start) > 1)
					swap_type_start = 0;
				break;

			case type::SWAP_ZSINT2:
				proc = newGUIProc<jwin_numedit_swap_zsint2_proc>;
				if(unsigned(swap_type_start) > 4)
					swap_type_start = 0;
				hascb = true;
				break;

			case type::NOSWAP_ZSINT:
				proc = newGUIProc<jwin_numedit_noswap_zsint_proc>;
				if(unsigned(swap_type_start) > 4)
					swap_type_start = 0;
				hascb = true;
				hasbutton = false;
				break;

			default:
				break;
		}
		
		int32_t totalwid = getWidth();
		int32_t btnwid = (24_px).resolve();
		int32_t txtfwid = totalwid-(isSwapType(true)?btnwid:0);
		
		alDialog = runner.push(shared_from_this(), DIALOG {
			proc,
			x, y, txtfwid, getHeight(),
			startVal, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int32_t>(maxLength), 0, // d1, d2
			buffer.get(), widgFont, nullptr // dp, dp2, dp3
		});
		if(hasbutton)
			swapBtnDialog = runner.push(shared_from_this(), DIALOG {
				newGUIProc<jwin_swapbtn_proc>,
				x+txtfwid, y, btnwid, getHeight(),
				0, 0,
				0, // key
				getFlags(), // flags
				swap_type_start, 0, // d1, d2
				nullptr, GUI_DEF_FONT, nullptr // dp, dp2, dp3
			}, true);
		if(hascb)
		{
			swap_cb = std::make_shared<GUI::Checkbox>();
			swap_cb->setText("True/On");
			swap_cb->setChecked(getVal()!=0);
			swap_cb->setPadding(0_px);
			swap_cb->onToggle(onValueChangedMsg);
			swap_cb->setOnToggleFunc([&](bool state)
				{
					setVal(state?10000:0);
					if(onValChanged) onValChanged(tfType, getText(), getVal());
				});
			swap_cb->calculateSize();
			swap_cb->arrange(x,y,txtfwid,getHeight());
			swap_cb->realize(runner);
		}
		alDialog->dp3 = (void*)this;
	}
	else if (tfType == type::TEXT_LEGACY)
	{
		alDialog = runner.push(shared_from_this(), DIALOG{
			newGUIProc<d_legacy_edit_proc>,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int32_t>(maxLength), 0, // d1, d2
			buffer.get(), widgFont, nullptr // dp, dp2, dp3
			});
	}
	else
	{
		switch(tfType)
		{
			case type::TEXT:
			case type::FIXED_DECIMAL:
				proc = newGUIProc<jwin_edit_proc>;
				break;

			case type::INT_DECIMAL:
				proc = newGUIProc<jwin_numedit_proc>;
				break;

			case type::INT_HEX:
				proc = newGUIProc<jwin_hexedit_proc>;
				break;

			default:
				break;
		}

		alDialog = runner.push(shared_from_this(), DIALOG {
			proc,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int32_t>(maxLength), 0, // d1, d2
			buffer.get(), widgFont, nullptr // dp, dp2, dp3
		});
	}
}

void TextField::applyVisibility(bool visible)
{
	last_applied_vis = visible;
	bool sw4 = false;
	Widget::applyVisibility(visible);
	if(swap_cb)
	{
		sw4 = getSwapType() == nswapBOOL;
		swap_cb->applyVisibility(sw4 && visible);
	}
	if(alDialog) alDialog.applyVisibility(!sw4 && visible);
	if(swapBtnDialog) swapBtnDialog.applyVisibility(visible);
}

void TextField::applyDisabled(bool dis)
{
	last_applied_dis = dis;
	bool sw4 = false;
	Widget::applyDisabled(dis);
	if(swap_cb)
	{
		sw4 = getSwapType() == nswapBOOL;
		swap_cb->applyDisabled(!sw4 || dis);
	}
	if(alDialog) alDialog.applyDisabled(sw4 || dis);
	if(swapBtnDialog) swapBtnDialog.applyDisabled(dis);
	applyReadableFont();
}

void TextField::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

int32_t TextField::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	int32_t message = -1;
	switch(event)
	{
		case geENTER:
			message = onEnterMsg;
			break;

		case geCHANGE_VALUE:
			if(!isSwapType())
			{
				message = onValueChangedMsg;
				if(onValChanged) onValChanged(tfType, getText(), getVal());
			}
			break;
			
		case geUPDATE_SWAP:
			if(isSwapType())
			{
				message = onValueChangedMsg;
				if (onValChanged) onValChanged(tfType, getText(), getVal());
				if(swap_cb && getSwapType() != nswapBOOL)
				{
					swap_cb->setChecked(getVal()!=0);
				}
			}
			break;
		
		case geCLICK:
			break;

		default:
			assert(false);
	}
	if(message < 0)
		return -1;

	if(maxLength > 0)
	{
		if(tfType == type::TEXT)
		{
			sendMessage(message, getText());
		}
		else
		{
			sendMessage(message, getVal());
		}
	}
	else // maxLength == 0 - actually, this isn't possible...
	{
		if(tfType == type::TEXT)
			sendMessage(message, std::string_view(""));
		else
			sendMessage(message, 0);
	}

	return -1;
}

size_t TextField::get_str_pos()
{
	size_t len = strlen((char const*)alDialog->dp);
	if(unsigned(alDialog->d2) > len)
		return len;
	return alDialog->d2;
}

int32_t TextField::get_str_selected_pos()
{
	size_t len = strlen((char const*)alDialog->dp);
	int32_t cursor_start = alDialog->d2 & 0x0000FFFF;
	if (cursor_start == 0xFFFF)
		cursor_start = -1;
	return cursor_start;
}

int32_t TextField::get_str_selected_endpos()
{
	size_t len = strlen((char const*)alDialog->dp);
	int32_t cursor_end = int32_t((alDialog->d2 & 0xFFFF0000) >> 16);
	if (cursor_end == 0xFFFF)
		cursor_end = -1;
	return cursor_end;
}

void TextField::refresh_cb_swap()
{
	if(swap_cb)
	{
		applyVisibility(last_applied_vis);
		applyDisabled(last_applied_dis);
		pendDraw();
	}
}

}
