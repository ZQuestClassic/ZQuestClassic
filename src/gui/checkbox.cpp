#include "checkbox.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include <cassert>
#include <utility>

namespace GUI
{

Checkbox::Checkbox(): checked(false), text(),
	placement(boxPlacement::LEFT), alDialog(), message(-1)
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
	setPreferredWidth(14_px+12_px+Size::pixels(gui_text_width(widgFont, text.c_str())));
	setPreferredHeight(Size::pixels(std::max(text_height(widgFont), 14)));
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
