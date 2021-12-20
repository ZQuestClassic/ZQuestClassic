#include "palette_frame.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include "../zsys.h"
#include <cassert>
#include <utility>
#include "tiles.h"

namespace GUI
{

PaletteFrame::PaletteFrame(): alDialog(), bmp(NULL), plt(NULL), ColorData(NULL), count(14)
{
	setPreferredWidth(128_spx);
	setPreferredHeight(8_spx*count);
	bgColor = palette_color[scheme[jcLIGHT]];
	fgColor = palette_color[scheme[jcBOXFG]];
}

void PaletteFrame::setBitmap(BITMAP* value)
{
	bmp = value;
}

void PaletteFrame::setColorData(byte* value)
{
	ColorData = value;
}

void PaletteFrame::setPal(PALETTE value)
{
	plt = value;
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
	Widget::applyVisibility(dis);
	if(alDialog) alDialog.applyDisabled(dis);
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
		bmp, ColorData, plt // dp, dp2, dp3
	});
}

}
