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

#define FONT sized(nfont, lfont_l)

namespace GUI
{

TextField::TextField(): buffer(nullptr), tfType(type::TEXT), maxLength(0),
	onEnterMsg(-1), onValueChangedMsg(-1), startVal(0)
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
}

std::string_view TextField::getText()
{
	if(maxLength > 0)
		return std::string_view(buffer.get(), maxLength+1);
	else
		return std::string_view("", 1);
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
	}
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
	
	setPreferredWidth(Size::largePixels(btnsz)+Size::em(std::min(newMax*0.75, 20.0)));
}

void TextField::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	assert(maxLength > 0);

	using ProcType = int(*)(int, DIALOG*, int);
	ProcType proc;
	switch(tfType)
	{
		case type::TEXT:
		case type::INT_DECIMAL:
		case type::INT_HEX:
		{
			switch(tfType)
			{
				case type::TEXT:
					proc = jwin_edit_proc;
					break;

				case type::INT_DECIMAL:
					proc = jwin_numedit_proc;
					break;

				case type::INT_HEX:
					proc = jwin_hexedit_proc;
					break;
			}

			alDialog = runner.push(shared_from_this(), DIALOG {
				proc,
				x, y, getWidth(), getHeight(),
				fgColor, bgColor,
				0, // key
				getFlags(), // flags
				static_cast<int>(maxLength), 0, // d1, d2
				buffer.get(), FONT, nullptr // dp, dp2, dp3
			});
			break;
		}
		case type::SWAP_BYTE:
		case type::SWAP_SSHORT:
		case type::SWAP_ZSINT:
		{
			switch(tfType)
			{
				case type::SWAP_BYTE:
					proc = jwin_numedit_swap_byte_proc;
					break;

				case type::SWAP_SSHORT:
					proc = jwin_numedit_swap_sshort_proc;
					break;

				case type::SWAP_ZSINT:
					proc = jwin_numedit_swap_zsint_proc;
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
				buffer.get(), FONT, nullptr // dp, dp2, dp3
			});
			swapBtnDialog = runner.push(shared_from_this(), DIALOG {
				jwin_swapbtn_proc,
				x+txtfwid, y, btnwid, getHeight(),
				0, 0,
				0, // key
				getFlags(), // flags
				0, 0, // d1, d2
				nullptr, FONT, nullptr // dp, dp2, dp3
			});
			//Set the dp3 to the swapbtn pointer
			//alDialog->dp3 = (void*)&(swapBtnDialog[0]);
			alDialog->dp3 = (void*)1;
		}
	}
	
}

void TextField::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
	if(swapBtnDialog) swapBtnDialog.applyVisibility(visible);
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
				message = onValueChangedMsg;
			break;
			
		case geUPDATE_SWAP:
			if(isSwapType())
				message = onValueChangedMsg;
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
