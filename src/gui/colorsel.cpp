#include "gui/colorsel.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/jwin.h"
#include "base/zdefs.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <utility>

namespace GUI
{

ColorSel::ColorSel(): onValueChangedMsg(-1), colorVal(0)
{
	setPreferredWidth(24_px);
	setPreferredHeight(24_px);
}

void ColorSel::setVal(byte val)
{
	colorVal = val;
	if(alDialog)
	{
		alDialog->d1 = val;
		pendDraw();
	}
}

byte ColorSel::getVal()
{
	return colorVal;
}
void ColorSel::setOnValChanged(std::function<void(byte)> newOnValChanged)
{
	onValChanged = std::move(newOnValChanged);
}

void ColorSel::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<jwin_color_swatch>,
		x+2, y+2, getWidth()-4, getHeight()-4,
		fgColor, bgColor,
		0, // key
		getFlags(), // flags
		colorVal, 16, // d1, d2
		nullptr, nullptr, nullptr // dp, dp2, dp3
	});
}

void ColorSel::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void ColorSel::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

int32_t ColorSel::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	int32_t message = -1;
	switch(event)
	{
		case geCHANGE_VALUE:
			colorVal = (byte)vbound(alDialog->d1, 0, 255);
			message = onValueChangedMsg;
			onValChanged(colorVal);
			break;
	}
	if(message < 0)
		return -1;
	
	sendMessage(message, colorVal);

	return -1;
}

}
