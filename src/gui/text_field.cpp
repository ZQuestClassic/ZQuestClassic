#include "text_field.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zdefs.h"
#include "../zquest.h"
#include "../zsys.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace GUI
{

TextField::TextField(): buffer(nullptr), tfType(type::TEXT), maxLength(0),
	onEnterMsg(-1), onValueChangedMsg(-1), startVal(0), lbound(0), ubound(-1),
	fixedPlaces(4), valSet(false), forced_length(false)
{
	setPreferredWidth(1_em);
	setPreferredHeight(24_lpx);
	fgColor = vc(12);
	bgColor = vc(1);
}

void TextField::setText(std::string_view newText)
{
	check_len(newText.size());
	newText.copy(buffer.get(), maxLength);
	buffer[std::min(maxLength, newText.size())] = '\0';
	valSet = true;
	pendDraw();
}

void TextField::setVal(int32_t val)
{
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
			break;
		case type::SWAP_SSHORT:
			startVal = vbound(val, -32768, 32767);
			break;
		case type::SWAP_ZSINT:
			startVal = val;
			break;
		case type::FIXED_DECIMAL:
		{
			int32_t scale = int32_t(pow(10, fixedPlaces));
			char templ[32] = {0};
			sprintf(templ, "%%d.%%0%dd", fixedPlaces);
			sprintf(buf, templ, val/scale, val%scale);
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
	std::string_view v(buf);
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
		case type::SWAP_SSHORT:
		case type::SWAP_ZSINT:
			s = 12;
			break;
	}
	check_len(s);
	//
	v.copy(buffer.get(), maxLength);
	buffer[std::min(maxLength, v.size())] = '\0';
	valSet = true;
	pendDraw();
}

std::string_view TextField::getText()
{
	if(maxLength > 0)
		return std::string_view(buffer.get(), maxLength+1);
	else
		return std::string_view("", 1);
}
void TextField::setLowBound(int32_t low)
{
	lbound = low;
	check_len(1);
}
void TextField::setHighBound(int32_t high)
{
	ubound = high;
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
			value = alDialog->fg;
			break;
			
		case type::FIXED_DECIMAL:
		{
			int32_t scale = int32_t(pow(10, fixedPlaces));
			char buf[32] = {0};
			char* decptr = NULL;
			strcpy(buf, buffer.get());
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
			value = atoi(buf) * scale;
			if(decptr) value += atoi(decptr);
			break;
		}
	}
	if(ubound > lbound)
		return vbound(value, lbound, ubound);
	return value;
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
	size_t s = std::max(min, maxLength);
	if(ubound > lbound)
	{
		s = std::max(count_digits(lbound), count_digits(ubound));
	}
	_updateBuf(s);
}

void TextField::_updateBuf(size_t sz)
{
	if(sz == maxLength)
		return;
	auto newBuffer = std::make_unique<char[]>(sz+1);
	if(maxLength > 0)
	{
		strncpy(newBuffer.get(), buffer.get(), std::min(maxLength, sz));
		newBuffer[sz-1] = '\0';
	}
	else
		newBuffer[0] = '\0';

	buffer = std::move(newBuffer);
	maxLength = sz;
	
	int32_t btnsz = isSwapType() ? 16 : 0;
	
	setPreferredWidth(Size::largePixels(btnsz)+Size::em(std::min((sz+sized(2,1))*0.75, 20.0)));
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

void TextField::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	assert(maxLength > 0);

	using ProcType = int32_t(*)(int32_t, DIALOG*, int32_t);
	ProcType proc;
	if(isSwapType())
	{
		switch(tfType)
		{
			case type::SWAP_BYTE:
				proc = newGUIProc<jwin_numedit_swap_byte_proc>;
				break;

			case type::SWAP_SSHORT:
				proc = newGUIProc<jwin_numedit_swap_sshort_proc>;
				break;

			case type::SWAP_ZSINT:
				proc = newGUIProc<jwin_numedit_swap_zsint_proc>;
				break;

			default:
				break;
		}
		
		int32_t totalwid = getWidth();
		int32_t btnwid = (24_lpx).resolve();
		int32_t txtfwid = totalwid-btnwid;
		
		alDialog = runner.push(shared_from_this(), DIALOG {
			proc,
			x, y, txtfwid, getHeight(),
			startVal, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int32_t>(maxLength), 0, // d1, d2
			buffer.get(), widgFont, nullptr // dp, dp2, dp3
		});
		swapBtnDialog = runner.push(shared_from_this(), DIALOG {
			newGUIProc<jwin_swapbtn_proc>,
			x+txtfwid, y, btnwid, getHeight(),
			0, 0,
			0, // key
			getFlags(), // flags
			0, 0, // d1, d2
			nullptr, GUI_DEF_FONT, nullptr // dp, dp2, dp3
		});
		//Set the dp3 to the swapbtn pointer
		//alDialog->dp3 = (void*)&(swapBtnDialog[0]);
		alDialog->dp3 = (void*)1;
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
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
	if(swapBtnDialog) swapBtnDialog.applyVisibility(visible);
}

void TextField::applyDisabled(bool dis)
{
	Widget::applyVisibility(dis);
	if(alDialog) alDialog.applyDisabled(dis);
	if(swapBtnDialog) swapBtnDialog.applyDisabled(dis);
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
			sendMessage(message, std::string_view(buffer.get()));
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

}
