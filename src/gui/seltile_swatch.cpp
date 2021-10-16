#include "seltile_swatch.h"
#include "common.h"
#include "dialog.h"
#include "dialog_runner.h"
#include "size.h"
#include "../jwin.h"
#include "../zquest.h"
#include <cassert>
#include <utility>
#include "tiles.h"

int32_t newg_seltile_proc(int32_t msg,DIALOG *d,int32_t c);

namespace GUI
{

SelTileSwatch::SelTileSwatch(): tile(0), cset(0),
	alDialog(), message(-1)
{
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s);
	setPreferredHeight(s);
}

void SelTileSwatch::setTile(int32_t value)
{
	tile = value;
	if(alDialog)
	{
		alDialog->d1 = value;
		pendDraw();
	}
}

void SelTileSwatch::setCSet(int32_t value)
{
	cset = value;
	if(alDialog)
	{
		alDialog->d2 = value;
		pendDraw();
	}
}

int32_t SelTileSwatch::getTile()
{
	return alDialog ? alDialog->d1 : tile;
}

int32_t SelTileSwatch::getCSet()
{
	return alDialog ? alDialog->d2 : cset;
}

void SelTileSwatch::applyVisibility(bool visible)
{
	Widget::applyVisibility(visible);
	if(alDialog) alDialog.applyVisibility(visible);
}

void SelTileSwatch::applyFont(FONT* newFont)
{
	if(alDialog)
	{
		alDialog->dp2 = newFont;
		pendDraw();
	}
	Widget::applyFont(newFont);
}

void SelTileSwatch::realize(DialogRunner& runner)
{
	Widget::realize(runner);
	alDialog = runner.push(shared_from_this(), DIALOG {
		newGUIProc<newg_seltile_proc>,
		x, y, getHeight(), getHeight(),
		fgColor, bgColor,
		0,
		getFlags(),
		tile, cset, // d1, d2,
		nullptr, widgFont, nullptr // dp, dp2, dp3
	});
}

void SelTileSwatch::calculateSize()
{
	Size s = sized(16_px,32_px)+4_px;
	setPreferredWidth(s + text_length(widgFont, "Tile: 999999"));
	setPreferredHeight(s);
}

int32_t SelTileSwatch::onEvent(int32_t event, MessageDispatcher& sendMessage)
{
	assert(event == geCHANGE_SELECTION);
	if(onSelectFunc)
		onSelectFunc(alDialog->d1, alDialog->d2);
	if(message >= 0)
		sendMessage(message, alDialog->d1);
	return -1;
}

}
