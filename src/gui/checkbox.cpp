#include "checkbox.h"
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

Checkbox::Checkbox(): checked(false), text(),
	placement(boxPlacement::LEFT), alDialog(), message(-1)
{
	setPreferredHeight(9_spx);
}

void Checkbox::setText(std::string newText)
{
	int textWidth = text_length(widgFont, newText.c_str());
	setPreferredWidth(Size::pixels(textWidth)+13_lpx);
	text = std::move(newText);
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
		static_cast<int>(placement), 0, // d1, d2,
		text.data(), widgFont, nullptr // dp, dp2, dp3
	});
}

void Checkbox::calculateSize()
{
	//setPreferredHeight(Size::pixels(std::min(9_spx.resolve(), text_height(widgFont)+2_spx.resolve())));
	setPreferredWidth(9_spx+12_px+Size::pixels(gui_text_width(widgFont, text.c_str())));
}

int Checkbox::onEvent(int event, MessageDispatcher& sendMessage)
{
	assert(event == geTOGGLE);
	if(onToggleFunc)
		onToggleFunc((alDialog->flags&D_SELECTED) != 0);
	if(message >= 0)
		sendMessage(message, (alDialog->flags&D_SELECTED) != 0);
	return -1;
}

}
