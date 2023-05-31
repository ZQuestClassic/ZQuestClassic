#include "gui/radio.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "jwin.h"
#include <cassert>
#include <utility>

namespace GUI
{

Radio::Radio(): checked(false), text(),
	alDialog(), message(-1), procset(0)
{
	setPreferredHeight(14_px);
}

void Radio::setText(std::string newText)
{
	int32_t textWidth = text_length(widgFont, newText.c_str());
	setPreferredWidth(Size::pixels(textWidth)+13_px);
	text = std::move(newText);
}

void Radio::setChecked(bool value)
{
	checked = value;
	if(alDialog)
	{
		if(checked)
		{
			alDialog.message(MSG_KEY,0);
		}
		else //May not work correctly?
		{
			alDialog->flags &= ~D_SELECTED;
		}
	}
}

bool Radio::getChecked()
{
	return alDialog ? alDialog->flags&D_SELECTED : checked;
}

void Radio::setProcSet(int32_t newProcSet)
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

void Radio::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
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
		newGUIProc<jwin_radiofont_proc>,
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
	setPreferredWidth(14_px+12_px+Size::pixels(gui_text_width(widgFont, text.c_str())));
	Widget::calculateSize();
}

int32_t Radio::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	if(event != geRADIO) return -1;
	if(message >= 0)
		sendMessage(message, (int32_t)index);
	return -1;
}

}
