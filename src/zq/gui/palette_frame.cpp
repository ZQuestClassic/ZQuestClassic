#include "palette_frame.h"
#include "zquest.h"
#include "gui/common.h"
#include "gui/dialog.h"
#include "gui/dialog_runner.h"
#include "gui/size.h"
#include "../jwin.h"
#include "base/zsys.h"
#include <cassert>
#include <utility>
#include "tiles.h"

namespace GUI
{

PaletteFrame::PaletteFrame(): alDialog(), bmp(NULL), plt(NULL), ColorData(NULL), count(14),
	onUpdate(NULL)
{
	setPreferredWidth(192_px);
	setPreferredHeight(12_px*count);
	bgColor = palette_color[scheme[jcLIGHT]];
	fgColor = palette_color[scheme[jcBOXFG]];
}

void PaletteFrame::setBitmap(BITMAP* value)
{
	bmp = value;
	if(alDialog)
	{
		alDialog->dp = value;
		pendDraw();
	}
}

void PaletteFrame::setColorData(byte* value)
{
	ColorData = value;
	if(alDialog)
	{
		alDialog->dp2 = value;
		pendDraw();
	}
}

void PaletteFrame::setPal(PALETTE* value)
{
	plt = value;
	if(alDialog)
	{
		alDialog->dp3 = *plt;
		pendDraw();
	}
}

int32_t PaletteFrame::getSelection()
{
	if (alDialog) return alDialog->d2;
	
	return 0;
}

void PaletteFrame::setCount(uint8_t value)
{
	count = vbound(value, 1, 14);
}

void PaletteFrame::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void PaletteFrame::applyDisabled(bool dis)
{
	Widget::applyDisabled(dis);
	if(alDialog) alDialog.applyDisabled(dis);
}

void PaletteFrame::calculateSize()
{
	setPreferredHeight(12_px*count);
	Widget::calculateSize();
}

void PaletteFrame::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<d_cset_proc>,
		x, y, getWidth(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		0, 0, // d1, d2,
		bmp, ColorData, *plt // dp, dp2, dp3
	});
}

int32_t PaletteFrame::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	
	if(onUpdate)
		onUpdate();
	return -1;
}

void PaletteFrame::setOnUpdate(std::function<void()> newOnUpdate)
{
	onUpdate = std::move(newOnUpdate);
}

}
