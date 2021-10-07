#include "text_field.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "../jwin.h"
#include "../zdefs.h"
#include "../zquest.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace GUI
{

TextField::TextField(): buffer(nullptr), tfType(type::TEXT), maxLength(0),
	onEnterMsg(-1), onValueChangedMsg(-1), startVal(0), lbound(0), ubound(-1),
	fixedPlaces(4), valSet(false)
{
	setPreferredWidth(1_em);
	setPreferredHeight(24_lpx);
	fgColor = vc(12);
	bgColor = vc(1);
}

void TextField::setText(std::string_view newText)
{
	// This probably could be handled with less allocating and copying...
	if(maxLength == 0 && newText.size() > 0)
		setMaxLength(newText.size());
	newText.copy(buffer.get(), maxLength);
	buffer[std::min(maxLength, newText.size())] = '\0';
	valSet = true;
	if(alDialog && allowDraw() && getVisible())
	{
		alDialog.message(MSG_DRAW, 0);
	}
}

void TextField::setVal(int val)
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
			double scale = pow(10, fixedPlaces);
			char templ[20];
			sprintf(templ, "%%.%df", fixedPlaces);
			sprintf(buf, templ, val/scale);
			break;
		}
		
	}
	std::string_view v(buf);
	// This probably could be handled with less allocating and copying...
	if(maxLength == 0)
	{
		switch(tfType)
		{
			case type::TEXT:
			case type::INT_DECIMAL:
			case type::INT_HEX:
			case type::FIXED_DECIMAL:
				if(v.size()>0)
				{
					setMaxLength(v.size());
				}
				break;
			case type::SWAP_BYTE:
			case type::SWAP_SSHORT:
			case type::SWAP_ZSINT:
				setMaxLength(12);
				break;
		}
	}
	v.copy(buffer.get(), maxLength);
	buffer[std::min(maxLength, v.size())] = '\0';
	valSet = true;
	if(alDialog && allowDraw() && getVisible())
	{
		alDialog.message(MSG_DRAW, 0);
	}
}

std::string_view TextField::getText()
{
	if(maxLength > 0)
		return std::string_view(buffer.get(), maxLength+1);
	else
		return std::string_view("", 1);
}
void TextField::setLowBound(int low)
{
	lbound = low;
}
void TextField::setHighBound(int high)
{
	ubound = high;
}
int TextField::getVal()
{
	int value=0;
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
			double scale = pow(10, fixedPlaces);
			value = int(strtod(buffer.get(), NULL)*scale);
			break;
		}
	}
	if(ubound > lbound)
		return vbound(value, lbound, ubound);
	return value;
}

void TextField::setMaxLength(size_t newMax)
{
	assert(newMax > 0);
	if(newMax == maxLength)
		return;

	auto newBuffer = std::make_unique<char[]>(newMax+1);
	if(maxLength > 0)
	{
		strncpy(newBuffer.get(), buffer.get(), std::min(maxLength, newMax));
		newBuffer[newMax-1] = '\0';
	}
	else
		newBuffer[0] = '\0';

	buffer = std::move(newBuffer);
	maxLength = newMax;
	
	int btnsz = isSwapType() ? 16 : 0;
	
	setPreferredWidth(Size::largePixels(btnsz)+Size::em(std::min((newMax+sized(2,1))*0.75, 20.0)));
}

void TextField::setOnValChanged(std::function<void(type,std::string_view,int)> newOnValChanged)
{
	onValChanged = std::move(newOnValChanged);
}

void TextField::setFixedPlaces(size_t places)
{
	places = vbound(places,1,4);
	if(valSet)
	{
		int val = getVal();
		fixedPlaces = places;
		setVal(val);
	}
	else fixedPlaces = places;
}

void TextField::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	assert(maxLength > 0);

	using ProcType = int(*)(int, DIALOG*, int);
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
		}
		
		int totalwid = getWidth();
		int btnwid = 24_lpx.resolve();
		int txtfwid = totalwid-btnwid;
		
		al_trace("%d = (%d + %d)\n", totalwid, txtfwid, btnwid);
		
		alDialog = runner.push(shared_from_this(), DIALOG {
			proc,
			x, y, txtfwid, getHeight(),
			startVal, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int>(maxLength), 0, // d1, d2
			buffer.get(), widgFont, nullptr // dp, dp2, dp3
		});
		swapBtnDialog = runner.push(shared_from_this(), DIALOG {
			jwin_swapbtn_proc,
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
		}

		alDialog = runner.push(shared_from_this(), DIALOG {
			proc,
			x, y, getWidth(), getHeight(),
			fgColor, bgColor,
			0, // key
			getFlags(), // flags
			static_cast<int>(maxLength), 0, // d1, d2
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

void TextField::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

int TextField::onEvent(int event, MessageDispatcher& sendMessage)
{
	int message = -1;
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
