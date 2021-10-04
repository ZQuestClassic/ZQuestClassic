#include "radio.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>

namespace GUI
{

Radio::Radio(): checked(false), text(),
	alDialog(), message(-1), procset(0)
{
	setPreferredHeight(9_spx);
}

void Radio::setText(std::string newText)
{
	int textWidth = text_length(widgFont, newText.c_str());
	setPreferredWidth(Size::pixels(textWidth)+13_lpx);
	text = std::move(newText);
}

void Radio::setChecked(bool value)
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

bool Radio::getChecked()
{
	return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void Radio::setProcSet(int newProcSet)
{
	procset = newProcSet;
	if(alDialog) alDialog->d1 = procset;
}
	
void Radio::setIndex(size_t newIndex)
{
	index = newIndex;
}

void Radio::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void Radio::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont(newFont);
}

void Radio::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		jwin_radiofont_proc,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		getAccelKey(text),
		getFlags()|(checked ? D_SELECTED : 0),
		procset, 0, // d1, d2,
		text.data(), widgFont, nullptr // dp, dp2, dp3
	});
}

void Radio::calculateSize()
{
	setPreferredWidth(9_spx+12_px+Size::pixels(gui_text_width(widgFont, text.c_str())));
}

int Radio::onEvent(int event, MessageDispatcher& sendMessage)
{
	if(event != geRADIO) return -1;
	if(message >= 0)
		sendMessage(message, (int)index);
	return -1;
}

}
