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
	onEnterMsg(-1), onValueChangedMsg(-1)
{
	setPreferredWidth(1_em);
	setPreferredHeight(24_lpx);
	fgColor=vc(12);
	bgColor=vc(1);
}

void TextField::setText(std::string_view newText)
{
	// This probably could be handled with less allocating and copying...
	if(maxLength==0 && newText.size()>0)
		setMaxLength(newText.size());
	newText.copy(buffer.get(), maxLength);
	buffer[std::min(maxLength-1, newText.size())]='\0';
}

void TextField::setMaxLength(size_t newMax)
{
	assert(newMax>0);
	if(newMax==maxLength)
		return;

	auto newBuffer=std::make_unique<char[]>(newMax+1);
	if(maxLength>0)
	{
		strncpy(newBuffer.get(), buffer.get(), std::min(maxLength, newMax));
		newBuffer[newMax-1]='\0';
	}
	else
		newBuffer[0]='\0';

	buffer=std::move(newBuffer);
	maxLength=newMax;

	setPreferredWidth(Size::em(std::min(newMax*0.75, 20.0)));
}

void TextField::realize(DialogRunner& runner)
{
	assert(maxLength>0);

	using ProcType=int(*)(int, DIALOG*, int);
	ProcType proc;
	switch(tfType)
	{
	case type::TEXT:
		proc=jwin_edit_proc;
		break;

	case type::INT_DECIMAL:
		proc=jwin_numedit_proc;
		break;

	case type::INT_HEX:
		proc=jwin_hexedit_proc;
		break;
	}

	alDialog=runner.push(shared_from_this(), DIALOG {
		proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		static_cast<int>(maxLength), 0, // d1, d2
		buffer.get(), FONT, nullptr // dp, dp2, dp3
	});
}

void TextField::applyVisibility(bool visible)
{
	if(alDialog)
	{
		if(visible)
			alDialog->flags&=~D_HIDDEN;
		else
			alDialog->flags|=D_HIDDEN;
	}
}

int TextField::onEvent(int event, MessageDispatcher sendMessage)
{
	int message;
	switch(event)
	{
	case ngeENTER:
		message=onEnterMsg;
		break;

	case ngeCHANGE_VALUE:
		message=onValueChangedMsg;
		break;

	default:
		assert(false);
	}
	if(message<0)
		return -1;

	if(maxLength>0)
	{
		int value;
		switch(tfType)
		{
		case type::TEXT:
			sendMessage(message, std::string_view(buffer.get()));
			break;

		case type::INT_DECIMAL:
			try { value=std::stoi(buffer.get(), nullptr, 10); }
			catch(std::exception) { value=0; }
			sendMessage(message, value);
			break;

		case type::INT_HEX:
			try { value=std::stoi(buffer.get(), nullptr, 16); }
			catch(std::exception) { value=0; }
			sendMessage(message, value);
			break;
		}
	}
	else // maxLength==0 - actually, this isn't possible...
	{
		if(tfType==type::TEXT)
			sendMessage(message, std::string_view(""));
		else
			sendMessage(message, 0);
	}

	return -1;
}

}
