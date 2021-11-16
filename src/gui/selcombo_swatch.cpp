#include "selcombo_swatch.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"

int32_t newg_selcombo_proc(int32_t msg,DIALOG *d,int32_t c);

namespace GUI
{

SelComboSwatch::SelComboSwatch(): combo(0), cset(0),
	alDialog(), message(-1)
{
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}

void SelComboSwatch::setCombo(int32_t value)
{
	combo = value;
	if(alDialog)
	{
		alDialog->d1 = value;
		pendDraw();
	}
}

void SelComboSwatch::setCSet(int32_t value)
{
	cset = value;
	if(alDialog)
	{
		alDialog->d2 = value;
		pendDraw();
	}
}

int32_t SelComboSwatch::getCombo()
{
	return alDialog ? alDialog->d1 : combo;
}

int32_t SelComboSwatch::getCSet()
{
	return alDialog ? alDialog->d2 : cset;
}

void SelComboSwatch::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void SelComboSwatch::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont(newFont);
}

void SelComboSwatch::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<newg_selcombo_proc>,
		x, y, getHeight(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		combo, cset, // d1, d2,
		nullptr, widgFont, nullptr // dp, dp2, dp3
	});
}

void SelComboSwatch::calculateSize()
{
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s + text_length(widgFont, "Combo: 99999"));
	setPreferredHeight(s);
}

int32_t SelComboSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(alDialog->d1, alDialog->d2);
	if(message >= 0)
		sendMessage(message, alDialog->d1);
	return -1;
}

}
