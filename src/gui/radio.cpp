#include "radio.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin_a5.h"
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
	int32_t textWidth = al_get_text_width(widgFont_a5, newText.c_str());
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

void Radio::applyFont_a5(ALLEGRO_FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
	}
	Widget::applyFont_a5(newFont);
}

void Radio::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<jwin_radiofont_proc_a5>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		getAccelKey(text),
		getFlags()|(checked ? D_SELECTED : 0),
		procset, 0, // d1, d2,
		text.data(), widgFont_a5, nullptr // dp, dp2, dp3
	});
}

void Radio::calculateSize()
{
	setPreferredWidth(14_px+12_px+Size::pixels(al_get_text_width(widgFont_a5, text.c_str())));
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
